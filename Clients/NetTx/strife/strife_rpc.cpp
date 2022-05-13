

#include "strife_rpc.h"
strife_rpc::strife_rpc(const NodeInfo &my_node_info,
                       const std::vector<NodeInfo> &others,
                       uint16_t thread_id_info, uint16_t port,
                       memcached_st *_memc, int _dev_id)
    : nictxn_rpc(my_node_info, others, thread_id_info, port, _memc, _dev_id)
{

    puts("strife init ok");
}

void strife_rpc::run_client_strife()
{
    bindCore(thread_id);
#ifdef YCSBplusT
    StoRandomDistribution<>::rng_type rng(thread_id /*seed*/);
    dd = new sampling::StoZipfDistribution<>(rng, 0, YCSB_TABLE_SIZE - 1, YCSBZIPF);
#endif

#ifdef ZIPFTPCC
    StoRandomDistribution<>::rng_type rng(thread_id * 3 + node_id /*seed*/);
    dd = new sampling::StoZipfDistribution<>(rng, 1, CROSSNNNN, 0.99);
#endif

    int test_count_debug = 0;
    double wait_length = 100000.0;
    latency_evaluation_t *strife_delivery_latency = NULL;
    latency_evaluation_t *strife_neworder_latency = NULL;
    if (thread_id == 0)
    {
        strife_delivery_latency = new latency_evaluation_t(IDMAPADDRSIZE);
        strife_neworder_latency = new latency_evaluation_t(IDMAPADDRSIZE);
        #ifdef YCSBHOT 
        strife_neworder_latency->set_range(400000); // not enough
        #endif
    }

    for (int i = 0; i < IDMAPADDRSIZE; i++)
    {
        if (thread_id == 0){
            strife_delivery_latency->init_thread(i);
            strife_neworder_latency->init_thread(i);
        }
        strife_cqe[i] = 0;
        strife_flag[i] = true;
        tpcc_type[i] = -1;
    }

    int head_id;
    int tail_id;

    int d_i = 0;

    while (true)
    {
        // if (node_id != 1) continue;
        // if (thread_id != 0) continue;
        for (int i = 0; i < SLOT_NUM; i++)
        {
            // sleep(5);
            if (!(strife_reply_ok(rpcnum[0]) && strife_reply_ok(rpcnum[0] + 1)))
            {
                break;
            }
            if (my_tx[i]->is_busy == false)
            {
                generate_tx(i); // call single one
                strife_flag[my_tx[i]->single_rpc_id] = false;

                if (node_id == 1 && thread_id == 0 && (ok_count > 2000))
                {
                    if (my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::del)
                    {
                        if (node_id == 1 && thread_id == 0)
                            strife_delivery_latency->begin(my_tx[i]->single_rpc_id);
                    }
                    else if (my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::new_order)
                    {
                        if (node_id == 1 && thread_id == 0)
                            strife_neworder_latency->begin(my_tx[i]->single_rpc_id);
                    }
                    tpcc_type[my_tx[i]->single_rpc_id] = my_tx[i]->tpcc_latency_type;
                }

                // if (thread_id == 0 && tpcc_type[my_tx[i]->single_rpc_id] == (int)tpcc_txn_type_t::pay){
                //     printf("send i=%d %d %d\n",i, my_tx[i]->single_rpc_id, rpcnum[0]);
                //     fflush(stdout);
                // }
            }
            else
            {
                uint64_t addr = connect[my_tx[i]->single_node_id]->get_my_reply_addr(i);
                test_count_debug++;
                if ((*(volatile rpc_id_t *)addr).rpc_id == my_tx[i]->single_rpc_id + NOTHING_DEBUG)
                {
                    // if (thread_id == 0 && my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::pay){
                    //     printf("send over i=%d  %d\n",i,my_tx[i]->single_rpc_id);
                    //     fflush(stdout);
                    // }
                    my_tx[i]->now_phase = (*(volatile rpc_id_t *)addr).my_nodeid;
                    throughput_c(i);
                    if (node_id == 1 && thread_id == 0 && (ok_count == 20000))
                    {
#ifndef YCSBplusT
                        strife_neworder_latency->merge_print("_tpcc_new_order", 0);
                        strife_delivery_latency->merge_print("_tpcc_delivery", 0);
#else
                        strife_neworder_latency->merge_print("_ycsb_latency", 0);
#endif

                    }
                }
            }
        }
        if ((strife_flag[d_i] == false) && update_reply_flag(d_i, 0))
        {
            strife_flag[d_i] = true;
            // if (thread_id == 0 && tpcc_type[d_i] == (int)tpcc_txn_type_t::pay){
            //     printf("ok %d\n",d_i);
            //     fflush(stdout);
            // }
            if (node_id == 1 && thread_id == 0 && (ok_count > 2000))
            {
                if (tpcc_type[d_i] == (int)tpcc_txn_type_t::new_order)
                {
                    strife_neworder_latency->end(d_i);
                }
                else if (tpcc_type[d_i] == (int)tpcc_txn_type_t::del)
                {
                    strife_delivery_latency->end(d_i);
                }
            }
        }
        d_i = (d_i + 1) % IDMAPADDRSIZE;
    }
    return;
}

bool strife_rpc::strife_reply_ok(uint32_t rpc_id)
{
    rpc_id = rpc_id % IDMAPADDRSIZE;
    // uint64_t addr = connect[server_id]->get_strife_reply_addr(rpc_id);
    // uint8_t reply = *(volatile uint8_t*)addr;
    // if (reply == strife_cqe[rpc_id]) {
    //     return true;
    // } else {
    //     return false;
    // }
    return strife_flag[rpc_id];
}
