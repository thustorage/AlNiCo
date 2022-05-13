

#include "nictxn_rpc.h"
#include "timer.h"

uint64_t test1;
uint64_t test1_array[6] = {32, 64, 128, 256, 512, 1024};

inline int hash_k2f_ycsb_t(uint32_t col, uint32_t key)
{
    // printf("%d %d %d\n", col, key, col * (FEATURE_SIZE / YCSBCOL) + key % (FEATURE_SIZE / YCSBCOL));
    // return col+10;
    return col * (test1 / YCSBCOL) + (key % (test1 / YCSBCOL));
    // return col * (FEATURE_SIZE / YCSBCOL) + (key % (FEATURE_SIZE / YCSBCOL));
}

inline int hash_k2f_2(tx_key_t key)
{
    return feature_partition.offset[key.first] + (key.second % feature_partition.size[key.first]) ;
}




#define TEST_TO_YCSB_READF(xx, my_id, my_key)   \
    ycsb_col_arrays[xx] = my_id;                    \
    ycsb_key_arrays[xx] = my_key;                   \
    feature_offset = hash_k2f_ycsb_t(my_id, my_key); \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8)); \
    txs[id].insert(make_pair(my_id, my_key)); 


#define TEST_READF(k_pair, my_id, my_key)                 \
    k_pair[(*k_pair##_count)].table_id_pair = my_id;        \
    k_pair[(*k_pair##_count)].table_key_pair = my_key;      \
    feature_offset = hash_k2f_2(k_pair[(*k_pair##_count)++]); \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8)); \
    txs[id].insert(make_pair(my_id, my_key)); 
    // printf("%d %d\n",my_id, my_key);

#define TEST_ARGS(args_array, value) \
    args_array[args_array##_count++] = value;

nocc::util::fast_random random_generator;
uint8_t feature[128];
uint32_t x_rand, y_rand, z_rand, w_rand;
void* my_ctx;
int* main_read_count;
tx_key_t* main_read;
uint64_t* remote_stocks;
int* remote_item_ids;
int* remote_reply;
int* local_stocks;
int* local_item_ids;
uint* local_supplies;
uint* remote_supplies;
float* i_price;
int num_remote_stocks, num_local_stocks;
order_line::value* remote_v_ol;
uint tx_main_partition;
uint warehouse_id;
uint districtID;
uint customerID;
uint numItems;
bool allLocal;
uint64_t c_key;
uint args[100];
int args_count;
std::set<uint32_t> ycsb_set;
uint16_t* ycsb_col_arrays;
uint32_t* ycsb_key_arrays;
uint32_t test_feature_size;
double zipf[8] = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4};
sampling::StoRandomDistribution<>* dd;

uint64_t shit_ans = 0;
uint64_t shit_real = 0;
inline uint32_t xorshift128_test(void)
{
    // return rand();
    uint32_t t = x_rand ^ (x_rand << 11);
    x_rand = y_rand;
    y_rand = z_rand;
    z_rand = w_rand;
    return w_rand = w_rand ^ (w_rand >> 19) ^ t ^ (t >> 8);
}

std::set<pair<uint64_t,uint64_t>> txs[2];
uint8_t txfeature[2][128];



int main(){
    memset(feature, 0, sizeof(feature));
    init_feature_partition();
    Timer tt;// = new Timer();
    tx_key_t x;
    x.first = 4;
    tt.begin();
    uint32_t feature_offset;
    for (int i = 0; i < 100000; i++){
        x.second = i;
        feature_offset = hash_k2f_2(x); 
        feature[feature_offset / 8] |= 1U << ((feature_offset % 8));
    }
    uint64_t ans = tt.end();
    printf("ans=%lf\n",ans / 100000.0);
    
    return 0;
}