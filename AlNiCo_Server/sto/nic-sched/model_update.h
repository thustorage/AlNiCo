/*
 * @Author: your name
 * @Date: 2021-06-21 16:13:13
 * @LastEditTime: 2022-05-04 10:32:45
 * @LastEditors: Your Name you@example.com
 * @Description: In User Settings Edit
 * @FilePath: /TxSys/include/control/model_update.h
 */
#ifndef MODEL_UPDATE_H
#define MODEL_UPDATE_H
#include "Debug.h"
#include "HugePageAlloc.h"
#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <xmmintrin.h>
using namespace std;

#define ANALYSIS_FEATURE_NUM 1024
struct feature_t{
    uint8_t f[64];
};
struct feature_partitiont_t
{
    int offset[15];
    int size[15];
    // int8_t belongs[FEATURE_SIZE];
};
extern feature_partitiont_t feature_partition;
class model_update_t
{
private:
    /* data */
    uint8_t *volatile weights; // reservered memory. FPGA DMA READ them.
    uint8_t *volatile guideline_group;
    uint16_t *core_weights[MAX_THREAD]; // weights per core. update it by the aborted keys.
    pair<double, uint16_t> *sorted_weights;
    pair<double, int> hottest_table;
    double table_weight_avg[TABLE_NUM_MAX];
    feature_t *core_features[MAX_THREAD];
    bool zipf_report = true;
    int feature_id[MAX_THREAD][16];
    

public:
    model_update_t()
    {
        zipf_report = true;
    }
    ~model_update_t()
    {
    }

    void init_addr(uint64_t addr)
    {
        weights = (uint8_t *)addr;
        sorted_weights = (pair<double, uint16_t> *)malloc(sizeof(pair<double, uint16_t>) * FEATURE_SIZE);
        guideline_group = (uint8_t *)(addr + FEATURE_SIZE);
        memset(weights, 0, 512);
        for (int i = 0; i < 512; i++)
        {
            guideline_group[i] = MAX_CORE+1; /*MAX_THREAD*/
        }
        zipf_report = true;
        _mm_mfence();
    }

    inline void update_weight(int thread_id, pair<int, int> abort_shift)
    {
        core_weights[thread_id][abort_shift.second]++;
        core_weights[thread_id][abort_shift.first]--;
    }

    inline void print()
    {
        return;
        int i = 0;
        for (int j = 0; j < FEATURE_SIZE; j++)
        {
            if (j == feature_partition.offset[i])
            {
                puts("");
                printf("\n%d offsetbegin=%d  size=%d: ", i, feature_partition.offset[i], feature_partition.size[i]);
                i++;
            }
            printf("%d-", weights[j]);
        }
        puts("");
    }

    inline void merge_weights(bool runtime_print)
    {
        int t_id = -1;
        hottest_table = make_pair(1e-5, -1);
        int nothing_count = 0;
        int count = 0;
        double max_double = 0;

        // update weights
        for (int j = 0; j < FEATURE_SIZE; j++)
        {
            if (j == feature_partition.offset[t_id + 1])
            {
                t_id++;
                nothing_count = 0;
                count = 0;
                table_weight_avg[t_id] = 0;
            }
            double weight_double = 0;
            for (int i = 0; i < MAX_THREAD; i++)
            {
                #ifndef YCSBplusT
                weight_double += 0.2 * core_weights[i][j] / MAX_THREAD;
                #else
                weight_double += core_weights[i][j] / MAX_THREAD;
                #endif
            }

            if (weight_double < 1)
            {
                // weights[j] = 1;
                nothing_count++;
            }
            table_weight_avg[t_id] += weight_double;
            sorted_weights[j].first = -weight_double;
            sorted_weights[j].second = j;
            count++;

            if (weight_double > max_double){
                max_double = weight_double;
            }
            if (count == feature_partition.size[t_id] && count != nothing_count && count-nothing_count>=MAX_THREAD)
            {
                table_weight_avg[t_id] /= (count - nothing_count);
                if (runtime_print){
                    // printf("avg = %lf\n",table_weight_avg[t_id]);
                }
                if (table_weight_avg[t_id] > hottest_table.first)
                {
                    hottest_table = make_pair(table_weight_avg[t_id], t_id);
                }
            }
        }

        // if (runtime_print)
        //     puts("");

        for (int j = 0; j < FEATURE_SIZE; j++)
            guideline_group[j] = MAX_CORE + 1;

        // partition the hottest table
        if (runtime_print)
            print();
        for (int j = 0; j < FEATURE_SIZE; j++){
            #ifdef YCSBplusT
            weights[j] = (-sorted_weights[j].first / max_double / FEATURE_SIZE);
            #else
            weights[j] = (-sorted_weights[j].first / max_double / 200) * 255u + 0.1;
            #endif
        }

        /*set guidelines*/
        if (hottest_table.second != -1)
        {
            int t_id = hottest_table.second;
            int hot_f_id_b = feature_partition.offset[t_id];
            int hot_f_id_e = feature_partition.offset[t_id] + feature_partition.size[t_id];
            if (feature_partition.size[t_id] >= MAX_THREAD ) 
            {
                std::sort(sorted_weights + hot_f_id_b, sorted_weights + hot_f_id_e);
                double table_local_max = sorted_weights[hot_f_id_b].first;
                
                int direct = 1;
                int ans = 0;
                for (int i = hot_f_id_b; i < hot_f_id_e; i++)
                {
                    // if (runtime_print)
                    //     printf("%d %d %lf\n",sorted_weights[i].second, ans, sorted_weights[i].first);
                    guideline_group[sorted_weights[i].second] = ans;
                    ans += direct;
                    sorted_weights[i].first = 0;

                    #ifdef YCSBplusT
                    weights[sorted_weights[i].second] = 50-i; 
                    #else 
                    weights[sorted_weights[i].second] = 90-i;
                    #endif 

                    if (ans == -1)
                    {
                        ans = 0;
                        direct = 1;
                    }
                    if (ans == MAX_THREAD)
                    {
                        ans = 0;
                        break;
                    }
                }
            }

        }
        if (runtime_print)
            print();
        
        _mm_mfence();
    }
    
    inline void store_feature(int thread_id, uint64_t addr){
        memcpy(core_features[thread_id][feature_id[thread_id][0] % ANALYSIS_FEATURE_NUM].f, (uint8_t *)addr ,64);
        feature_id[thread_id][0]++;
    }


    inline void init_thread(int thread_id)
    {
        // +1 for the init_value of oldest_key_hash
        feature_id[thread_id][0] = 0;
        core_features[thread_id] = (feature_t *)hugePageAlloc(sizeof(feature_t) * ANALYSIS_FEATURE_NUM);
        core_weights[thread_id] = (uint16_t *)hugePageAlloc((FEATURE_SIZE + 1) * sizeof(uint16_t));
        memset(core_weights[thread_id], 0, (FEATURE_SIZE + 1) * sizeof(uint16_t));
    }
};

#endif
