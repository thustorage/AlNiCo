
#include "tx_rpc.h"

tx_rpc::tx_rpc(const NodeInfo &my_node_info,
               const std::vector<NodeInfo> &others,
               uint16_t thread_id_info, uint16_t port, memcached_st *_memc, int _dev_id)
    : rpc(my_node_info, others, thread_id_info, port, _memc, _dev_id)
{
    connect = (Connect **)malloc(sizeof(Connect*) * MAX_SERVER);
    create_workgen_array();
    create_workgen_array_tpcc();
    tot_time = 0;
    tot_time_client = 0;
    test_count_client = 0;
    ok_count = 0;
    fail_count_l = 0;
    fail_count_v = 0;
    flag = false;
    anstp = 0.0;
    ansa1 = 0.0;
    ansa2 = 0.0;

    for (int i = 0; i < MAX_SERVER; i++)
    {
        for (int j = 0; j < MAXMPL; j++)
        {
            performance_lock.init_thread(i * MAXMPL + j);
        }
    }
    performance.init_thread(thread_id);
    
}

void tx_rpc::throughput_c(uint8_t tx_id)
{
    // #ifdef NICNICNIC
    
#ifdef BREAKDOWN
    breakdown_coordination.segment_sum_end(tx_id);
    breakdown_total.end(tx_id);
#endif
    // if (ok_count > COLD_LATENCY_NUM) {
    //     performance_total.end(tx_id);
    // }

    // #else
    performance_total.end(tx_id);
    // #endif
    // switchtxn_throughput.count(thread_id);
    
    // if (node_id == 1 && thread_id == 0 && (ok_count == 2 * COLD_LATENCY_NUM)){
    //     // performance_lock.merge_print("lock_0",0);
    //     // performance_version.merge_print("version_0",0);
    //     performance_total.merge_print("total_virtual",0);
    // }
    ok_count++;
    // puts("??????????");
    my_tx[tx_id]->tx_ok();
#if (DEBUG_ON)

    // if (thread_id == 0) {
    Debug::notifyInfo("work[%d,%d],ok_id[id=%d,magic=%d],total=%d", node_id, thread_id, tx_id, my_tx[tx_id]->magic, test_count_client);
    fflush(stdout);
    // }
#endif

    // if (test_count_client % RESULT_PRINT_EPOCH == 0) {
    //     Debug::notifyError("beats %d (%d-in-%d)", thread_id, ok_count, test_count_client);
    // }
    if (ok_count % RESULT_PRINT_EPOCH == 0) {
        clock_gettime(CLOCK_REALTIME, &E);
        if (ok_count != 0) {
#ifdef NICNICNIC
            if (thread_id == 0) {
                apply_num_base_hot();
            }
#endif

            double f = (E.tv_nsec - S.tv_nsec) * 1.0 / S2N + E.tv_sec - S.tv_sec;
            uint64_t tot = (RESULT_PRINT_EPOCH + fail_count_l + fail_count_v);
            // if (ok_count < 1500000){
            // printf("----[th=%d] count = %d, t = %lf tp = %lf \n --ok = %d fail_l = %d fail_v = %d,[f1=%f],[f2=%f]\n", thread_id,test_count_client, f,100000/f,
            //                     ok_count, fail_count_l,fail_count_v,fail_count_l*1.0/tot,fail_count_v*1.0/tot);
            // fflush(stdout);
            // }
            if (ok_count == RESULT_PRINT_EPOCH) {
                null_poll = 0;
                poll = 0;
            }

            if (ok_count >= RESULT_PRINT_BEGIN * RESULT_PRINT_EPOCH) {
                anstp += RESULT_PRINT_EPOCH / f;
                ansa1 += fail_count_l * 1.0 / tot;
                ansa2 += fail_count_v * 1.0 / tot;
            }
            // if (ok_count == RESULT_PRINT_PRINT * RESULT_PRINT_EPOCH && thread_id == 0){
            //     uint64_t p_c1 = 0, p_c2 = 0;
            //     double t_ans1 = 0, t_ans2 = 0;
            //     if (node_id == 0 && thread_id == 0){
            //         performance_lock.merge_print("lock_0",0);
            //         // performance_version.merge_print("version_0",0);
            //         performance_total.merge_print("total_0.txt",0);
            //     }
            //     if (node_id == 7 && thread_id == 0){
            //         performance_lock.merge_print("lock_7",0);
            //         // performance_version.merge_print("version_7",0);
            //         performance_total.merge_print("total_7.txt",0);
            //     }
            //     double epochs = RESULT_PRINT_PRINT - RESULT_PRINT_BEGIN + 1;
            //     printf("\n%d:ansTTT%dAAA%dKKK%d %lf %lf %lf avt=%lf div %lld = %lf avt2=%lf div %lld = %lf , %lf\n", thread_id,
            //         MAX_THREAD, ASYNCTXNUM, RANGEOFKEY,
            //         anstp / epochs, ansa1 / epochs, ansa2 / epochs,
            //         t_ans1, p_c1, t_ans1 / p_c1,
            //         t_ans2, p_c2, t_ans2 / p_c2,
            //         null_poll * 1.0 / poll);
            //     fflush(stdout);
            // }

            if (ok_count == RESULT_PRINT_END * RESULT_PRINT_EPOCH) {
                uint64_t p_c1 = 0, p_c2 = 0;
                double t_ans1 = 0, t_ans2 = 0;
                // for (int i = 0; i < MAX_SERVER; i++) {
                //     if (i == node_id)
                //         continue;
                //     for (int j = 0; j < ASYNCTXNUM; j++) {
                //         t_ans1 += switch_tx_status[i][j].time_a;
                //         p_c1 += switch_tx_status[i][j].time_count;
                //         t_ans2 += switch_tx_status[i][j].time_v;
                //         p_c2 += switch_tx_status[i][j].v_count;
                //     }
                // }

                // if (node_id == 7 && thread_id == 0){
                //     performance_lock.merge_print("lock_7",0);
                //     // performance_version.merge_print("version_7",0);
                //     performance_total.merge_print("total_7.txt",0);
                // }
                // #ifdef PROFILE
                //                 if (thread_id == 0)
                //                     ProfilerStop();
                // #endif

                double epochs = RESULT_PRINT_END - RESULT_PRINT_BEGIN + 1;
                // printf("\n%d:ansTTT%dAAA%dKKK%d %lf %lf %lf (avt=%lf div %lld = %lf) (avt2=%lf div %lld = %lf) , %lf\n", thread_id,
                //     MAX_THREAD, ASYNCTXNUM, RANGEOFKEY,
                //     anstp / epochs, ansa1 / epochs, ansa2 / epochs,
                //     t_ans1, p_c1, t_ans1 / p_c1,
                //     t_ans2, p_c2, t_ans2 / p_c2,
                //     null_poll * 1.0 / poll);
                fflush(stdout);
            }
            // if (ok_count >= 2200000){
            //     flag = true;
            // }
            fail_count_l = 0;
            fail_count_v = 0;
        }
        clock_gettime(CLOCK_REALTIME, &S);
    }
}

// count for abort transaction
bool tx_rpc::throughput_f(uint8_t tx_id, uint8_t msg_type, bool* b)
{
// switch_tx_flag for the first fail message!!!!!!!
#if (DEBUG_ON)
    Debug::notifyInfo("fail %d %d", tx_id, my_tx[tx_id]->magic);
#endif

    if (*b == false) {
        // Debug::notifyInfo("------[%d,%d],fail %d %d",node_id,thread_id,tx_id,my_tx[tx_id]->magic);
        *b = true;
        if (msg_type == (uint8_t)TxControlType::LOCK_FAIL)
            fail_count_l++;
        else
            fail_count_v++;
        // switchtxn_throughput.retry_count(thread_id);
        
        my_tx[tx_id]->tx_fail();
        // addmission_production();

        return true;
    }
    return false;
}