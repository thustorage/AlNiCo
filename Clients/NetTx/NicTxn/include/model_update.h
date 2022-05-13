/*
 * @Author: your name
 * @Date: 2021-06-21 16:13:13
 * @LastEditTime: 2022-03-29 17:01:21
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /TxSys/include/control/model_update.h
 */
#ifndef MODEL_UPDATE_H
#define MODEL_UPDATE_H
#include "Debug.h"
#include "tpcc_schema.h"
#include "mystorage.h"
#include <bits/stdc++.h>
using namespace std;





struct feature_partitiont_t{
    int offset[15];
    int size[15];
    // int8_t belongs[FEATURE_SIZE];
};

extern feature_partitiont_t feature_partition;


inline int hash_k2f(tx_key_t key)
{
    return feature_partition.offset[key.first] + (key.second % feature_partition.size[key.first]);
}

inline int hash_k2f_ycsb(uint32_t col, uint32_t key)
{
#if YCSBCOL <= FEATURE_SIZE
    return col * (FEATURE_SIZE / YCSBCOL) + (key % (FEATURE_SIZE / YCSBCOL));
#endif
    return 0;
}

struct abort_queue_t {
    queue<int> abort_key;
    int abort_depth;
    void init()
    {
        abort_depth = 0;
        while (!abort_key.empty()) {
            abort_key.pop();
        }
    }
    pair<int, int> push(tx_key_t new_key)
    {
        pair<int, int> ret;
        if (abort_depth == ABORT_DEPTH)
            ret.first = abort_key.front();
        else
            ret.first = FEATURE_SIZE + 1;
        ret.second = hash_k2f(new_key);
        return ret;
    }
};

void init_feature_partition();

class model_update_t {
private:
    /* data */
    uint8_t* volatile weights; // reservered memory. FPGA DMA READ them.
    uint8_t* volatile guideline_group;
    uint16_t* core_weights[MAX_THREAD]; // weights per core. update it by the aborted keys.
    pair<double, uint16_t>* sorted_weights;
    pair<double, uint16_t> hottest_table;
    double table_weight_avg[TABLE_NUM_MAX];

public:
    model_update_t()
    {
    }
    ~model_update_t()
    {
    }

    void init_addr(uint64_t addr)
    {
        weights = (uint8_t*)addr;
        sorted_weights = (pair<double, uint16_t>*)malloc(sizeof(pair<double, uint16_t>) * FEATURE_SIZE);
        guideline_group = (uint8_t*)(addr + FEATURE_SIZE);
        memset(weights, 0, 512);
        for (int i = 0; i < 512; i++) {
            guideline_group[i] = 20; /*MAX_THREAD*/
        }
        _mm_mfence();
    }

    inline void update_weight(int thread_id, pair<int, int> abort_shift)
    {
        core_weights[thread_id][abort_shift.second]++;
        core_weights[thread_id][abort_shift.first]--;
    }

    inline void merge_weights()
    {

        int t_id = -1;
        hottest_table = make_pair(1e-5, -1);
        for (int j = 0; j < FEATURE_SIZE; j++) {
            if (j == feature_partition.offset[t_id + 1]) {
                t_id++;
                table_weight_avg[t_id] = 0;
            }
            double weight_double = weights[j] * 0.9;
            for (int i = 0; i < MAX_THREAD; i++) {
                weight_double += 0.2 * core_weights[i][j] / MAX_THREAD;
            }

            if (weight_double < 1)
                weights[j] = 1;
            else
                weights[j] = weight_double;
            table_weight_avg[t_id] += weight_double / feature_partition.size[t_id];
            sorted_weights[j].first = -weight_double;
            sorted_weights[j].second = j;
            if (table_weight_avg[t_id] > hottest_table.first) {
                hottest_table = make_pair(table_weight_avg[t_id], t_id);
            }
            // weights[j] = 0;
        }
        for (int j = 0; j < FEATURE_SIZE; j++)
            guideline_group[j] = MAX_THREAD;

        // hard partition the hottest table
        if (hottest_table.second != -1){
            int t_id = hottest_table.second;
            int hot_f_id_b = feature_partition.offset[t_id];
            int hot_f_id_e = feature_partition.offset[t_id] + feature_partition.size[t_id]; 
            if (feature_partition.size[t_id] > MAX_THREAD){
                sort(sorted_weights + hot_f_id_b, sorted_weights + hot_f_id_e);
                double table_local_max = sorted_weights[hot_f_id_b].first;
                int direct = 1;
                int ans = 0;
                for (int i = hot_f_id_b; i < hot_f_id_e; i++){
                    guideline_group[sorted_weights[i].second] = ans;
                    ans+=direct;
                    if (ans == -1){
                        ans = 0;
                        direct = 1;
                    }
                    if (ans == MAX_THREAD){
                        ans = MAX_THREAD - 1;
                        direct = -1;
                    }
                    sorted_weights[i].first = 0;
                    weights[sorted_weights[i].second] = table_local_max;
                }
            }
        }

        sort(sorted_weights, sorted_weights + FEATURE_SIZE);
        // other table
        for (int j = 0; j < MAX_THREAD; j++) {
            int ans = j;
            if (sorted_weights[j].first > -8) {
                ans = MAX_THREAD;
            }
            guideline_group[sorted_weights[j].second] = ans;
        }
        
        /*
        for (int i = 0; i < 20; i++){
            guideline_group[i+35] = i;
            weights[i+35] = 100;
        }
        */
        _mm_mfence();
    }
    inline void print()
    {
        int i = 0;
        for (int j = 0; j < FEATURE_SIZE; j++) {
            if (j == feature_partition.offset[i]) {
                puts("");
                printf("\n%d offsetbegin=%d  size=%d: ", i, feature_partition.offset[i], feature_partition.size[i]);
                i++;
            }
            printf("%d-", weights[j]);
        }
        puts("");
    }

    inline void init_thread(int thread_id)
    {
        // +1 for the init_value of oldest_key_hash
        core_weights[thread_id] = (uint16_t*)malloc((FEATURE_SIZE + 1) * sizeof(uint16_t));
        memset(core_weights[thread_id], 0, (FEATURE_SIZE + 1) * sizeof(uint16_t));
    }
};

#endif
