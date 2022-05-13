

#if !defined(_latncy_evaluation_)
#define _latncy_evaluation_
#include "timer.h"
#include <bits/stdc++.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
// using namespace nap;

inline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }
class latency_evaluation_t {
private:
    /* data */
    const int time_u = 100; // unit=(time_u)ns // 1000, 60000 for strife
    // 100.00us
    int range;

    uint64_t** latency_bucket;
    uint64_t* ans;
    uint64_t latency_segment_sum;

    Timer** timer;
    uint64_t** req_count;
    uint64_t** req_retry_count;
    int thread_num;
    uint64_t throughput;
    Timer try_print_time;
    uint64_t mpl_tps;
    const int try_gap = 3;
    int try_cnt = 0;
    double average = 0;
    uint64_t interval;
    uint64_t l_p50, l_p90, l_p99, l_p999;

    volatile uint64_t epoch[24];
    volatile uint64_t g_epoch;

    volatile uint16_t g_mpl;

    // const uint64_t cold_start_ok = 1000;
    // uint64_t** cold_start;

public:
    uint32_t print_count;
    // latency_evaluation_t(int _thread_num);
    // ~latency_evaluation_t();

    void init_thread(int thread_id)
    {
        if (latency_bucket[thread_id] != 0)
            return;
        if (thread_id >= thread_num) {
            printf("%d %d\n", thread_id, thread_num);
            puts("Error! init_thread for latency_evaluation");
        }

        latency_bucket[thread_id] = (uint64_t*)malloc((range + 1) * sizeof(uint64_t));
        timer[thread_id] = new Timer();
        req_count[thread_id] = new uint64_t[24];
        req_retry_count[thread_id] = new uint64_t[8];
        // cold_start[thread_id] = new uint64_t[8];

        // memset(cold_start[thread_id], 0, 8 * sizeof(uint64_t));
        memset(latency_bucket[thread_id], 0, range * sizeof(uint64_t));
        memset(req_count[thread_id], 0, 24 * sizeof(uint64_t));
        memset(req_retry_count[thread_id], 0, 8 * sizeof(uint64_t));
        // printf("init_latency:%d\n", thread_id);
        return;
    }

    void increase_epoch(int thread_id)
    {
        (epoch[thread_id])++;
    }
    int check_epoch(int thread_id)
    {
        return (g_epoch - epoch[thread_id]);
    }
    void increase_g_epoch(int x)
    {
        g_epoch += x;
    }
    bool check_all(int x)
    {
        for (int i = 0; i < thread_num-1; i++) {
            if (g_epoch - epoch[i] != x) {
                // printf("%d %lu\n", i, epoch[i]);
                return false;
            }
        }
        return true;
    }
    void increase_all()
    {
        for (int i = 0; i < thread_num; i++) {
            (epoch[i])++;
        }
    }

    void set_mpl(uint16_t n_mpl)
    {
        g_mpl = n_mpl;
    }
    uint16_t get_mpl()
    {
        return g_mpl;
    }
    inline void count_by_id(int thread_id, int parameter_id)
    {
        (req_count[thread_id][parameter_id])++;
        return;
    }

    inline void local_performance_begin(int thread_id)
    {

        return;
    }

    inline void count(int thread_id)
    {
        (req_count[thread_id][0])++;
        return;
    }
    inline void retry_count(int thread_id)
    {
        (req_retry_count[thread_id][0])++;
        return;
    }
    inline uint64_t merge_count(bool all = true)
    {
        uint64_t retry_count = 0;
        for (int i = 0; i < thread_num; i++) {
            if (req_retry_count[i] == 0)
                continue;
            // if (all)
                printf("%d ", req_retry_count[i][0] - req_retry_count[i][1]);
            retry_count += req_retry_count[i][0] - req_retry_count[i][1];
            req_retry_count[i][1] = req_retry_count[i][0];
        }
        // if (all)
        printf("===%d %lf\n", retry_count, retry_count * 1000000000.0 / interval);

        uint64_t last_throught = throughput;
        throughput = 0;
        for (int i = 0; i < thread_num; i++) {
            if (req_count[i] == 0)
                continue;
            // if (all)
                printf("%d ", req_count[i][0] - req_count[i][1]);
            req_count[i][1] = req_count[i][0];
            throughput += req_count[i][0];
        }
        if (all)
            printf("===%d\n", throughput - last_throught);
        return throughput - last_throught;
    }

    inline uint64_t merge_count_by_id(int id)
    {
        uint64_t last_throught = 0;
        for (int i = 0; i < thread_num; i++) {
            if (req_count[i] == 0)
                continue;
            if (id <= 1)
                printf("%d ", req_count[i][id] - req_count[i][id + 8]);
            last_throught += req_count[i][id] - req_count[i][id + 8];
            // = req_count[i][id+16];
            req_count[i][id + 8] = req_count[i][id];
        }
        if (id <= 1)
            puts("");
        // printf("===%d\n", throughput - last_throught);
        return last_throught;
    }
    inline uint64_t get_real_time_tps(int thread_id)
    {
        // if (req_count[thread_id][0] - req_count[thread_id][8]){
        //     return (req_count[thread_id][0] - req_count[thread_id][8]) * 1000000000.0 / interval;
        // }
        return mpl_tps;
        interval = timer[thread_id]->end();
        if (interval > 1000000000ull) {
            uint64_t tps = (req_count[thread_id][0] - req_count[thread_id][16]) * (100000000000.0 / interval);
            printf("tps = %llu %llu %llu (%llu %llu %llu %llu) %llu \n", tps,
                req_count[thread_id][0], req_count[thread_id][16], req_count[thread_id][1],
                req_count[thread_id][2], req_count[thread_id][3], req_count[thread_id][4],
                interval);
            req_count[thread_id][16] = req_count[thread_id][0];
            timer[thread_id]->begin();
            return tps;
        }
        return 0;
    }

    bool try_print_throughput_all(int print_num)
    {
        interval = try_print_time.end();
        if (interval > 1000000000ull * try_gap) {

            uint64_t printf_v;
            for (int i = 0; i < print_num; i++) {
                interval = try_print_time.end();
                printf_v = merge_count_by_id(i);
                if (i == 0) {
                    mpl_tps = printf_v;
                }
                std::string s = "X";
#ifdef ASYNCTXNUM
                s = "A=" + std::to_string(ASYNCTXNUM) + "|";
#endif
                printf("%s %d.%d(%f) %f\n", s.c_str(), try_cnt, i, interval / 1000000000.0, printf_v * 1000000000.0 / interval);
            }

            try_cnt++;
            fflush(stdout);
            try_print_time.begin();
            print_count++;
            return true;
        }
        return false;
    }
    bool try_print_throughput(bool all = true)
    {
        interval = try_print_time.end();
        if (interval > 1000000000ull * try_gap) {
            uint64_t printf_v;
            printf_v = merge_count(all);
            interval = try_print_time.end();
            printf("%d(%f) %f\n", try_cnt++, interval / 1000000000.0, printf_v * 1000000000.0 / interval);
            fflush(stdout);
            try_print_time.begin();
            print_count++;
            return true;
        }
        return false;
    }

    void throughput_listen_ms(uint64_t listen_length)
    {
        int gap = 5; // ms
        int old = dup(1);

        FILE* fp = freopen("Throughput listen.txt", "w", stdout);
        puts("[Throughput listen]: begin");

        uint64_t print_max = listen_length * 1000 / gap;
        uint64_t* printf_array = (uint64_t*)malloc((print_max + 10) * sizeof(uint64_t));
        uint64_t print_num = 0;
        Timer print_time;

        print_time.begin();
        while (print_num < print_max) {
            if (print_time.end() > 1000000 * gap) {
                print_time.begin();
                printf_array[print_num++] = merge_count();
            }
        }
        for (uint64_t i = 0; i < print_max; i++) {
            printf("%d %llu\n", i * gap, printf_array[i] / gap);
        }
        printf("Total: %llu\n", throughput);
        puts("[Throughput listen]: end");
        fflush(fp);
        dup2(old, 1);
    }
    inline void begin(int thread_id)
    {
        timer[thread_id]->begin();
        return;
    }
    inline uint64_t end(int thread_id)
    {
        uint64_t latency_value = timer[thread_id]->end();
        // printf("lat = %d\n",latency_value);
        latency_bucket[thread_id][min(latency_value / time_u, range - 1)]++;
        return latency_value;
    }

    inline void segment_sum_begin(int thread_id)
    {
        latency_segment_sum = 0;
        return;
    }

    inline void segment_begin(int thread_id)
    {
        timer[thread_id]->begin();
        return;
    }

    inline void segment_end(int thread_id)
    {
        latency_segment_sum += timer[thread_id]->end();
        return;
    }

    inline void segment_sum_end(int thread_id)
    {
        latency_segment_sum += timer[thread_id]->end();
        latency_bucket[thread_id][min(latency_segment_sum / time_u, range - 1)]++;
        return;
    }

    void merge_print(std::string filename, int thread_id = 0)
    {
        if (thread_id != 0)
            return;

        ans = (uint64_t*)malloc(sizeof(uint64_t) * (range + 1));
        ans[0] = 0;
        average = 0;
        for (int i = 0; i < range; i++) {
            for (int j = 0; j < thread_num; j++) {
                if (latency_bucket[j] == 0) {
                    continue;
                }
                average += latency_bucket[j][i] * i;
                ans[i] += latency_bucket[j][i];
            }
            ans[i + 1] = ans[i];
        }
        average = average / ans[range] * (time_u / 1000.0);
        int old = dup(1);

        FILE* fp = freopen(("latency" + filename + ".txt").c_str(), "a", stdout);
        printf("total_num = %llu\n", ans[range]);
        puts("[latency_cdf]: begin");
        for (int i = 0; i < range; i++) {
            if (ans[i] * 1.0 / ans[range] < 0.5) {
                l_p50 = i * time_u;
            }
            if (ans[i] * 1.0 / ans[range] < 0.9) {
                l_p90 = i * time_u;
            }
            if (ans[i] * 1.0 / ans[range] < 0.99) {
                l_p99 = i * time_u;
            }
            if (ans[i] * 1.0 / ans[range] < 0.999) {
                l_p999 = i * time_u;
            }
            printf("%f,%f\n", ans[i], i * (time_u / 1000.0), ans[i] * 1.0 / ans[range]);
        }

        puts("[latency_cdf]: end");
        printf("[avg,50,90,99,999]: %lf %lf, %lf, %lf, %lf\n", average, l_p50 / 1000.0, l_p90 / 1000.0, l_p99 / 1000.0,
            l_p999 / 1000.0);
        fflush(fp);
        dup2(old, 1);
        printf("[avg,50,90,99,999]: %lf %lf, %lf, %lf, %lf\n", average, l_p50 / 1000.0, l_p90 / 1000.0, l_p99 / 1000.0,
            l_p999 / 1000.0);
        // fflush();
        free(ans);
        // fclose(stdout);
        // freopen("/dev/console","w",stdout);
    }
    latency_evaluation_t(int _thread_num, int _new_range = -1)
    {
        #ifdef YCSBplusT
            range = 120000;
        #else 
            range = 400000;
        #endif
        g_epoch = 0;
        // memset(()epoch,0,sizeof(epoch));
        for (int i = 0; i < min(_thread_num,24); i++) {
            epoch[i] = 0;
        }
        throughput = 0;
        g_mpl = 1;
        thread_num = _thread_num + 1;
        timer = (Timer**)malloc(sizeof(Timer*) * thread_num);

        latency_bucket = (uint64_t**)malloc(sizeof(uint64_t*) * thread_num);
        memset(latency_bucket, 0, sizeof(uint64_t*) * thread_num);

        // cold_start = (uint64_t**)malloc(sizeof(uint64_t*) * thread_num);
        // memset(cold_start, 0, sizeof(uint64_t*) * thread_num);

        req_count = (uint64_t**)malloc(sizeof(uint64_t*) * thread_num);
        memset(req_count, 0, sizeof(uint64_t*) * thread_num);

        req_retry_count = (uint64_t**)malloc(sizeof(uint64_t*) * thread_num);
        memset(req_retry_count, 0, sizeof(uint64_t*) * thread_num);

        try_print_time.begin();
        print_count = 0;
        return;
    }
    void set_range(int range_){
        range = range_;
        return;
    }
    ~latency_evaluation_t() { return; }
};

#endif // _latncy_evaluation_
