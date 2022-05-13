

#include "nictxn_rpc.h"

uint64_t test1;
uint64_t test1_array[6] = {32, 64, 128, 256, 512, 1024};

inline int hash_k2f_ycsb_t(uint32_t col, uint32_t key)
{
    // printf("%d %d %d\n", col, key, col * (FEATURE_SIZE / YCSBCOL) + key % (FEATURE_SIZE / YCSBCOL));
    // return col+10;
    return col * (test1 / YCSBCOL) + (key % (test1 / YCSBCOL));
    // return col * (FEATURE_SIZE / YCSBCOL) + (key % (FEATURE_SIZE / YCSBCOL));
}

inline int hash_k2f_2(pair<uint64_t, uint64_t> key)
{
    // return (key.second * 10 + key.first) % 512;
    return hash_k2f_ycsb_t(key.first, key.second);
    return key.second % test1;
    // return (key.first * 512/10 + key.second % (512/10));
    // return feature_partition.offset[key.first] * test1 / 512 + (key.second % (feature_partition.size[key.first] * test1 / 512)) ;

    return feature_partition.offset[key.first] + (key.second % feature_partition.size[key.first]);
}

#define TEST_TO_YCSB_READF(xx, my_id, my_key)                    \
    ycsb_col_arrays[xx] = my_id;                                 \
    ycsb_key_arrays[xx] = my_key;                                \
    feature_offset = hash_k2f_ycsb_t(my_id, my_key);             \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8)); \
    txs[id].insert(make_pair(my_id, my_key));

#define TEST_READF(k_pair, my_id, my_key)                        \
    k_pair[(*k_pair##_count)].table_id_pair = my_id;             \
    k_pair[(*k_pair##_count)].table_key_pair = my_key;           \
    feature_offset = hash_k2f_2(k_pair[(*k_pair##_count)++]);    \
    feature[feature_offset / 8] |= 1U << ((feature_offset % 8)); \
    txs[id].insert(make_pair(my_id, my_key));
// printf("%d %d\n",my_id, my_key);

#define TEST_ARGS(args_array, value) \
    args_array[args_array##_count++] = value;

nocc::util::fast_random random_generator;
uint8_t feature[128];
uint32_t x_rand, y_rand, z_rand, w_rand;
void *my_ctx;
int *main_read_count;
tx_key_t *main_read;
uint64_t *remote_stocks;
int *remote_item_ids;
int *remote_reply;
int *local_stocks;
int *local_item_ids;
uint *local_supplies;
uint *remote_supplies;
float *i_price;
int num_remote_stocks, num_local_stocks;
order_line::value *remote_v_ol;
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
uint16_t *ycsb_col_arrays;
uint32_t *ycsb_key_arrays;
uint32_t test_feature_size;
double zipf[8] = {0.0, 0.2, 0.4, 0.6, 0.8, 1.1, 1.2, 1.4};
sampling::StoRandomDistribution<> *dd;

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

std::set<pair<uint64_t, uint64_t>> txs[2];
uint8_t txfeature[2][128];

inline void ycsb_test(int id)
{
    txs[id].clear();
    int x = RandomNumber(random_generator, 1, 20) - 1;
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    uint16_t nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    // nic = x;
    args_count = 0;
    uint64_t ctx_offset = 0;
    NEW_CTX_ARRAY(uint32_t, ycsb_key_arrays, my_ctx, ctx_offset, YCSBSIZE);
    NEW_CTX_ARRAY(uint16_t, ycsb_col_arrays, my_ctx, ctx_offset, YCSBSIZE);

    ycsb_set.clear();
    bool is_write;
    uint16_t ycsb_write_num = 0;
    for (int i = 0; i < YCSBSIZE; i++)
    {
        uint32_t key = dd->sample();
        while (ycsb_set.find(key) != ycsb_set.end())
        {
            key = dd->sample();
        }
        ycsb_set.insert(key);
        TEST_TO_YCSB_READF(i, nic, key);
    }

    memcpy(txfeature[id], feature, 64);
    return;
}
inline void new_order_test(int id)
{
    // puts("1?");
    txs[id].clear();
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    tx_main_partition = 0; // the only one partition
    uint16_t args_count = 0;

    uint64_t ctx_offset = 0;

    uint16_t warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    uint16_t warehouse_id_end_ = GetEndWarehouse(tx_main_partition);

    uint16_t warehouse_id = xorshift128_test() % scale_factor + 1;
    uint16_t districtID = xorshift128_test() % 10 + 1;

    uint16_t customerID = GetCustomerId(random_generator);                  // +++++
    uint16_t c_key = makeCustomerKey(warehouse_id, districtID, customerID); // +++++

    NEW_CTX_ARRAY(int, main_read_count, my_ctx, ctx_offset, 1)
    NEW_CTX_ARRAY(tx_key_t, main_read, my_ctx, ctx_offset, MAX_ITEM * 2 + 10)
    NEW_CTX_ARRAY(uint64_t, remote_stocks, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(int, remote_item_ids, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(int, remote_reply, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(int, local_stocks, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(int, local_item_ids, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(uint, local_supplies, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(uint, remote_supplies, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(float, i_price, my_ctx, ctx_offset, MAX_ITEM)
    NEW_CTX_ARRAY(order_line::value, remote_v_ol, my_ctx, ctx_offset, MAX_ITEM)

    *main_read_count = 0;

    allLocal = true;
    std::set<uint64_t> stock_set; // remove identity stock ids;
    stock_set.clear();
    // local buffer used store stocks

    num_remote_stocks = 0, num_local_stocks = 0;

    numItems = RandomNumber(random_generator, 5, MAX_ITEM); // +++++

    // txs[id].insert(make_pair(CUST,c_key));
    TEST_READF(main_read, CUST, c_key)
    // txs[id].insert(make_pair(WARE, warehouse_id));
    TEST_READF(main_read, WARE, warehouse_id)

    uint64_t d_key = makeDistrictKey(warehouse_id, districtID);
    TEST_READF(main_read, DIST, d_key)
    // next phase
    TEST_ARGS(args, warehouse_id);
    TEST_ARGS(args, districtID);
    TEST_ARGS(args, customerID);
    TEST_ARGS(args, numItems);
    // puts("2?");
    for (uint i = 0; i < numItems; i++)
    {
        bool conflict = false;
        int item_id = GetItemId(random_generator);
        // main partition
        uint64_t s_key;
        uint supplier_warehouse_id;
        if (KEY2NODE_MOD == 1 || RandomNumber(random_generator, 1, 100) > g_new_order_remote_item_pct)
        {
            // local stock case
            supplier_warehouse_id = warehouse_id;
            s_key = makeStockKey(supplier_warehouse_id, item_id);
            if (stock_set.find(s_key) != stock_set.end())
            {
                i--;
                continue;
            }
            else
            {
                stock_set.insert(s_key);
            }
            local_supplies[num_local_stocks] = supplier_warehouse_id;
            local_item_ids[num_local_stocks] = item_id;
            local_stocks[num_local_stocks++] = s_key;
        }
        else
        {
            // remote stock case
            do
            {
                supplier_warehouse_id = RandomNumber(random_generator, 1, NumWarehouses());
                s_key = makeStockKey(supplier_warehouse_id, item_id);
                if (stock_set.find(s_key) != stock_set.end())
                {
                    conflict = true;
                }
                else
                {
                    stock_set.insert(s_key);
                }
                // puts("3?");
            } while (supplier_warehouse_id == warehouse_id);
            allLocal = false;

            if (conflict)
            {
                i--;
                continue;
            }
            /* if possible, add remote stock to remote stocks */
            local_item_ids[num_local_stocks] = item_id;
            local_supplies[num_local_stocks] = supplier_warehouse_id;
            local_stocks[num_local_stocks++] = s_key;
        }
        TEST_ARGS(args, item_id);
        TEST_ARGS(args, supplier_warehouse_id);
        const uint ol_quantity = RandomNumber(random_generator, 1, 10);
        TEST_ARGS(args, ol_quantity);

        TEST_READF(main_read, ITEM, item_id)
        // TEST_READF(main_read, STOC, s_key)
    }
    reqPair req[4];

    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;

    memcpy(txfeature[id], feature, 64);
    // nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 0 /*new_order*/, tx_id, nic, (uint64_t)feature);
    return;
}
double check()
{
    std::set<pair<uint64_t, uint64_t>> result;
    uint8_t inter_f[128];
    result.clear();
    std::set<pair<uint64_t, uint64_t>>::iterator it;
    // std::set_intersection(std::begin(txs[0]), std::end(txs[1]), std::begin(txs[0]), std::end(txs[1]),std::inserter(result, std::begin(result)));
    it = txs[0].begin();
    while (it != txs[0].end())
    {
        if (txs[1].find(make_pair((*it).first, (*it).second)) != txs[1].end())
        {
            result.insert(make_pair((*it).first, (*it).second));
        }
        it++;
    }
    memset(inter_f, 0, sizeof(inter_f));
    // puts("4？");
    it = result.begin();
    while (it != result.end())
    {
        // printf("%d %d\n",(*it).first, (*it).second);
        uint64_t feature_offset = hash_k2f_2(*it);
        inter_f[feature_offset / 8] |= 1U << ((feature_offset % 8));
        it++;
    }
    int t_ans = 0;
    int t_real = 0;
    for (int i = 0; i < 1024; i++)
    {
        bool real = ((inter_f[i / 8] & (1U << (i % 8))) != 0);
        bool fake = ((txfeature[0][i / 8] & (1U << (i % 8)) & txfeature[1][i / 8]) != 0);
        // printf("%d %d %d ?? %d %d %d %d\n",inter_f[i/8], txfeature[0][i/8], txfeature[1][i/8], (txfeature[0][i / 8] & (1U << (i % 8)) & txfeature[1][i / 8]), (inter_f[i / 8] & (1U << (i % 8))), real, fake);
        if (fake)
        {
            t_ans++;
        }
        if (real && fake)
        {
            t_real++;
        }
    }
    shit_real += t_real;
    shit_ans += t_ans;
    // printf("%d %d\n",t_ans, t_real);
    if (t_ans == 0)
        return 1;
    else
        return t_real * 1.0 / t_ans;
}
int main()
{

    for (int x = 0; x < 8; x++)
    {
        // test1 = test1_array[x];
        test1 = 1024;
        shit_real = 0;
        shit_ans = 0;
        int cdf[1005];
        int cdf_count[1005];
        memset(cdf, 0, sizeof(cdf));
        memset(cdf_count, 0, sizeof(cdf_count));
        StoRandomDistribution<>::rng_type rng(233 /*seed*/);
        dd = new sampling::StoZipfDistribution<>(rng, 0, YCSB_TABLE_SIZE - 1, zipf[x]);

        random_generator.set_seed0(233);
        scale_factor = 2;
        init_feature_partition();
        my_ctx = (void *)malloc(CTXBUFFER);
        uint32_t x_rand = 233, y_rand = 17, z_rand = 19260817, w_rand = 31;

        double t_ans = 0;
        // puts("init over");
        // new_order_test(0);
        ycsb_test(0);
        const int top = 500000;
        for (int i = 1; i < top; i++)
        {
            // new_order_test(i%2);
            ycsb_test(i % 2);
            double ans = check();
            int shit = (int)(ans * 1000);
            if (shit < 0)
                shit = 0;
            if (shit > 1000)
                shit = 1000;
            t_ans += ans;
            cdf_count[shit]++;
            // printf("%d\n",shit);
        }
        for (int i = 0; i <= 1000; i++)
        {
            // printf("cdf_count = %d\n",cdf_count[i]);
            cdf[i] += cdf_count[i];
            // printf("%f\n",cdf[i]*1.0/top);
            cdf[i + 1] = cdf[i];
        }
        // printf("ans = %lf\n", t_ans / top);
        // printf("shit_real %d, shit_ans %d =  %lf\n", shit_real, shit_ans, (shit_real + 0.001) / shit_ans);
        printf("ans(zipf=%lf)=%lf\n", x*0.2, shit_real, shit_ans, (shit_real+0.001)/shit_ans);
    }


    return 0;
}