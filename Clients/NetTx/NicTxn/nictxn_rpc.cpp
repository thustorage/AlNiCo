
#include "nictxn_rpc.h"

model_update_t model_update;

int fpga_fd;
volatile fpga_dispatch_queue_t* fpga_dispatch_queue;
uint32_t* control_addr;
// volatile uint32_t fpga_init_over;
uint64_t fpga_mm;
uint64_t control_mm;
uint64_t ddio_mm; // not sure whether ddio is ok
uint64_t host_mm;
uint64_t reply_mm = 0;
int operation_fd;
uint64_t fpga_operation_mm;
uint64_t parallel_ip_addr;
const uint32_t fpga_rev_size = SLOT_LEN * SLOT_NUM * MAX_CLIENT * CLIENT_THREAD;

nictxn_tx::nictxn_tx(nictxn_rpc* nictxn_rpc_ptr_,
    uint8_t _tx_id, uint64_t _global_id, uint8_t _node_id,
    uint8_t _thread_id)
    : switch_tx(_tx_id, _global_id, _node_id, _thread_id)
{
    nictxn_rpc_ptr = nictxn_rpc_ptr_;
    now_phase = 0;
    
}

nictxn_rpc::nictxn_rpc(const NodeInfo& my_node_info,
    const std::vector<NodeInfo>& others,
    uint16_t thread_id_info, uint16_t port,
    memcached_st* _memc, int _dev_id)
    : tx_rpc(my_node_info, others, thread_id_info, port, _memc, _dev_id)
{

    /* client alloc & init for txns */
    int old = dup(0);
    FILE* fp = freopen("../../hot.txt", "r", stdin);
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 20; j++){
            scanf("%d ", &hot_record[i][j]);
        }
    } 
    fflush(fp);
    dup2(old, 0);
    if(thread_id_info == 0){
       for (int i = 0; i < 8; i++){
        for (int j = 0; j < 20; j++){
            printf("%d ", hot_record[i][j]);
        }
        puts("");
        fflush(stdout);
        }  
    }

    if (my_node_info.server_type == SERVERTYPE::CLIENT_ONLY) {
        for (int i = 0; i < SLOT_NUM; i++) {
            my_tx[i] = new nictxn_tx(this, i, my_global_id, my_node.node_id, thread_id);
#ifdef BREAKDOWN
            breakdown_coordination.init_thread(i);
            breakdown_issue.init_thread(i);
            breakdown_total.init_thread(i);
#endif
        }
        for (int i = 0; i < SLOT_NUM; i++) {
            performance_total.init_thread(i);
        }
    }

    /* static MPL in NicTxn*/
    MPL = ASYNCTXNUM;

    /* server thread 0, client: create contect for RDMA*/
    if (!(my_node_info.server_type == SERVERTYPE::SERVER_NIC && thread_id_info != 0)) {
        createContext(&context, 1, 3, dev_id);
        cq = ibv_create_cq(context.ctx, (MAX_SERVER + 1) * BatchPostRec * 2 + 4096,
            NULL, NULL, 0);
        send_cq = ibv_create_cq(context.ctx, 2 * BatchSignalSend + 16, NULL, NULL, 0);
    }

    memset(switch_tx_status, 0, sizeof(switch_tx_status));

    if (my_node_info.server_type == SERVERTYPE::SERVER_NIC) {
        Debug::notifyError("server:%d", thread_id);
        my_fpga_queue = fpga_dispatch_queue + thread_id;
        if (thread_id_info != 0)
            my_node.server_type = SERVERTYPE::WORKER_ONLY;
        model_update.init_thread(thread_id);
        abort_queue.init();
    }

    /* : alloc RDMA mm*/
    if (my_node.server_type == SERVERTYPE::SERVER_NIC) {
        createContext(&context_dual, 1, 3, 2);
        cq_dual = ibv_create_cq(context_dual.ctx, (MAX_SERVER + 1) * 2 * BatchPostRec, NULL, NULL, 0);
        send_cq_dual = ibv_create_cq(context_dual.ctx, 2 * BatchSignalSend + 16, NULL, NULL, 0);

        recv_mm.init_dual(fpga_mm, fpga_rev_size, &context, &context_dual); // metadata

        mm_size = (my_node.message_size_rpc_reply) * (MAX_CLIENT * CLIENT_THREAD); // reply
        uint64_t send_addr = (uint64_t)(volatile uint64_t*)hugePageAlloc(mm_size);
        send_mm.init_dual(send_addr, mm_size, &context, &context_dual);
        reply_mm = send_mm.mm_part1; // for print

        uint64_t data_size = my_node.double_write_data_size * (MAX_CLIENT * CLIENT_THREAD); // data
        uint64_t doublerdma_addr = (uint64_t)(volatile uint64_t*)hugePageAlloc(data_size);
        double_recv_mm.init_dual(doublerdma_addr, data_size, &context, &context_dual);

    } else if (my_node.server_type != SERVERTYPE::WORKER_ONLY) {
        puts("????");
        mm_size = (my_node.message_size_rpc_reply + my_node.message_size + my_node.double_write_data_size) * my_node.server_num;
        alloc_rdma_mm();
    }
    init_connect();
    puts("nictxn init ok");
}

void nictxn_rpc::init_connect()
{

    if (my_node.server_type == SERVERTYPE::WORKER_ONLY)
        return;
    for (size_t k = 0; k < others_server.size(); k++) {

        // client to server_nic
        if (my_node.server_type == SERVERTYPE::CLIENT_ONLY && others_server[k].server_type != SERVERTYPE::SERVER_NIC) {
            continue;
        }
        if (my_node.server_type == SERVERTYPE::SERVER_NIC && others_server[k].server_type != SERVERTYPE::CLIENT_ONLY) {
            continue;
        }

        if (my_node.server_type == SERVERTYPE::SERVER_NIC) {
            for (int thread_i = 0; thread_i < CLIENT_THREAD; thread_i++) {
                nictxn_new_connect(others_server[k].node_id, thread_i);
            }
        } else {
            nictxn_new_connect(others_server[k].node_id);
        }
    }

    while (true) {
        int flag = 1; // weather all servers are read
        for (size_t k = 0; k < others_server.size(); k++) {
            auto remote_node = others_server[k];
            if (remote_node.node_id == my_node.node_id) {
                continue;
            }

            if (my_node.server_type == SERVERTYPE::CLIENT_ONLY && remote_node.server_type != SERVERTYPE::SERVER_NIC) {
                continue;
            }
            if (my_node.server_type == SERVERTYPE::SERVER_NIC && remote_node.server_type != SERVERTYPE::CLIENT_ONLY) {
                continue;
            }

            bool if_server_nic = (my_node.server_type == SERVERTYPE::SERVER_NIC);
            int connect_thread_num = if_server_nic ? CLIENT_THREAD : 1;
            Connect* tmp_connect;

            for (int thread_i = 0; thread_i < connect_thread_num; thread_i++) {
                tmp_connect = if_server_nic ? nic_connect[remote_node.node_id][thread_i]
                                            : connect[remote_node.node_id];
                if (tmp_connect->online)
                    continue;

                flag = 0;
                if (nictxn_fill_connect(remote_node.node_id, thread_i)) {
                    // if (!if_server_nic)
                    //     post_rec(tmp_connect); TODO: no recv cq for nictxn
                    Debug::notifyInfo("I am server %d, I contect S(%d):T(%d)",
                        my_node.node_id, remote_node.node_id, thread_id);
                }
            }
        }

        if (flag)
            break;
    }
    Debug::notifyInfo("I am Server(%d).T(%d) Connect for Server over. ",
        my_node.node_id, thread_id);

    return;
}

void nictxn_rpc::nictxn_new_connect(uint16_t remote_id,
    uint16_t client_thread_id)
{
    Connect* peer = new Connect();
    RdmaContext* context_ptr = &context;
    ibv_cq *send_cq_ptr = send_cq, *cq_ptr = cq;

    bool if_server_nic = my_node.server_type == SERVERTYPE::SERVER_NIC;
    if (if_server_nic) { // nictxn server
        uint16_t remote_global_id = (remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id;
        peer->localBase = my_node.message_size * remote_global_id + recv_mm.mm_part1; // tx metadata buffer
        peer->reply_addr = my_node.message_size_rpc_reply * remote_global_id + send_mm.mm_part1; // reply buffer
        peer->data_addr = my_node.double_write_data_size * remote_global_id + double_recv_mm.mm_part1;

        localMeta.rAddr = peer->localBase;
        localMeta.reply_rAddr = peer->reply_addr;
        localMeta.data_rAddr = peer->data_addr;

        /* dual port */
        if ((remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id >= MAX_CLIENT * CLIENT_THREAD / 2) {
            // puts("YYYYYYYYYYYYYYYYYYYYYY");
            context_ptr = &context_dual;
            send_cq_ptr = send_cq_dual, cq_ptr = cq_dual;
            localMeta.rkey = recv_mm.mr_part2->rkey; // FPGA for metadata
            localMeta.reply_rkey = send_mm.mr_part2->rkey; // reply on cpu
            localMeta.data_rkey = double_recv_mm.mr_part2->rkey; // data on CPU
        } else {
            localMeta.rkey = recv_mm.mr_part1->rkey;
            localMeta.reply_rkey = send_mm.mr_part1->rkey;
            localMeta.data_rkey = double_recv_mm.mr_part1->rkey;
        }

    } else { // client
        peer->localBase = mm + my_node.message_size * remote_id;
        peer->reply_addr = mm + my_node.message_size * my_node.server_num + remote_id * my_node.message_size_rpc_reply;
        peer->data_addr = mm + (my_node.message_size + my_node.message_size_rpc_reply) * my_node.server_num + remote_id * my_node.double_write_data_size;

        localMeta.reply_rkey = message_mr->rkey;
        localMeta.reply_rAddr = peer->reply_addr;
        localMeta.rkey = localMeta.rAddr = 0;
        localMeta.data_rkey = localMeta.data_rAddr = 0;
    } 

    createQueuePair(&peer->qp, IBV_QPT_RC, send_cq_ptr, cq_ptr, context_ptr);
    write_connect(remote_id, context_ptr, peer, client_thread_id);

    if (my_node.server_type == SERVERTYPE::SERVER_NIC)
        nic_connect[remote_id][client_thread_id] = peer;
    else
        connect[remote_id] = peer;
    return;
}

bool nictxn_rpc::nictxn_fill_connect(uint16_t remote_id, uint16_t client_thread_id)
{
    RdmaContext* context_ptr = &context;
    bool if_server_nic = (my_node.server_type == SERVERTYPE::SERVER_NIC);

    Connect* peer;
    peer = if_server_nic ? nic_connect[remote_id][client_thread_id] : connect[remote_id];

    if (if_server_nic && ((remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id >= MAX_CLIENT * CLIENT_THREAD / 2)) {
        context_ptr = &context_dual;
    }
    uint16_t remote_global_id = (remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id;

    return read_connect(remote_id, context_ptr, peer, client_thread_id);
}

void nictxn_rpc ::run_server()
{
    // new_tx by time

    if (my_node.server_type == SERVERTYPE::CLIENT_ONLY) {
        bindCore(thread_id);
    } else {
        if (thread_id > MAX_THREAD / 2) {
            bindCore(thread_id + 2);
        } else {
            bindCore(thread_id);
        }
    }

    printf("server_type = %d\n", (int)my_node.server_type);
    if (my_node.server_type == SERVERTYPE::WORKER_ONLY) {
        init_buffer_thread(SBUFSIZE * 2);
        run_worker();
        Debug::notifyError("error %s", __func__);
    }
    if (my_node.server_type == SERVERTYPE::SERVER_NIC) {
        // run_nic();
        init_buffer_thread(SBUFSIZE * 2);
        run_worker();
    }
    if (my_node.server_type == SERVERTYPE::CLIENT_ONLY) {
#ifdef YCSBplusT
        StoRandomDistribution<>::rng_type rng(thread_id /*seed*/);
        dd = new sampling::StoZipfDistribution<>(rng, 0, YCSB_TABLE_SIZE/YCSBCOL - 1, YCSBZIPF);
#endif

#ifdef ZIPFTPCC
        StoRandomDistribution<>::rng_type rng(thread_id * 3 + node_id /*seed*/);
        dd = new sampling::StoZipfDistribution<>(rng, 1, CROSSNNNN, 0.99);
#endif

        // run_client();
        run_tpcc_client();
    }
}
