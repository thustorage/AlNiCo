#include "mystorage.h"
#include "Debug.h"
// #include "SwitchTxCommon.h"

// #include "latency_evaluation.h"

std::unordered_map<uint64_t, MAP_entry_t> storage_map;
libcuckoo::cuckoohash_map<uint64_t, MAP_entry_t*> tpcc_map[9][tpccmapsize];
// gp_hash_table<int, int>
// __gnu_pbds::gp_hash_table<uint64_t,uint64_t> tpcc_map[9][tpccmapsize][8];
volatile SIZETYPE schema_size[10];
// boost::shared_mutex mtx[9][tpccmapsize][8];

latency_evaluation_t performance(CLIENT_THREAD);




// std::map<tx_key_t, MAP_entry_t> storage_map;
// index
uint32_t num_accounts_global, num_hot_global, num_base_hot;
uint8_t test_type;
std::string NameTokens[22] = {
    std::string("BAR"),
    std::string("OUGHT"),
    std::string("ABLE"),
    std::string("PRI"),
    std::string("PRES"),
    std::string("ESE"),
    std::string("ANTI"),
    std::string("CALL"),
    std::string("ATIO"),
    std::string("EING"),
    std::string("ALIC"),
    std::string("BOB"),
    std::string("CARO"),
    std::string("DAVE"),
    std::string("EVE"),
    std::string("FRAN"),
    std::string("GRAC"),
    std::string("HANS"),
    std::string("Isabe"),
    std::string("Jason"),
    std::string("Kate"),
    std::string("Louis")
};

thread_local storage_buffer_t s_buf;
uint32_t x, y, z, w;

uint32_t get_num_gloabl()
{
    return num_accounts_global;
}
small_bank_t small_bank_count[RANGEOFKEY_STORAGE * 2 + KEY2NODE_MOD * 2];

int key2node(tx_key_t key)
{
    if (test_type == 0) {
        return (uint64_t)(key.table_key_pair) % KEY2NODE_MOD;
    } else {
        // FIX ME -max client
        return (uint64_t)(key.table_key_pair) % (KEY2NODE_MOD);
    }
}
int key2thread(tx_key_t key)
{
    return ((uint64_t)(key.table_key_pair) / KEY2NODE_MOD) % MAX_THREAD;
}
int key2global(tx_key_t key)
{
    int a = key2node(key);
    int b = key2thread(key);
    return a * MAX_THREAD + b;
}

bool get_value_various_size(tx_key_t key, uint64_t addr, tx_version_t* version, SIZETYPE* size, uint16_t id)
{
    int hash_value = easy_hash(key);
    auto& db_m = tpcc_map[key.table_id_pair][hash_value];

    bool flag;
    MAP_entry_t* x = NULL;
    // if (key.table_id_pair >= 4 && key.table_id_pair <= 6 && MAX_THREAD != 1) {
    //     // auto& db_lock = mtx[key.table_id_pair][hash_value][0];
    //     {
    //         // read_lock rlock(db_lock);
    //         db_m.find(key.table_key_pair, x);
    //         // x = (MAP_entry_t*)(db_m[key.table_key_pair]);
    //     }
    // } else {
    //     db_m.find(key.table_key_pair, x);

    //     // x = (MAP_entry_t*)(db_m[key.table_key_pair]);
    // }
    db_m.find(key.table_key_pair, x);
    *version = x->version;
    flag = (x->server_lock_id != -1 && x->server_lock_id != id);
    // return value for read
    if (addr != 0) {
        memcpy((char*)addr, (char*)(x->addr), x->size);
        *size = x->size;
    }
    // Debug::notifyError("id,key=(%d,%d), size=%d lock=%d server=%d",key.first,key.second,x.size,x.lock,x.server_lock_id);
    return flag;
}

tx_value_t get_value(tx_key_t key)
{
#ifdef SMALLBANK

    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;

    if (local_key >= RANGEOFKEY_STORAGE * 2) {
        printf("%d %d\n", local_key, RANGEOFKEY_STORAGE * 2);
    }
    return small_bank_count[local_key].value;
#elif defined TPCCBENCH
    exit(-1);
    return 1;
#else
    // Debug::notifyError("read key %llu",key);
    if (storage_map.count(key.table_key_pair) == 0) {
        storage_map[key.table_key_pair].lock = false;
        storage_map[key.table_key_pair].version = 100;
        storage_map[key.table_key_pair].value = 100; // 100 yuan every one
        storage_map[key.table_key_pair].server_lock_id = -1;
    }
    return storage_map[key.table_key_pair].value;
#endif
}

tx_version_t get_version(tx_key_t key)
{
#ifdef SMALLBANK
    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;
    return small_bank_count[local_key].version;
#elif defined TPCCBENCH
    exit(-1);
    return 1;
#else
    return storage_map[key.table_key_pair].version;
#endif
}

void print_lock(){
    printf("lock = %d\n",small_bank_count[0].server_lock_id);
}

bool validation_key(tx_key_t key, tx_version_t version, uint16_t id)
{
#ifdef SMALLBANK
    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;
    if (small_bank_count[local_key].lock == true && id != small_bank_count[local_key].server_lock_id) {
        // Debug::notifyError("validation key %llu fail i am %d ,lock is %d",key,id,small_bank_count[local_key].version);
        return small_bank_count[local_key].version == version;
    }
    if (small_bank_count[local_key].version != version) {
        // Debug::notifyError("validation key %llu fail i am %d ,vesion is %d, now the version is %d",key,id,version,small_bank_count[local_key].version);
        return false;
    }
    // if (key<150) return false;
    return true;
#elif defined TPCCBENCH
    uint64_t read_version;
    if (get_value_various_size(key, 0, &read_version, NULL, id)) {
        return false;
    }
    // if (read_version != version)
    //     Debug::notifyError("%s:[%d,%llu] %d != %d", __func__, key.first, key.second, read_version, version);
    return read_version == version;
#else
    if (storage_map[key.table_key_pair].lock == true && id != storage_map[key.table_key_pair].server_lock_id) {
        // Debug::notifyError("validation key %llu fail i am %d ,lock is %d",key,id,storage_map[key].server_lock_id);
        return storage_map[key.table_key_pair].version == version;
    }
    if (storage_map[key.table_key_pair].version != version) {
        // Debug::notifyError("validation key %llu fail i am %d ,vesion is %d, now the version is %d",key,id,version,storage_map[key].version);
        return false;
    }
    // if (key<150) return false;
    return true;
#endif
}
uint64_t lock_id(tx_key_t key)
{
    return small_bank_count[key.table_key_pair].server_lock_id;
}

bool lock_key(tx_key_t key, uint16_t id, uint64_t shit)
{
#ifdef SMALLBANK
    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;
    if (__sync_bool_compare_and_swap(&small_bank_count[local_key].server_lock_id, -1, id)) {
        small_bank_count[local_key].lock = true;
#if (DEBUG_ON)
        Debug::notifyError("---------lock key %llu %llu i am %d-- okkkkkkkkkkkk", local_key, key.table_key_pair, id);
#endif
        return true;
    } else {
#if (DEBUG_ON)
        Debug::notifyError("---------lock key fail %llu %llu i am %d but key is %d ", local_key, key.table_key_pair, id, small_bank_count[local_key].server_lock_id);
#endif
        return small_bank_count[local_key].server_lock_id == id;
        // return false;
        // Debug::notifyError("---------lock key %llu i am %d-- fail key is %d",key,id,small_bank_count[local_key].server_lock_id);
        // assert(small_bank_count[local_key].server_lock_id != id);
    }
#elif defined TPCCBENCH
    int hash_value = easy_hash(key);
    auto& db_m = tpcc_map[key.table_id_pair][hash_value];
#if (MAX_THREAD == 1)
    if (db_m.count(key.table_key_pair) != 0) {
        MAP_entry_t* ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        if (shit != 0) {
            (*(uint64_t*)shit) = (uint64_t)ptr;
        }
        if (__sync_bool_compare_and_swap(&ptr->server_lock_id, -1, id)) {
            return true;
        } else {
            // Debug::notifyError("ptr->server_lock_id=%d, id=%llu, key=%llu", ptr->server_lock_id,key.table_id_pair,key.table_key_pair);
            // Debug::notifyError("ptr->version%llu addr%llu size%lu\n",ptr->version,ptr->addr,ptr->size);
            // #if (DEBUG_ON)
            //     Debug::notifyError("ptr->server_lock_id=%d", ptr->server_lock_id);
            // #endif
            return false;
        }

    } else {
        MAP_entry_t* x = init_entry(key.table_id_pair);
        x->server_lock_id = id;
        db_m[key.table_key_pair] = (uint64_t)x;
        if (shit != 0) {
            (*(uint64_t*)shit) = (uint64_t)x;
        }
        return true;
    }

#else
    MAP_entry_t* ptr = NULL;
    if (key.table_id_pair >= 4 && key.table_id_pair <= 6) {
        // auto& db_lock = mtx[key.table_id_pair][hash_value][0];
        // db_m[key.table_key_pair] = (uint64_t)x;
        // return true;
        // {
        // uint64_t s;
        // upgrade_lock uplock(db_lock);
        if (!db_m.find(key.table_key_pair, ptr)) {
            // upwrite_lock wlock(uplock);
            // if (db_m.count(key.table_key_pair) == 0) {
            MAP_entry_t* x = init_entry(key.table_id_pair);
            x->server_lock_id = id;
            db_m.insert(key.table_key_pair, x);
            if (shit != 0) {
                (*(uint64_t*)shit) = (uint64_t)x;
            }
            return true;
        }
        // }
        // if (ptr == NULL) {

        //     // MAP_entry_t* shitp = (MAP_entry_t*)(*(uint64_t*)shit);
        //     // printf("111 %d %d\n", shit->addr, shit->version);
        //     return true;
        // } else
        //     free_entry(key.table_id_pair);

    } else {
        db_m.find(key.table_key_pair, ptr);
        // ptr = (MAP_entry_t*)(db_m[key.table_key_pair]);
    }
    if (shit != 0) {
        (*(uint64_t*)shit) = (uint64_t)ptr;
    }
    // MAP_entry_t* shitp = (MAP_entry_t*)(*(uint64_t*)shit);
    // printf("111 %d %d\n", shit->addr, shit->version);
    if (__sync_bool_compare_and_swap(&ptr->server_lock_id, -1, id)) {
        return true;
    } else {
        return false;
    }
#endif

    // Debug::notifyError("insert new key (%d,%llu)", key.table_id_pair, key.table_key_pair);
    // assert(id != 0);

    // printf("key == %llu %llu\n",key.table_key_pair, x.addr);

#else
    if (__sync_bool_compare_and_swap(&storage_map[key.table_key_pair].server_lock_id, -1, id)) {
        storage_map[key.table_key_pair].lock = true;
        return true;
    } else {
        return false;
    }
#endif
}

bool free_key(tx_key_t key, uint16_t id, uint64_t shit)
{
    // Debug::notifyError("---------free key %llu i am %d-- ",key,id);
    // Debug::notifyError("free key %llu",key);
    /*
    if (storage_map[key].server_lock_id == id){
        storage_map[key].lock = false;
    }
    return true;
    */

#ifdef SMALLBANK
    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;
    if (__sync_bool_compare_and_swap(&small_bank_count[local_key].server_lock_id, id, -1)) {
#if (DEBUG_ON)
        Debug::notifyError("---------free key %llu %llu i am %d-- ", local_key, key.table_key_pair, id);
#endif
        small_bank_count[local_key].lock = false;
        return true;
    }
    // puts("??????");
    return false;
#elif defined TPCCBENCH

    MAP_entry_t* ptr = NULL;
    if (shit != 0) {
        ptr = (MAP_entry_t*)shit;
    } else {
        int hash_value = easy_hash(key);
        auto& db_m = tpcc_map[key.table_id_pair][hash_value];
        // if (MAX_THREAD != 1) {
        //     if (key.table_id_pair >= 4 && key.table_id_pair <= 6) {
        //         // auto& db_lock = mtx[key.table_id_pair][hash_value][0];
        //         {
        //             // read_lock rlock(db_lock);
        //             db_m.find(key.table_key_pair, ptr);
        //             // ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        //         }
        //     } else {
        //         db_m.find(key.table_key_pair, ptr);
        //         // ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        //     }

        // } else {
        //     db_m.find(key.table_key_pair, ptr);
        //     // ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        // }
        db_m.find(key.table_key_pair, ptr);
    }
    ptr->server_lock_id = -1;
    return true;
#else
    if (__sync_bool_compare_and_swap(&storage_map[key.table_key_pair].server_lock_id, id, -1)) {
        storage_map[key.table_key_pair].lock = false;
        return true;
    }
#endif
}

void write_value_various_size(tx_key_t key, uint64_t addr, SIZETYPE size, uint64_t shit)
{
    uint64_t tmp_addr;
    MAP_entry_t* ptr = NULL;
    // puts("writes");
    if (shit != 0) {
        ptr = (MAP_entry_t*)shit;
    } else {
        int hash_value = easy_hash(key);
        auto& db_m = tpcc_map[key.table_id_pair][hash_value];

        // if (key.table_id_pair >= 4 && key.table_id_pair <= 6 && MAX_THREAD != 1) {

        //     // auto& db_lock = mtx[key.table_id_pair][hash_value][0];
        //     // read_lock rlock(db_lock);
        //     db_m.find(key.table_key_pair, ptr);
        //     // ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        //     // Debug::notifyError("key(%d,%d) size = %d copy from %llu to %llu", key.first, key.second, size,
        //     //     addr, (ptr->addr) );
        // } else {
        //     db_m.find(key.table_key_pair, ptr);
        //     // ptr = (MAP_entry_t*)db_m[key.table_key_pair];
        // }
        db_m.find(key.table_key_pair, ptr);
    }
    ptr->version += 1;
    ptr->server_lock_id = -1;
    tmp_addr = ptr->addr;
    memcpy((char*)(tmp_addr), (char*)addr, size);
}

void write_value(tx_key_t key, tx_value_t value)
{
    // Debug::notifyError("write key %llu = %llu %llu",key,value,storage_map[key].version+1);

#ifdef SMALLBANK
    uint64_t local_key = key.table_key_pair / KEY2NODE_MOD;
    small_bank_count[local_key].version += 1;
    small_bank_count[local_key].value = value;

    return;
#elif defined TPCCBENCH
    exit(-1);
    return;
#else
    storage_map[key.table_key_pair].version += 1;
    storage_map[key.table_key_pair].value = value;
    return;
#endif
}
void init_buffer_thread(uint64_t size)
{
#ifdef TPCCBENCH
    s_buf.storage_buffer = (char*)malloc(size);
    s_buf.head = (uint64_t)s_buf.storage_buffer;
    memset(s_buf.storage_buffer, 0, size);
#endif
}



MAP_entry_t* init_entry(uint64_t entry_type)
{
    if (s_buf.head + 1024 - (uint64_t)s_buf.storage_buffer >= SBUFSIZE * 2) {
        Debug::notifyError("buf is not enough");
        fflush(stdout);
        assert(0);
    }
    uint64_t addr = s_buf.head;
    MAP_entry_t* x = (MAP_entry_t*)addr;
    x->version = 0;
    x->server_lock_id = -1;
    x->addr = s_buf.head + sizeof(MAP_entry_t) + 4;
    x->size = schema_size[entry_type];
    s_buf.head += (schema_size[entry_type] + sizeof(MAP_entry_t) + 4 + 63) & (~63);
    return (MAP_entry_t*)addr;
}
void free_entry(uint64_t entry_type)
{
    s_buf.head -= (schema_size[entry_type] + sizeof(MAP_entry_t) + 4 + 63) & (~63);
    return;
}

void apply_num_base_hot()
{
    num_base_hot = (num_base_hot + num_hot_global / 2) % num_accounts_global;
}
void get_various_account(uint64_t* seed, uint64_t* acct_id, int x)
{
    if (x == 0) {
        *acct_id = hrd_fastrand(seed) % num_accounts_global;
    } else {
        if (hrd_fastrand(seed) % 100 < TX_HOT) {
            *acct_id = (((hrd_fastrand(seed) % (num_accounts_global - MAX_THREAD - 1)) / (MAX_THREAD)) * (MAX_THREAD) + x);
        } else {
            *acct_id = hrd_fastrand(seed) % num_accounts_global;
        }
    }
}
void get_account_zipf_offset(zipf_gen_state* state, uint64_t* acct_id, uint16_t offset)
{
    *acct_id = (mehcached_zipf_next(state) + offset * num_accounts_global / MAX_THREAD) % num_accounts_global;
}

void get_account_zipf(zipf_gen_state* state, uint64_t* acct_id)
{
    *acct_id = mehcached_zipf_next(state) % num_accounts_global;
}

// void get_account_zipf_node(zipf_gen_state* state, uint64_t* acct_id){
//     *acct_id = mehcached_zipf_next(state) % num_accounts_global;
// }
void get_two_accounts_zipf(zipf_gen_state* state, uint64_t* acct_id_0, uint64_t* acct_id_1)
{
    *acct_id_0 = mehcached_zipf_next(state) % num_accounts_global;
    *acct_id_1 = mehcached_zipf_next(state) % num_accounts_global;
    while (*acct_id_1 == *acct_id_0) {
        *acct_id_1 = mehcached_zipf_next(state) % num_accounts_global;
    }
}

void get_account(uint64_t* seed, uint64_t* acct_id)
{
    if (hrd_fastrand(seed) % 100 < TX_HOT) {
        *acct_id = hrd_fastrand(seed) % num_hot_global;
    } else {
        *acct_id = hrd_fastrand(seed) % num_accounts_global;
    }
}
void get_two_accounts(uint64_t* seed, uint64_t* acct_id_0, uint64_t* acct_id_1)
{
    if (hrd_fastrand(seed) % 100 < TX_HOT) {
        *acct_id_0 = hrd_fastrand(seed) % num_hot_global;
        *acct_id_1 = hrd_fastrand(seed) % num_hot_global;
        while (*acct_id_1 == *acct_id_0) {
            *acct_id_1 = hrd_fastrand(seed) % num_hot_global;
        }
    } else {
        *acct_id_0 = hrd_fastrand(seed) % num_accounts_global;
        *acct_id_1 = hrd_fastrand(seed) % num_accounts_global;
        while (*acct_id_1 == *acct_id_0) {
            *acct_id_1 = hrd_fastrand(seed) % num_accounts_global;
        }
    }
}
void get_account_node(uint64_t* seed, uint64_t* acct_id_0, uint64_t* acct_id_1, uint16_t _node)
{
    *acct_id_0 = ((hrd_fastrand(seed) % RANGEOFKEY_STORAGE) * KEY2NODE_MOD + _node) % num_accounts_global;
    *acct_id_1 = ((hrd_fastrand(seed) % RANGEOFKEY_STORAGE) * KEY2NODE_MOD + _node) % num_accounts_global;
    while (*acct_id_1 == *acct_id_0) {
        *acct_id_1 = ((hrd_fastrand(seed) % RANGEOFKEY_STORAGE) * KEY2NODE_MOD + _node) % num_accounts_global;
    }
    return;
}

void get_account_node_1_zipf(zipf_gen_state* state, uint64_t* acct_id_0, uint16_t _node)
{
    *acct_id_0 = (mehcached_zipf_next(state) % (RANGEOFKEY_STORAGE-2) ) * KEY2NODE_MOD + _node;
    // *acct_id_0 = ((hrd_fastrand(seed) % (RANGEOFKEY_STORAGE-2)) * KEY2NODE_MOD + _node);
    // *acct_id_0 = (0 % (RANGEOFKEY_STORAGE-2)) * KEY2NODE_MOD + _node;
    return;
}

void get_account_node_1_zipf_col(zipf_gen_state* state, uint64_t* acct_id_0, uint16_t _node, int col){
    *acct_id_0 = ((mehcached_zipf_next(state) % ((RANGEOFKEY_STORAGE - 2 * MAX_THREAD)/MAX_THREAD)) * MAX_THREAD + col ) * KEY2NODE_MOD + _node;
    return;
}

void get_account_node_1(uint64_t* seed, uint64_t* acct_id_0, uint16_t _node)
{
    // *acct_id_0 = ((hrd_fastrand(seed) % (RANGEOFKEY_STORAGE-2)) * KEY2NODE_MOD + _node);
    *acct_id_0 = ((RANGEOFKEY_STORAGE-2) * KEY2NODE_MOD + _node);
    return;
}

void init_insert(tx_key_t key, uint64_t addr, SIZETYPE offset)
{
    int hash_value = easy_hash(key);
    // printf("%u\n",hash_value);
    auto& db_m = tpcc_map[key.table_id_pair][hash_value];
    // if (key.first == 8){
    //     Debug::notifyError("insert key == %d",key.table_key_pair);
    // }
    // warehouse::value* xx = (warehouse::value*)addr;
    // // Debug::notifyError("init %d %d", key.first, key.second);
    // uint64_t* test = (uint64_t*)xx;
    // for (int i = 0; i < 10; i++) {
    //     printf("%ull ", test[i]);
    // }
    // puts("");
    // std::cout << xx->w_city << std::endl;
    offset = (offset + 63) / 64 * 64;
    MAP_entry_t* x = (MAP_entry_t*)(addr + offset);
    x->version = 0;
    x->addr = addr;
    x->server_lock_id = -1;
    x->size = schema_size[key.table_id_pair];
    // printf("insert %lu,%lu\n",hash_value, key.table_key_pair);
    db_m.insert(key.table_key_pair, x);
    // puts("?");
    // db_m[key.table_key_pair] = (uint64_t)x;
}

void print_size(int x)
{
    int old = dup(1);

    FILE* fp = freopen(("map_size20" + to_string(x) + ".txt").c_str(), "w", stdout);
    // puts("[Throughput listen]: begin");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < tpccmapsize; j++) {
            printf("[%d,%d] %d %lf\n", i, j, tpcc_map[i][j].size(), tpcc_map[i][j].load_factor());
        }
    }
    fflush(fp);
    dup2(old, 1);
}