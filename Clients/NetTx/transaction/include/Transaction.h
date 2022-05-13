
#ifndef _TRANSACTION_H__
#define _TRANSACTION_H__

#include "Debug.h"
#include "SwitchTxCommon.h"
#include "mystorage.h"
#include "rpc_struct.h"
#include "tpcc_loader.h"
#include <boost/functional/hash.hpp>
#define CTXBUFFER 2048
#define YCSBBUF (YCSBSIZE * COL_WIDTH)
#define WBUFFER 3072
#define MAX_REQ_NUM 300

// using namespace sampling; CMAKETODO:

#define MAX_ITEM 15

#define NEW_CTX_ARRAY(type, name, ctx, coffset, len) \
    name = (type*)((uint64_t)ctx + coffset);         \
    coffset += (len) * sizeof(type);


// feature format [write | read]
#define ADD_TO_YCSB_WRITEF(xx, my_id, my_key)   \
    ycsb_col_arrays[xx] = my_id;                    \
    ycsb_key_arrays[xx] = my_key;                   \
    feature_offset = hash_k2f_ycsb(my_id, my_key); \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8));


#define ADD_TO_WRITE(k_pair, my_id, my_key)           \
    k_pair[(*k_pair##_count)].table_id_pair = my_id; \
    k_pair[(*k_pair##_count)++].table_key_pair = my_key;

#define ADD_TO_WRITEF(k_pair, my_id, my_key)                 \
    k_pair[(*k_pair##_count)].table_id_pair = my_id;        \
    k_pair[(*k_pair##_count)].table_key_pair = my_key;      \
    feature_offset = hash_k2f(k_pair[(*k_pair##_count)++]); \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8));

#define ADD_TO_YCSB_WRITEF(xx, my_id, my_key)   \
    ycsb_col_arrays[xx] = my_id;                    \
    ycsb_key_arrays[xx] = my_key;                   \
    feature_offset = hash_k2f_ycsb(my_id, my_key); \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8));


#define ADD_TO_YCSB_READF(xx, my_id, my_key)   \
    ycsb_col_arrays[xx] = my_id;                    \
    ycsb_key_arrays[xx] = my_key;                   \
    feature_offset = hash_k2f_ycsb(my_id, my_key) + FEATURE_SIZE;  \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8));

#define ADD_TO_READF(k_pair, my_id, my_key)                 \
    k_pair[(*k_pair##_count)].table_id_pair = my_id;        \
    k_pair[(*k_pair##_count)].table_key_pair = my_key;      \
    feature_offset = hash_k2f(k_pair[(*k_pair##_count)++]) + FEATURE_SIZE; \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8));


#define ADD_TO_YCSB_WRITE_BIT(i, write_bits) \
    write_bits |= (1ull << i);

#define ADD_TO_ARGS(args_array, value) \
    args_array[args_array##_count++] = value;

#define ADD_TO_YCSB_READ(xx, my_id, my_key) \
    ycsb_col_arrays[xx] = my_id;            \
    ycsb_key_arrays[xx] = my_key;

void initstorage(uint8_t node_id, uint8_t _test_type = 0);

class switch_tx {
public:
    read_entry_t tmp_read;
    write_entry_t tmp_write;
    // Rpc* rpc; // CMAKETODO:
    uint16_t req_num_wait;
    uint64_t ycsb_count;
    uint16_t read_server; // node
    uint16_t write_server; // node
    uint32_t read_server_eris[KEY2NODE_MOD]; // thread - node
    uint32_t write_server_eris[KEY2NODE_MOD]; // thread - node
    void* my_ctx;
    void* ycsb_ctx;
    uint64_t ctx_offset;
    // for nic
    int nic;
    void* w_buffer;
    uint64_t w_offset;

    std::set<uint32_t> ycsb_set;

    struct zipf_gen_state state;
    int wait_cnt;
    uint8_t tx_id;
    uint64_t global_id;
    uint8_t thread_id;
    uint8_t node_id;

    uint64_t seed;
    nocc::util::fast_random random_generator;

    std::unordered_map<tx_key_t, read_entry_t, boost::hash<tx_key_t>> read_set;
    std::unordered_map<tx_key_t, write_entry_t, boost::hash<tx_key_t>> write_set;
    // std::map<tx_key_t, read_entry_t> read_set;
    // std::map<tx_key_t, write_entry_t> write_set;
    uint8_t req_num[MAX_SERVER];
    uint8_t read_num[MAX_SERVER];
    uint8_t write_num[MAX_SERVER];
    uint8_t req_pair_addr[MAX_SERVER];
    reqPair req_pair[MAX_REQ_NUM * 2 + MAX_SERVER];
    uint8_t all_req_num;
    SwitchTXMessage m;
    SwitchTXMessage reply;
    ErisTXMessage erism;
    uint16_t read_server_reply;
    uint16_t write_server_reply;
    uint8_t magic;
    int single_node_id;
    
    // uint64_t shit1[8];
    uint32_t single_rpc_id;
    // uint64_t shit2[8];

    bool is_busy;
    int retry_count;
    bool is_retry;
    int retry_type;

    int tpcc_latency_type = -1;
    int now_phase;
    bool eris_tx_flag;
    bool switch_tx_flag;
    // for normal TX
    bool normal_tx_flag;


    
    uint8_t counter;
    uint32_t x_rand, y_rand, z_rand, w_rand;
    inline uint32_t xorshift128(void)
    {
        // return rand();
        uint32_t t = x_rand ^ (x_rand << 11);
        x_rand = y_rand;
        y_rand = z_rand;
        z_rand = w_rand;
        return w_rand = w_rand ^ (w_rand >> 19) ^ t ^ (t >> 8);
    }

    void start()
    {
        // if (is_retry) {
        //     print_map();
        //     exit(0);
        // }
        read_set.clear();
        write_set.clear();
        read_server = 0;
        write_server = 0;
        memset(read_server_eris, 0, sizeof(read_server_eris));
        memset(write_server_eris, 0, sizeof(write_server_eris));

        read_server_reply = 0;
        write_server_reply = 0;
        req_num_wait = 0;
        memset(req_num, 0, sizeof(req_num));
        memset(read_num, 0, sizeof(read_num));
        memset(write_num, 0, sizeof(write_num));
        memset(req_pair_addr, 0, sizeof(req_pair_addr));
        all_req_num = 0;
        is_busy = true;
        if ((++magic) == 0) {
            ++magic;
        }
        // printf("%s:%d\n",__func__,magic);
        ctx_offset = 0;
        w_offset = 0;
        // rpc->init(txid);
    }

    
    switch_tx(int _tx_id, uint64_t _global_id, uint8_t _node_id, uint8_t _thread_id)
    {
        ycsb_count = 0;
        // rpc = _rpc;
        tx_id = (uint8_t)_tx_id;
        node_id = _node_id;
        thread_id = _thread_id;
        global_id = node_id * CLIENT_THREAD + thread_id;
        is_busy = false;
        double theta = ZIPFTHETA;
        my_ctx = (void*)malloc(CTXBUFFER);
#ifdef YCSBplusT
        ycsb_ctx = (void*)malloc(YCSBBUF);
#endif

        ctx_offset = 0;
        w_buffer = (void*)malloc(WBUFFER);
        w_offset = 0;

        // m.switch_slot = rpc->generate_slot_id(tx_id); CMAKETODO:
        magic = _global_id;
        is_retry = false;
        srand(global_id * 100 + _tx_id);
        seed = rand() * 19210817 + global_id * (rand() + 10) + _tx_id;
        x_rand = seed;
        y_rand = rand() + seed;
        z_rand = _global_id;
        uint64_t tx_seed = (_global_id * 1921081719210817ull + _tx_id * 997 + _tx_id * 17) % (1UL << 48);
        w_rand = tx_seed;
        if (theta < 1. || theta >= 40.) {
#ifdef CROSSALL
            mehcached_zipf_init(&state, RANGEOFKEY_STORAGE, theta, tx_seed);
#else
            mehcached_zipf_init(&state, get_num_gloabl(), theta, tx_seed);
#endif
        }
        random_generator.set_seed0(tx_seed);

        start();

        is_busy = false;
    }

    void tx_fail()
    {
        is_busy = false;
        if ((++magic) == 0) {
            ++magic;
        }
        // printf("%s:%d\n",__func__,magic);
        is_retry = true;
    }

    void tx_ok()
    {
        retry_count = 0;
        is_busy = false;
        if ((++magic) == 0) {
            ++magic;
        }
        // printf("%s:%d\n",__func__,magic);
        is_retry = false;
    }
    virtual void sys_execution(uint64_t call_type, void* ctx)
    {
        exit(-1);
        return;
    }
    virtual void do_commit_eris(ErisTXMessage* rec_m){
        exit(-1);
        return;
    }
    virtual bool check_eris(ErisTXMessage* rec_m){
        exit(-1);
        return true;
    }
    void execution(uint64_t call_type, void* ctx);

    int* main_read_count;
    tx_key_t* main_read;

    uint16_t* ycsb_col_arrays;
    uint32_t* ycsb_key_arrays;
    uint64_t write_bits;

    uint args[100];
    int args_count;
    uint64_t* remote_stocks;
    int* remote_item_ids;
    int* remote_reply;
    int* local_stocks;
    int* local_item_ids;
    uint* local_supplies;
    uint* remote_supplies;
    uint tx_main_partition;
    uint warehouse_id;
    uint districtID;
    uint customerID;
    uint numItems;
    bool allLocal;
    uint64_t c_key;
    float* i_price;
    int num_remote_stocks, num_local_stocks;
    order_line::value* remote_v_ol;
    int32_t my_next_o_id;
    unsigned warehouse_id_start_, warehouse_id_end_;

    virtual void do_after_read(uint64_t call_type, replyPair reply, void* ctx)
    {
        exit(-1);
        return;
    }
    virtual void do_commit()
    {
        exit(-1);
        return;
    }
    virtual void do_commit_fixed_size()
    {
        exit(-1);
        return;
    }
    virtual void do_commit_various_size()
    {
        exit(-1);
        return;
    }

    void tx_alloc(uint64_t* addr, uint32_t size)
    {
        (*addr) = (uint64_t)w_buffer + w_offset;
        w_offset += size;
        // assert(w_offset<=WBUFFER);
    }


};

#endif
