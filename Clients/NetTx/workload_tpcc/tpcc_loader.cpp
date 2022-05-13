/*
 * @Date: 2020-10-02 20:32:33
 * @LastEditTime: 2021-11-17 15:45:48
 * @FilePath: /TxSys/src/app/tpcc_loader.cpp
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "tpcc_loader.h"
// #include "mystorage.h"
using namespace std;
int g_uniform_item_dist = 0;
int scale_factor = CROSSNNNN;
int g_new_order_remote_item_pct = RANGEOFKEY_STORAGE;
uint64_t init_load_buf;
nocc::util::fast_random random_generator_;

void tpcc_load(int partition_id)
{
    // init_load_buf = (uint64_t)malloc(SBUFSIZE * 3); // max 2 G
    init_load_buf = (uint64_t)hugePageAlloc(MBSIZE * 2048);

    memset((char*)init_load_buf, 0, MBSIZE * 2048);
    uint64_t oldbuf = init_load_buf;
    random_generator_.set_seed0(partition_id);

    ware_load(partition_id);
    puts("1");
    // init_load_buf = (init_load_buf - oldbuf + (2 * MBSIZE - 1) ) / (2 * MBSIZE) * (2 * MBSIZE) + oldbuf;
    district_load(partition_id);
    puts("2");
    // init_load_buf = (init_load_buf - oldbuf + (2 * MBSIZE - 1) ) / (2 * MBSIZE) * (2 * MBSIZE) + oldbuf;
    customer_load(partition_id);
    puts("3");
    // init_load_buf = (init_load_buf - oldbuf + (2 * MBSIZE - 1) ) / (2 * MBSIZE) * (2 * MBSIZE) + oldbuf;
    order_load(partition_id);
    puts("4");
    init_load_buf = (init_load_buf - oldbuf + (2 * MBSIZE - 1)) / (2 * MBSIZE) * (2 * MBSIZE) + oldbuf;
    stock_load(partition_id);
    puts("5");
    // init_load_buf = (init_load_buf - oldbuf + (2 * MBSIZE - 1) ) / (2 * MBSIZE) * (2 * MBSIZE) + oldbuf;
    item_load(partition_id);
    puts("6");

    printf("size = %lfMB\n", (init_load_buf - oldbuf) / 1024.0 / 1024.0);
    assert(init_load_buf < oldbuf + MBSIZE * 2048);
    puts("ok");

    return;
}

void ware_load(int partition_id)
{
    uint64_t warehouse_total_sz = 0, n_warehouses = 0;
    for (uint i = GetStartWarehouse(partition_id); i <= GetEndWarehouse(partition_id); i++) {

        const warehouse::key k(i);
        const string w_name = RandomStr(random_generator_, RandomNumber(random_generator_, 6, 10));
        const string w_street_1 = RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20));
        const string w_street_2 = RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20));
        const string w_city = RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20));
        const string w_state = RandomStr(random_generator_, 3);
        const string w_zip = "123456789";

        // int w_size = store_->_schemas[WARE].total_len;
        int w_size = sizeof(warehouse::value) + 8;

        // char* wrapper = (char*)malloc(w_size + sizeof(MAP_entry_t));
        char* wrapper = (char*)init_load_buf;
        
        init_load_buf += (w_size + 63) / 64 * 64 + 64;

        memset(wrapper, 0, w_size);
        warehouse::value* v = (warehouse::value*)(wrapper);
        v->w_ytd = 300000 * 100;
        v->w_tax = (float)RandomNumber(random_generator_, 0, 2000) / 10000.0;
        v->w_name.assign(w_name);
        v->w_street_1.assign(w_street_1);
        v->w_street_2.assign(w_street_2);
        v->w_city.assign(w_city);
        v->w_state.assign(w_state);
        v->w_zip.assign(w_zip);
        // std::cout << v->w_city << std::endl;

        // checker::SanityCheckWarehouse(&k, v);
        // const size_t sz = Size(*v);
        // warehouse_total_sz += sz;
        // n_warehouses++;
        tx_key_t key(make_pair(WARE, i));
        init_insert(key, (uint64_t)wrapper, w_size);
        // store_->Put(WARE, i, (uint64_t*)wrapper);
    }
}

void district_load(int partition_id)
{
    string obj_buf;

    const ssize_t bsize = -1;
    uint64_t district_total_sz = 0, n_districts = 0;
    uint cnt = 0;
    for (uint w = GetStartWarehouse(partition_id); w <= GetEndWarehouse(partition_id); w++) {
        for (uint d = 1; d <= NumDistrictsPerWarehouse(); d++, cnt++) {
            // const district::key k(makeDistrictKey(w, d));

            int d_size = sizeof(district::value) + sizeof(uint64_t);
            // char* wrapper = (char*)malloc(d_size + sizeof(MAP_entry_t));
            char* wrapper = (char*)init_load_buf;
            init_load_buf += (d_size + 63) / 64 * 64 + 64;

            memset(wrapper, 0, d_size);
            district::value* v = (district::value*)(wrapper);
            v->d_ytd = 30000 * 100; //notice i did the scale up
            v->d_tax = (float)(RandomNumber(random_generator_, 0, 2000) / 10000.0);
            v->d_next_o_id = 3001;
            v->d_name.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 6, 10)));
            v->d_street_1.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
            v->d_street_2.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
            v->d_city.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
            v->d_state.assign(RandomStr(random_generator_, 3));
            v->d_zip.assign("123456789");

            const size_t sz = Size(*v);
            district_total_sz += sz;
            n_districts++;

            tx_key_t key(make_pair(DIST, makeDistrictKey(w, d)));
            init_insert(key, (uint64_t)wrapper, d_size);
            // store_->Put(DIST, key, (uint64_t*)wrapper);
        }
    }
}

void customer_load(int partition_id)
{
    string obj_buf;

    const uint w_start = GetStartWarehouse(partition_id);
    const uint w_end = GetEndWarehouse(partition_id);
    // uint w_end = 20;

    const size_t batchsize = NumCustomersPerDistrict();

    const size_t nbatches = (batchsize > NumCustomersPerDistrict()) ? 1 : (NumCustomersPerDistrict() / batchsize);

    uint64_t total_sz = 0;

    for (uint w = w_start; w <= w_end; w++) {
        //      if (pin_cpus)
        //        PinToWarehouseId(w);
        for (uint d = 1; d <= NumDistrictsPerWarehouse(); d++) {
            for (uint batch = 0; batch < nbatches;) {
                //          scoped_str_arena s_arena(arena);
                const size_t cstart = batch * batchsize;
                const size_t cend = std::min((batch + 1) * batchsize, NumCustomersPerDistrict());
                for (uint cidx0 = cstart; cidx0 < cend; cidx0++) {
                    const uint c = cidx0 + 1;
                    uint64_t key = makeCustomerKey(w, d, c);

                    const customer::key k(key);

                    int c_size = sizeof(customer::value) + sizeof(uint64_t);
                    // char* wrapper = (char*)malloc(c_size + sizeof(MAP_entry_t));
                    char* wrapper = (char*)init_load_buf;
                    init_load_buf += (c_size + 63) / 64 * 64 + 64;

                    memset(wrapper, 0, c_size);
                    customer::value* v = (customer::value*)(wrapper);
                    v->c_discount = (float)(RandomNumber(random_generator_, 1, 5000) / 10000.0);
                    if (RandomNumber(random_generator_, 1, 100) <= 10)
                        v->c_credit.assign("BC");
                    else
                        v->c_credit.assign("GC");

                    if (c <= 1000)
                        v->c_last.assign(GetCustomerLastName(random_generator_, c - 1));
                    else
                        v->c_last.assign(GetNonUniformCustomerLastNameLoad(random_generator_));

                    v->c_first.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 8, 16)));
                    v->c_credit_lim = 50000;

                    v->c_balance = -10;
                    v->c_balance_1 = 0;
                    v->c_ytd_payment = 10;
                    v->c_payment_cnt = 1;
                    v->c_delivery_cnt = 0;

                    v->c_street_1.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
                    v->c_street_2.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
                    v->c_city.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 20)));
                    v->c_state.assign(RandomStr(random_generator_, 3));
                    v->c_zip.assign(RandomNStr(random_generator_, 4) + "11111");
                    v->c_phone.assign(RandomNStr(random_generator_, 16));
                    v->c_since = GetCurrentTimeMillis();
                    v->c_middle.assign("OE");
                    v->c_data.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 300, 500)));

                    const size_t sz = Size(*v);
                    total_sz += sz;

                    tx_key_t tx_key(make_pair(CUST, key));
                    init_insert(tx_key, (uint64_t)wrapper, c_size);
                    // store_->Put(CUST, key, (uint64_t*)wrapper);
                    // customer name index

                    /* uint64_t sec = makeCustomerIndex(w, d, v->c_last.str(true), v->c_first.str(true));

                        //    uint64_t* mn = store_->_indexs[CUST_INDEX].Get(sec);
                        uint64_t* mn = store_->Get(CUST_INDEX, sec);
                        if (mn == NULL) {
                            char* ciwrap = new char[META_LENGTH + sizeof(uint64_t) * 2 + sizeof(uint64_t)];
                            memset(ciwrap, 0, META_LENGTH);
                            uint64_t* prikeys = (uint64_t*)(ciwrap + META_LENGTH);

                            prikeys[0] = 1;
                            prikeys[1] = key;
                            //printf("key %ld\n",key);
                            store_->Put(CUST_INDEX, sec, (uint64_t*)ciwrap);
                        } else {
                            assert(false);
                            printf("ccccc\n");
                            uint64_t* value = (uint64_t*)((char*)mn + META_LENGTH);
                            int num = value[0];
                            char* ciwrap = new char[META_LENGTH + sizeof(uint64_t) * (num + 2)];
                            memset(ciwrap, 0, META_LENGTH);
                            uint64_t* prikeys = (uint64_t*)(ciwrap + META_LENGTH);
                            prikeys[0] = num + 1;
                            for (int i = 1; i <= num; i++)
                                prikeys[i] = value[i];
                            prikeys[num + 1] = key;
                            store_->Put(CUST_INDEX, sec, (uint64_t*)ciwrap);
                        } */
                    // char* hwrap = new char[sizeof(history::value) + 8 + sizeof(MAP_entry_t)];
                    char* hwrap = (char*)init_load_buf;
                    init_load_buf += (sizeof(history::value) + 8 + 63) / 64 * 64 + 64;

                    uint64_t hkey = makeHistoryKey(c, d, w, d, w);

                    history::key k_hist(makeHistoryKey(c, d, w, d, w));

                    history::value* v_hist = (history::value*)(hwrap);
                    v_hist->h_amount = 10;
                    v_hist->h_data.assign(RandomStr(random_generator_, RandomNumber(random_generator_, 10, 24)));

                    tx_key_t tx_key_hist(make_pair(HIST, hkey));
                    init_insert(tx_key_hist, (uint64_t)hwrap, sizeof(history::value) + 8);
                    // store_->Put(HIST, hkey, (uint64_t*)hwrap);
                }
                batch++;
            }
        }
        /* end iterating warehosue */
    }
}

void order_load(int partition_id)
{
    string obj_buf;
    uint64_t order_line_total_sz = 0, n_order_lines = 0;
    uint64_t oorder_total_sz = 0, n_oorders = 0;
    uint64_t new_order_total_sz = 0, n_new_orders = 0;

    const uint w_start = GetStartWarehouse(partition_id);
    const uint w_end = GetEndWarehouse(partition_id);
    // uint w_end = 22;

    for (uint w = w_start; w <= w_end; w++) {
        for (uint d = 1; d <= NumDistrictsPerWarehouse(); d++) {
            set<uint> c_ids_s;
            vector<uint> c_ids;
            while (c_ids.size() != NumCustomersPerDistrict()) {
                const auto x = (random_generator_.next() % NumCustomersPerDistrict()) + 1;
                if (c_ids_s.count(x))
                    continue;
                c_ids_s.insert(x);
                c_ids.emplace_back(x);
            }
            for (uint c = 1; c <= NumCustomersPerDistrict();) {

                uint64_t okey = makeOrderKey(w, d, c);
                const oorder::key k_oo(okey);

                // char* wrapper = new char[sizeof(oorder::value) + sizeof(uint64_t) + sizeof(MAP_entry_t)];
                char* wrapper = (char*)init_load_buf;
                init_load_buf += (sizeof(oorder::value) + 63) / 64 * 64 + 64;

                memset(wrapper, 0, sizeof(oorder::value));
                oorder::value* v_oo = (oorder::value*)(wrapper);
                v_oo->o_c_id = c_ids[c - 1];
                if (c < 2101)
                    v_oo->o_carrier_id = RandomNumber(random_generator_, 1, 10);
                else
                    v_oo->o_carrier_id = 0;
                v_oo->o_ol_cnt = RandomNumber(random_generator_, 5, 15);
                //  v_oo->o_ol_cnt = 0;x

                v_oo->o_all_local = 1;
                v_oo->o_entry_d = GetCurrentTimeMillis();

                const size_t sz = Size(*v_oo);
                oorder_total_sz += sz;
                n_oorders++;
                tx_key_t tx_key_orde(make_pair(ORDE, okey));
                // if (w <= 20)
                init_insert(tx_key_orde, (uint64_t)wrapper, sizeof(oorder::value));
                // store_->Put(ORDE, okey, (uint64_t*)wrapper);

                /* uint64_t sec = makeOrderIndex(w, d, v_oo->o_c_id, c);

                char* oiwrapper = new char[META_LENGTH + 16 + sizeof(uint64_t)];
                memset(oiwrapper, 0, META_LENGTH + 16 + sizeof(uint64_t));
                uint64_t* prikeys = (uint64_t*)(oiwrapper + META_LENGTH);
                prikeys[0] = 1;
                prikeys[1] = okey;
                store_->Put(ORDER_INDEX, sec, (uint64_t*)oiwrapper); */

                // const oorder_c_id_idx::key k_oo_idx(makeOrderIndex(w, d, v_oo->o_c_id, c));
                // const oorder_c_id_idx::value v_oo_idx(0);
                if (c >= 2101) {
                    uint64_t nokey = makeNewOrderKey(w, d, c);
                    const new_order::key k_no(makeNewOrderKey(w, d, c));

                    char* nowrap = (char*)init_load_buf;
                    init_load_buf += (sizeof(new_order::value) + 63) / 64 * 64 + 64;

                    // char* nowrap = new char[sizeof(new_order::value) + sizeof(uint64_t) + sizeof(MAP_entry_t)];

                    memset(nowrap, 0, sizeof(new_order::value));
                    new_order::value* v_no = (new_order::value*)(nowrap);

                    // SanityCheckNewOrder(&k_no, v_no);
                    const size_t sz = Size(*v_no);
                    new_order_total_sz += sz;
                    n_new_orders++;
                    tx_key_t tx_key_norde(make_pair(NEWO, nokey));

                    init_insert(tx_key_norde, (uint64_t)nowrap, sizeof(new_order::value));
                }

                for (uint l = 1; l <= uint(v_oo->o_ol_cnt); l++) {

                    uint64_t olkey = makeOrderLineKey(w, d, c, l);
                    const order_line::key k_ol(makeOrderLineKey(w, d, c, l));

                    // char* olwrapper = new char[sizeof(order_line::value) + sizeof(uint64_t) + sizeof(MAP_entry_t)];
                    char* olwrapper = (char*)init_load_buf;
                    init_load_buf += (sizeof(order_line::value) + 63) / 64 * 64 + 64;

                    memset(olwrapper, 0, sizeof(order_line::value));
                    order_line::value* v_ol = (order_line::value*)(olwrapper);
                    v_ol->ol_i_id = RandomNumber(random_generator_, 1, 100000);
                    if (c < 2101) {
                        v_ol->ol_delivery_d = v_oo->o_entry_d;
                        v_ol->ol_amount = 0;
                    } else {
                        v_ol->ol_delivery_d = 0;
                        /* random within [0.01 .. 9,999.99] */
                        v_ol->ol_amount = (float)(RandomNumber(random_generator_, 1, 999999) / 100.0);
                    }

                    v_ol->ol_supply_w_id = w;
                    v_ol->ol_quantity = 5;
                    // v_ol.ol_dist_info comes from stock_data(ol_supply_w_id, ol_o_id)
                    SanityCheckOrderLine(&k_ol, v_ol);
                    const size_t sz = Size(*v_ol);
                    order_line_total_sz += sz;
                    n_order_lines++;
                    tx_key_t tx_key_ol(make_pair(ORLI, olkey));
                    init_insert(tx_key_ol, (uint64_t)olwrapper, sizeof(order_line::value));
                    // store_->Put(ORLI, olkey, (uint64_t*)olwrapper);
                }
                c++;
            }
        }
    }
}

void stock_load(int partition_id)
{
    string obj_buf, obj_buf1;
    uint64_t stock_total_sz = 0, n_stocks = 0;
    const uint w_start = GetStartWarehouse(partition_id);
    const uint w_end = GetEndWarehouse(partition_id);
    // uint w_end = 20;

    for (uint w = w_start; w <= w_end; w++) {
        const size_t batchsize = NumItems();
        const size_t nbatches = (batchsize > NumItems()) ? 1 : (NumItems() / batchsize);

        for (uint b = 0; b < nbatches;) {

            const size_t iend = std::min((b + 1) * batchsize + 1, NumItems());
            for (uint i = (b * batchsize + 1); i <= iend; i++) {
                uint64_t key = makeStockKey(w, i);

                // int s_size = store_->_schemas[STOC].total_len;
                int s_size = sizeof(stock::value);
                s_size = (s_size + 7) / 8 * 8 + 8;

                char* wrapper = NULL;

                // If main store, allocation memory on RDMA heap
                // wrapper = (char*)malloc(s_size + sizeof(MAP_entry_t));
                wrapper = (char*)init_load_buf;
                init_load_buf += (s_size+ 63) / 64 * 64 + 64;

                memset(wrapper, 0, s_size );
                stock::value* v = (stock::value*)(wrapper);

                v->s_quantity = RandomNumber(random_generator_, 10, 100);
                v->s_ytd = 0;
                v->s_order_cnt = 0;
                v->s_remote_cnt = 0;

                const int len = RandomNumber(random_generator_, 26, 50);
                if (RandomNumber(random_generator_, 1, 100) > 10) {
                    const string s_data = RandomStr(random_generator_, len);
                } else {
                    const int startOriginal = RandomNumber(random_generator_, 2, (len - 8));
                    const string s_data = RandomStr(random_generator_, startOriginal + 1)
                        + "ORIGINAL" + RandomStr(random_generator_, len - startOriginal - 7);
                }
                const size_t sz = Size(*v);
                stock_total_sz += sz;
                n_stocks++;

                tx_key_t tx_key_stock(make_pair(STOC, key));
                init_insert(tx_key_stock, (uint64_t)wrapper, s_size);
                //   auto node = store_->Put(STOC, key, (uint64_t *)wrapper,sizeof(stock::value));
            }
            b++;
        }
    }
}

void item_load(int partition_id)
{
    string obj_buf;
    uint64_t total_sz = 0;
    for (uint i = 1; i <= NumItems(); i++) {
        const item::key k(i);

        int i_size = sizeof(item::value) + 8;
        // char* wrapper = (char*)malloc(i_size + sizeof(MAP_entry_t));
        char* wrapper = (char*)init_load_buf;
        init_load_buf += (i_size + 63) / 64 * 64 + 64;
        memset(wrapper, 0, i_size);

        item::value* v = (item::value*)(wrapper);
        ;
        const string i_name = RandomStr(random_generator_, RandomNumber(random_generator_, 14, 24));
        v->i_name.assign(i_name);
        v->i_price = (float)RandomNumber(random_generator_, 100, 10000) / 100.0;
        const int len = RandomNumber(random_generator_, 26, 50);
        if (RandomNumber(random_generator_, 1, 100) > 10) {
            const string i_data = RandomStr(random_generator_, len);
            v->i_data.assign(i_data);
        } else {
            const int startOriginal = RandomNumber(random_generator_, 2, (len - 8));
            const string i_data = RandomStr(random_generator_, startOriginal + 1) + "ORIGINAL" + RandomStr(random_generator_, len - startOriginal - 7);
            v->i_data.assign(i_data);
        }
        v->i_im_id = RandomNumber(random_generator_, 1, 10000);
        SanityCheckItem(&k, v);
        const size_t sz = Size(*v);
        total_sz += sz;
        tx_key_t tx_key_item(make_pair(ITEM, i));
        init_insert(tx_key_item, (uint64_t)wrapper, i_size);
        // store_->Put(ITEM, i, (uint64_t*)wrapper);
    }
}