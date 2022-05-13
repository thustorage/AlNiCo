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
    const int time_u = 100; // unit=(time_u)ns
    // 100.00us
    const int range = 10;
    volatile uint8_t dynamic_test_phase;
    uint64_t** latency_bucket;
    uint64_t* ans;
    Timer** timer;
    uint64_t** volatile req_count;
    uint64_t** req_retry_count;
    int thread_num;
    uint64_t throughput;
    Timer try_print_time;
    const double try_gap = 3;
    int try_cnt = 0;
    double average = 0;
    uint64_t l_p50, l_p90, l_p99, l_p999;
    int throughput_ms;
    uint64_t print_num_listen = 0;

    // const uint64_t cold_start_ok = 1000;
    // uint64_t** cold_start;

public:
    // latency_evaluation_t(int _thread_num);
    // ~latency_evaluation_t();
    void update_dynamic_phase(uint8_t now){
        dynamic_test_phase = now;
    }
    uint8_t get_dynamic_phase() volatile{
        return dynamic_test_phase;
    }
    void init_thread(int thread_id)
    {
        if (latency_bucket[thread_id] != 0)
            return;
        if (thread_id >= thread_num) {
            printf("%d %d\n", thread_id, thread_num);
            puts("Error! init_thread for latency_evaluation");
        }
        // printf("init performance %d\n",thread_id);

        latency_bucket[thread_id] = (uint64_t*)malloc(range * sizeof(uint64_t));
        timer[thread_id] = new Timer();
        req_count[thread_id] = (uint64_t*)malloc(8 * sizeof(uint64_t));
        req_retry_count[thread_id] = (uint64_t*)malloc(8 * sizeof(uint64_t));
        // cold_start[thread_id] = new uint64_t[8];

        // memset(cold_start[thread_id], 0, 8 * sizeof(uint64_t));
        memset(latency_bucket[thread_id], 0, range * sizeof(uint64_t));
        memset(req_count[thread_id], 0, 8 * sizeof(uint64_t));
        memset(req_retry_count[thread_id], 0, 8 * sizeof(uint64_t));
        // printf("init_latency:%d\n", thread_id);
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
    inline pair<uint64_t,uint64_t> merge_count(bool runtime_print)
    {
        uint64_t retry_count = 0;
        pair<uint64_t,uint64_t> ret;
        // puts("");
        for (int i = 0; i < thread_num; i++) {
            if (req_retry_count==NULL) continue;
            if (req_retry_count[i] == 0)
                continue;
            // if (runtime_print)
            //     printf("%lld ", req_count[i][0] - req_count[i][1]);
            retry_count += req_retry_count[i][0] - req_retry_count[i][1];
            req_retry_count[i][1] = req_retry_count[i][0];
        }
        // if (runtime_print)
        //     printf("===%d\n", retry_count);

        uint64_t last_throught = throughput;
        // printf("old = %lld: ", throughput);
        throughput = 0;
        for (int i = 0; i < thread_num; i++) {
            if (req_count==NULL) continue;
            if (req_count[i] == NULL) continue;
            // if (runtime_print)
            //     printf("%lld ", req_count[i][0] - req_count[i][1]);
            req_count[i][1] = req_count[i][0];
            throughput += req_count[i][0];
        }
        // if (runtime_print)
        //     printf("===%d\n", throughput - last_throught);
        ret.first = throughput - last_throught;
        ret.second = retry_count;
        return ret;
    }
    bool try_print_throughput()
    {
        uint64_t interval = try_print_time.end();
        if (interval > 1000000000ull * try_gap) {
            pair<uint64_t, uint64_t> printf_v;
            printf_v = merge_count(true);
            interval = try_print_time.end();
            // printf("THREAD(%d) throughput%d-th [total_txns:%f abort_txns:%f abort_rate:%f]\n", MAX_THREAD, try_cnt,
            //     printf_v.first * 1000 / interval, printf_v.second * 1000000000.0 / interval,
            //     (printf_v.second * 1.0 + 0.0001) / (printf_v.first + printf_v.second + 0.001)
            // );
            printf("THREAD(%d) throughput%d-th [perf: %f MTxns/s  abort: %f ]\n", MAX_THREAD, try_cnt++,
                printf_v.first * 1000.0 / interval, // printf_v.second * 1000000000.0 / interval,
                (printf_v.second * 1.0 + 0.0001) / (printf_v.first + printf_v.second + 0.001)
            );
            try_print_time.begin();
            fflush(stdout);
            return true;
        }
        return false;
    }

    inline uint64_t total_count()
    {
        uint64_t now_count = 0;
        for (int i = 0; i < thread_num; i++) {
            if (req_count==NULL) continue;
            if (req_count[i] == NULL) continue;
            now_count += req_count[i][0];
        }
        return now_count;
    }

    inline uint64_t read_count(int thread_id)
    {
        return req_count[thread_id][0];
    }


    bool check_batch_size(uint64_t size_limit, uint64_t time_limit){
        uint64_t interval = try_print_time.end();
        uint64_t tmp_tp = total_count();
        if ((tmp_tp-throughput) >= size_limit || interval > time_limit * 1000000){
            throughput = tmp_tp;
            try_print_time.begin();
            return true;
        }
        return false;
    }
    bool try_print_throughput_null()
    {
        uint64_t interval = try_print_time.end();
        if (interval > 1000000000ull * 2) {
            try_print_time.begin();
            return true;
        }
        return false;
    }
    bool check_throughput(){
        if (throughput_ms < 1100 && print_num_listen % 2 == 0){ // 10ms print == 20ms updates
            return true;
        }
        return false;
    }
    

    void throughput_listen_ms(uint64_t listen_length)
    {
        puts("listen");
        int gap = 10; // ms
        uint64_t print_max = listen_length * 1000 / gap;
        uint64_t* printf_array = (uint64_t*)malloc((print_max + 10) * sizeof(uint64_t));
        uint64_t* printf_abort = (uint64_t*)malloc((print_max + 10) * sizeof(uint64_t));
        uint64_t* phase_count = (uint64_t*)malloc((print_max + 10) * sizeof(uint64_t));
        print_num_listen = 0;
        Timer print_time;

        print_time.begin();
        while (print_num_listen < print_max) {
            if (print_time.end() > 1000000ull * gap) {
                uint64_t ans = print_time.end();
                print_time.begin();
                pair<uint64_t, uint64_t> print_t = merge_count(false);
                printf_array[print_num_listen++] = print_t.first;
                printf_abort[print_num_listen] = print_t.second;
                phase_count[print_num_listen] = dynamic_test_phase;
                throughput_ms = printf_array[print_num_listen-1] / (ans/1000000);
            }
        }
        int old = dup(1);

        #if (CPUBASED==3)
        FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_strife.txt", "w", stdout);
        #endif

        #if (CPUBASED==4)
        #if (BASELINE==1)
            FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_NetSTO.txt", "w", stdout);
        #else
            FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_schism.txt", "w", stdout);
        #endif
        #endif

        #if (CPUBASED==1)
        FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_colocate.txt", "w", stdout);
        #endif

        #if (CPUBASED==2)
        FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_central.txt", "w", stdout);
        #endif

        #ifndef CPUBASED
        FILE* fp = freopen("/home/ljr/AlNiCo-ATC22/AlNiCo_Server/sto/tp_listen_alnico.txt", "w", stdout);
        #endif


        // puts("[Throughput listen], time, throughput(txns/ms)");
        for (uint64_t i = 0; i < print_max; i++) {
            // printf("- %d %d %llu %lf\n", i * gap, phase_count[i], printf_array[i] / gap, 
            //     (printf_abort[i] + 0.01)/ (0.01+printf_abort[i] + printf_array[i])
            // );
            printf("%d %llu\n", i * gap, printf_array[i] / gap);
        }
        // printf("The overall Total: %llu\n", throughput/());
        // puts("[Throughput listen]: end");
        fflush(fp);
        dup2(old, 1);
    }
    inline void begin(int thread_id)
    {
        timer[thread_id]->begin();
    }
    inline uint64_t end(int thread_id)
    {
        uint64_t latency_value = timer[thread_id]->end();
        // printf("lat = %d\n",latency_value);
        latency_bucket[thread_id][min(latency_value / time_u, range - 1)]++;
        return latency_value;
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
        free(ans);
        // fclose(stdout);
        // freopen("/dev/console","w",stdout);
    }
    latency_evaluation_t(int _thread_num)
    {
        dynamic_test_phase = 0;
        throughput = 0;
        throughput_ms = 0;

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
        return;
    }
    ~latency_evaluation_t() { return; }
};

#endif // _latncy_evaluation_
