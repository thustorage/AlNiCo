#ifndef _MYSTORAGE_H__
#define _MYSTORAGE_H__

#define SBUFSIZE 256ULL * 1024ULL * 1024ULL
#define MBSIZE 1 * 1024ULL * 1024ULL

#include "Debug.h"
#include "HugePageAlloc.h"
#include "latency_evaluation.h"
// #include "nic2worker.h"
#include "tpcc_common.h"
#include "tpcc_schema.h"
// #include "tpcc_loader.h"
#include "zipf.h"
#include <bits/stdc++.h>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <libcuckoo/cuckoohash_map.hh>

using namespace std;


typedef uint32_t SIZETYPE;

typedef boost::shared_lock<boost::shared_mutex> read_lock;
typedef boost::unique_lock<boost::shared_mutex> write_lock;
typedef boost::upgrade_lock<boost::shared_mutex> upgrade_lock;
typedef boost::upgrade_to_unique_lock<boost::shared_mutex> upwrite_lock;

extern latency_evaluation_t performance;



// 45 3 2 3 2
#define FREQUENCY_AMALGAMATE 0
#define FREQUENCY_BALANCE 0
#define FREQUENCY_DEPOSIT_CHECKING 0
#define FREQUENCY_SEND_PAYMENT 100
#define FREQUENCY_TRANSACT_SAVINGS 0
#define FREQUENCY_WRITE_CHECK 0

extern uint8_t test_type;
extern volatile SIZETYPE schema_size[10];

struct storage_buffer_t {
    char* storage_buffer;
    uint64_t head;
};

enum class sb_txn_type_t : int {
    amalgamate = 20,
    balance,
    deposit_checking,
    send_payment,
    transact_saving,
    write_check,
};


#ifndef TWOPHASE
#define FREQUENCY_new_order 50
#define FREQUENCY_pay 40
#define FREQUENCY_del 2
#define FREQUENCY_order 4
#define FREQUENCY_stock 4
#else
#define FREQUENCY_new_order 100
#define FREQUENCY_pay 0
#define FREQUENCY_del 0
#define FREQUENCY_order 0
#define FREQUENCY_stock 0
#endif

#define tpcc_txn_type_base 30
enum class tpcc_txn_type_t : int {
    test = tpcc_txn_type_base,
    test_phase_1,
    new_order_phase_main,
    new_order_phase_other,
    new_order,
    pay,
    del,
    order,
    stock,
};



// extern feature_partitiont_t feature_partition; CMAKETODO


#define SINGLE_TX_TEST 6
#define SINGLE_TX_TPCC_NEW_ORDER 7

static inline uint32_t hrd_fastrand(uint64_t* seed)
{
    *seed = *seed * 1103515245 + 2019310834;
    return (uint32_t)(*seed >> 32);
}

#define table_id_pair first
#define table_key_pair second
struct tx_pair{
    uint64_t first;
    uint64_t second;
    pair<uint64_t,uint64_t> to_std_pair(){
        return make_pair(first,second);
    }
};



typedef struct pair<uint64_t,uint64_t> tx_key_t;

inline tx_key_t make_pair_t(uint64_t a, uint64_t b){
    tx_key_t x;
    x.first = a;
    x.second = b;
    return x;
}


#ifdef SMALLBANK
typedef double tx_value_t;
#else
typedef uint64_t tx_value_t;
#endif
struct key_hash {
    size_t operator()(const uint64_t& key) const
    {
        uint64_t __h = ((key & 0xffffffffULL) << 32) | (key >> 32);
        return size_t(__h);
    }
};
typedef uint64_t tx_version_t;

struct small_bank_t {
    volatile int64_t server_lock_id;
    uint8_t lock;
    tx_version_t version;
    tx_value_t value;
};

extern small_bank_t small_bank_count[RANGEOFKEY_STORAGE * 2 + KEY2NODE_MOD * 2];

struct write_entry_t {
    uint8_t server_id;
    tx_key_t key;
    tx_value_t value;
    SIZETYPE size;
    uint64_t value_addr;
};

struct read_entry_t {
    uint8_t server_id;
    tx_key_t key;
    tx_version_t version;
    tx_value_t value;
    // tx_value_t* value_addr;
};
#ifdef USEERIS
struct MAP_entry_t {
    tx_version_t version;
    tx_value_t value;
    uint64_t addr;
    SIZETYPE size;
    uint8_t lock;
    uint8_t write_lock;
    uint8_t read_lock;
    std::set<uint16_t> S;
    volatile int64_t server_lock_id;
};
#else
struct MAP_entry_t {
    tx_version_t version;
    // tx_value_t value;
    uint64_t addr;
    SIZETYPE size;
    // uint8_t lock;
    volatile int64_t server_lock_id;
} __attribute__((packed));
#endif


extern std::unordered_map<uint64_t, MAP_entry_t> storage_map;
extern libcuckoo::cuckoohash_map<uint64_t, MAP_entry_t*> tpcc_map[9][tpccmapsize];
extern uint32_t num_accounts_global, num_hot_global, num_base_hot;

inline int NumWarehouses()
{
    return scale_factor * KEY2NODE_MOD;
}

inline int easy_hash(tx_key_t& key)
{
    // from key to 1000
    // return (key ^ (key >> 31)) % 1024ull;
    // return ((((key * 1103515245 + 31) * 37) >> 23) ^ (key >> 55)) % 1024;
    // return ((uint32_t)((~key) + (key << 18)) ^ key ^ (key >> 31)) % 1024ull ;
    // key = (~key) + (key << 18); // key = (key << 18) - key - 1;
    // key = key ^ (key >> 31);
    // key = key * 21; // key = (key + (key << 2)) + (key << 4);
    // key = key ^ (key >> 11);
    // key = key + (key << 6);
    // key = key ^ (key >> 22);
    // (addr * 1103515247 + 12345) % BUCKET_CNT;
    uint64_t tmp = key.second;
    switch (key.first) {
    case WARE: {
        key.second = 0;
        return (tmp * 3) % tpccmapsize;
        break;
    }
    case DIST: {
        key.second = 0;
        return (tmp * 2) % tpccmapsize;
        break;
    }
    case CUST: {
        key.second = tmp & 0xFFFFFFFFULL;
        tmp = (uint32_t)((MASK_UPPER & tmp) >> 32);
        break;
    }
    case NEWO: {
        key.second = tmp & 0xFFFFFFFFULL;
        tmp = (uint32_t)((MASK_UPPER & tmp) >> 32);
        break;
    }
    case ORDE: {
        key.second = tmp & 0xFFFFFFFFULL;
        tmp = (uint32_t)((MASK_UPPER & tmp) >> 32);
        break;
    }
    case ORLI: {
        key.second = tmp % 150000000uLL;
        tmp = (tmp / 15 / 10000000uLL);
        break;
    }
    case ITEM: {
        key.second = tmp / tpccmapsize;
        return tmp % tpccmapsize;
        break;
    }
    case STOC: {
        key.second = tmp % 100000ull;
        tmp = tmp / 100000ull;
        return (tmp * (tpccmapsize / (scale_factor + 1)) + ((key.second) % (tpccmapsize / (scale_factor+1)))) % tpccmapsize;
        break;
    }
    default: {
        key.second = tmp / tpccmapsize;
        return tmp % tpccmapsize;
        break;
    }
    }

    return (tmp * NumDistrict() + (key.second % NumDistrict())) % tpccmapsize;
}


int key2node(tx_key_t key);
int key2global(tx_key_t key);
int key2thread(tx_key_t key);

tx_value_t get_value(tx_key_t key);
tx_version_t get_version(tx_key_t key);
bool validation_key(tx_key_t key, tx_version_t version, uint16_t id);
bool lock_key(tx_key_t key, uint16_t id, uint64_t shit = 0);
bool free_key(tx_key_t key, uint16_t id, uint64_t shit = 0);
void write_value(tx_key_t key, tx_value_t value);

bool get_value_various_size(tx_key_t key, uint64_t addr, tx_version_t* version, SIZETYPE* size, uint16_t id);
void write_value_various_size(tx_key_t key, uint64_t addr, SIZETYPE size, uint64_t shit = 0);



void init_buffer_thread(uint64_t size);
MAP_entry_t* init_entry(uint64_t entry_type);
void free_entry(uint64_t entry_type);
// for workload
void get_account(uint64_t* seed, uint64_t* acct_id);
void get_two_accounts(uint64_t* seed, uint64_t* acct_id_0, uint64_t* acct_id_1);
void get_account_zipf(zipf_gen_state* state, uint64_t* acct_id);
void get_account_zipf_offset(zipf_gen_state* state, uint64_t* acct_id, uint16_t offset);
void get_two_accounts_zipf(zipf_gen_state* state, uint64_t* acct_id_0, uint64_t* acct_id_1);

uint32_t get_num_gloabl();
void get_account_node(uint64_t* seed, uint64_t* acct_id_0, uint64_t* acct_id_1, uint16_t _node);
void get_account_node_1(uint64_t* seed, uint64_t* acct_id_0, uint16_t _node);
void get_account_node_1_zipf(zipf_gen_state* state, uint64_t* acct_id_0, uint16_t _node);
void get_account_node_1_zipf_col(zipf_gen_state* state, uint64_t* acct_id_0, uint16_t _node, int col);

// for tpcc
void init_insert(tx_key_t key, uint64_t addr, SIZETYPE offset);
uint64_t lock_id(tx_key_t key);

void apply_num_base_hot();
void get_various_account(uint64_t* seed, uint64_t* acct_id, int x);
void print_size(int x);
void print_lock();
#endif