/*
 * @Date: 2020-09-21 09:44:14
 * @LastEditTime: 2022-01-09 16:15:55
 * @FilePath: /TxSys/src/control/rpc_init.cpp
 * @Author: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "Rpc.h"
#include "PlatformFeatures.hh"

latency_evaluation_t performance(MAX_CORE);
latency_evaluation_t performance_listen(MAX_CORE);
latency_evaluation_t strife_batch(MAX_CORE);



Rpc::Rpc(const NodeInfo& my_node_info, const std::vector<NodeInfo>& others, memcached_st* _memc, int _dev_id)
{
    /*ID base*/
    int thread_id_info = 0;
    thread_id = thread_id_info;
    my_node = my_node_info;
    my_global_id = my_node_info.node_id * MAX_THREAD + thread_id;
    node_id = my_node_info.node_id;
    others_server = others;
    dev_id = _dev_id;
    puts("??");
    /*For Connection*/
    Debug::notifyError("begin connect:%d", thread_id);
    memc = _memc;

    createContext(&context, 1, 3, 1);
    createContext(&context_dual, 1, 3, 2);

    cq = ibv_create_cq(context.ctx, (MAX_SERVER + 1) * 2 * BatchPostRec, NULL, NULL, 0);
    send_cq = ibv_create_cq(context.ctx, 2 * BatchSignalSend + 16, NULL, NULL, 0);
    context.send_cq = send_cq;
    
    cq_dual = ibv_create_cq(context_dual.ctx, (MAX_SERVER + 1) * 2 * BatchPostRec, NULL, NULL, 0);
    send_cq_dual = ibv_create_cq(context_dual.ctx, 2 * BatchSignalSend + 16, NULL, NULL, 0);
    context_dual.send_cq = send_cq_dual;

    recv_mm.init_dual(fpga_mm, fpga_rev_size, &context, &context_dual); // metadata

    mm_size = (my_node.message_size_rpc_reply) * (MAX_CLIENT * CLIENT_THREAD); // reply
    uint64_t send_addr = (uint64_t)(volatile uint64_t*)malloc(mm_size);
    send_mm.init_dual(send_addr, mm_size, &context, &context_dual);
    memset((uint32_t *)send_addr, 0, mm_size); // for reply // don't change it
    reply_mm = send_mm.mm_part1; // for print

    uint64_t data_size = my_node.double_write_data_size * (MAX_CLIENT * CLIENT_THREAD); // data
    uint64_t doublerdma_addr = (uint64_t)(volatile uint64_t*)hugePageAlloc(data_size);
    double_recv_mm.init_dual(doublerdma_addr, data_size, &context, &context_dual);
    printf("!!! write my from %llx to %llx ------- total %llx\n", doublerdma_addr, doublerdma_addr + data_size, data_size);
    init_connect();
    // for (int i = 0; i < MAX_SERVER; i++)
    // {
    //     for (int j = 0; j < ASYNCTXNUM; j++)
    //     {
    //         performance_lock.init_thread(i * ASYNCTXNUM + j);
    //         // performance_version.init_thread(i * ASYNCTXNUM + j);
    //     }
    // }
    // performance.init_thread(thread_id);

    // printf("init over\n");
}

memcached_st* connectMemcached()
{
    memcached_server_st* servers = NULL;
    memcached_return rc;
    std::string memcached_ip = "10.0.2.140";
    in_port_t port = 2666;
    // freopen("./memcached.conf", "r", stdin);
    // puts("??");
    // std::cin >> memcached_ip >> port;
    // puts("??");
    // freopen("CON", "r", stdin);
    memcached_st* memc = memcached_create(NULL);
    servers = memcached_server_list_append(servers, memcached_ip.c_str(), port, &rc);
    rc = memcached_server_push(memc, servers);

    if (rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "Counld't add server:%s\n", memcached_strerror(memc, rc));
        exit(0);
        return NULL;
    }
    return memc;
}

Rpc* server;