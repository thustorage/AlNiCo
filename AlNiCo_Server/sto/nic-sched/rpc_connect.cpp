/*
 * 
 *    ┏┓　　　┏┓
 *  ┏┛┻━━━┛┻┓
 *  ┃　　　　　　　┃
 *  ┃　　　━　　　┃
 *  ┃　＞　　　＜　┃
 *  ┃　　　　　　　┃
 *  ┃...　⌒　...　┃
 *  ┃　　　　　　　┃
 *  ┗━┓　　　┏━┛
 *      ┃　　　┃　
 *      ┃　　　┃
 *      ┃　　　┃
 *      ┃　　　┃  神兽保佑
 *      ┃　　　┃  代码无bug　　
 *      ┃　　　┃
 *      ┃　　　┗━━━┓
 *      ┃　　　　　　　┣┓
 *      ┃　　　　　　　┏┛
 *      ┗┓┓┏━┳┓┏┛
 *        ┃┫┫　┃┫┫
 *        ┗┻┛　┗┻┛
 */

#include "Rpc.h"

void Rpc::post_rec(Connect* connect)
{
    if (connect->recnum == 0) {
        struct ibv_recv_wr* bad_recv_wr;
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.1??");
        }
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.2??");
        }
        connect->recnum = 2 * BatchPostRec;
    }
    if (connect->recnum == BatchPostRec) {
        struct ibv_recv_wr* bad_recv_wr;
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.3??");
        }
        connect->recnum = 2 * BatchPostRec;
    }
    return;
}
void Rpc::init_connect()
{
    server_connect();
}

void Rpc::server_connect()
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

            int connect_thread_num = CLIENT_THREAD;
            Connect* tmp_connect;

            for (int thread_i = 0; thread_i < connect_thread_num; thread_i++) {
                tmp_connect = nic_connect[remote_node.node_id][thread_i];
                if (tmp_connect->online)
                    continue;

                flag = 0;
                if (nictxn_fill_connect(remote_node.node_id, thread_i)) {
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

void Rpc::nictxn_new_connect(uint16_t remote_id,
    uint16_t client_thread_id)
{
    Connect* peer = new Connect();
    RdmaContext* context_ptr = &context;
    ibv_cq *send_cq_ptr = send_cq, *cq_ptr = cq;

    uint16_t remote_global_id = (remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id;
    peer->localBase = my_node.message_size * remote_global_id + recv_mm.mm_part1; // tx metadata buffer
    peer->reply_addr = my_node.message_size_rpc_reply * remote_global_id + send_mm.mm_part1; // reply buffer
    peer->data_addr =  my_node.double_write_data_size * remote_global_id + double_recv_mm.mm_part1;

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
        peer->local_reply_key = send_mm.mr_part2->lkey;
    } else {
        localMeta.rkey = recv_mm.mr_part1->rkey;
        localMeta.reply_rkey = send_mm.mr_part1->rkey;
        localMeta.data_rkey = double_recv_mm.mr_part1->rkey;
        peer->local_reply_key = send_mm.mr_part1->lkey;
    }


    createQueuePair(&peer->qp, IBV_QPT_RC, send_cq_ptr, cq_ptr, context_ptr);
    write_connect(remote_id, context_ptr, peer, client_thread_id);

    nic_connect[remote_id][client_thread_id] = peer;
    return;
}

bool Rpc::nictxn_fill_connect(uint16_t remote_id, uint16_t client_thread_id)
{
    RdmaContext* context_ptr = &context;

    Connect* peer;
    peer = nic_connect[remote_id][client_thread_id];

    if (((remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id >= MAX_CLIENT * CLIENT_THREAD / 2)) {
        context_ptr = &context_dual;
    }
    uint16_t remote_global_id = (remote_id - KEY2NODE_MOD) * CLIENT_THREAD + client_thread_id;

    return read_connect(remote_id, context_ptr, peer, client_thread_id);    
}



bool Rpc::read_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer, uint16_t client_thread_id)
{
    

    size_t l;
    uint32_t flags;
    std::string getK;
    getK = getKey_nic(remote_id, client_thread_id);

    ExchangeMeta* remoteMeta;
    memcached_return rc;
    int cnt = 1;
    while (cnt--) {
        remoteMeta = (ExchangeMeta*)memcached_get(memc, getK.c_str(), getK.size(), &l, &flags, &rc);
        if (rc == MEMCACHED_SUCCESS) {
            // construct peer data
            memcpy(peer->remoteGid, remoteMeta->gid, 16);
            peer->remoteAddr = remoteMeta->rAddr;
            peer->remoteRkey = remoteMeta->rkey;

            peer->remote_reply_addr = remoteMeta->reply_rAddr;
            peer->remote_reply_key = remoteMeta->reply_rkey;

            peer->remote_data_addr = remoteMeta->data_rAddr;
            peer->remote_data_key = remoteMeta->data_rkey;
            
            // printf("%llx i read\n", remoteMeta->read_rkey);
            peer->remoteQPNum = remoteMeta->qpNum;
            peer->remoteLid = remoteMeta->lid;
            peer->remoteNodeID = remoteMeta->NodeID;
            Debug::notifyError("rQPNum=%d, rLid=%d, rGid=%d %d %d %d", peer->remoteQPNum, peer->remoteLid,
                peer->remoteGid[12], peer->remoteGid[13], peer->remoteGid[14], peer->remoteGid[15]);
            
            modifyQPtoInit(peer->qp, context_ptr);
            modifyQPtoRTR(peer->qp, peer->remoteQPNum, peer->remoteLid, peer->remoteGid, context_ptr);
            
            modifyQPtoRTS(peer->qp);
            free(remoteMeta);
            break;
        } else if (cnt == 0)
            return false;
    }

    

    peer->online = true;
    peer->thread_id = thread_id;
    return true;
}

void Rpc::write_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer,
    uint16_t client_thread_id)
{

    // Debug::notifyError("remoteid = %d localbase = %llx %llx",remote_id,
    // peer->localBase, mm);
    peer->poolSize = my_node.message_size;
    peer->tail = my_node.message_size - 1;

    localMeta.NodeID = my_node.node_id;
    localMeta.lid = context_ptr->lid;
    localMeta.qpNum = peer->qp->qp_num;
    memcpy(localMeta.gid, &(context_ptr->gid), 16);
    // printf("qpnum=%d,lid=%d,gid=%d\n", localMeta.qpNum, localMeta.lid,
    // localMeta.gid[15]);

    // register meta info
    bool if_server_nic = my_node.server_type == SERVERTYPE::SERVER_NIC;
    memcached_return rc;
    std::string setK;
    setK = if_server_nic ? setKey_nic(remote_id, client_thread_id)
                         : setK = setKey(remote_id);

    int cnt = 10;
    while (true) {
        rc = memcached_set(memc, setK.c_str(), setK.size(), (char*)&localMeta,
            sizeof(localMeta), (time_t)0, (uint32_t)0);
        // Debug::notifyError(setK.c_str());
        if (rc == MEMCACHED_SUCCESS)
            break;
        if (--cnt < 0) {
            break;
        }
        fprintf(stderr, "Couldn't register my(%d, %d %d) info: %s, retry...\n",
            my_node.node_id, remote_id, thread_id,
            memcached_strerror(memc, rc));
        usleep(100);
    }
}
