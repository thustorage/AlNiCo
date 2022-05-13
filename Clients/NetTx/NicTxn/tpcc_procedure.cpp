
#include "nictxn_rpc.h"

void nictxn_tx::tpcc_procedure()
{
#ifdef NULLRPC
    tpcc_latency_type = (int)tpcc_txn_type_t::new_order; // only for latency test
    return null_rpc();
#endif

#ifdef YCSBHOT
    tpcc_latency_type = (int)tpcc_txn_type_t::new_order; // only for latency test 
    return HOT();
#endif

#ifdef YCSBplusT
    tpcc_latency_type = (int)tpcc_txn_type_t::new_order; // only for latency test 
    return YCSB_partitional();
#endif

    int x = (int)nictxn_rpc_ptr->workgen_arr_tpcc[xorshift128()%100];
    tpcc_latency_type = x;
    switch (x) {
    case (int)tpcc_txn_type_t::new_order:
        new_order_procedure();
        break;
    case (int)tpcc_txn_type_t::pay:
        payment_procedure();
        break;
    case (int)tpcc_txn_type_t::del:
        delivery();
        break;
    case (int)tpcc_txn_type_t::order:
        order_status();
        break;
    case (int)tpcc_txn_type_t::stock:
        stock_level();
        break;
    default:
        tpcc_latency_type = (int)tpcc_txn_type_t::new_order;
        new_order_procedure();
        break;
    }
    
    return;
}

void nictxn_tx::YCSB_procedure()
{
    ycsb_count++; 
    int outx = 0;
    int x = RandomNumber(random_generator, 1, 20) - 1;
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    args_count = 0;
    write_bits = 0;
    ctx_offset = 0;
    NEW_CTX_ARRAY(uint32_t, ycsb_key_arrays, my_ctx, ctx_offset, YCSBSIZE);
    NEW_CTX_ARRAY(uint16_t, ycsb_col_arrays, my_ctx, ctx_offset, YCSBSIZE);
    ycsb_set.clear();
    bool is_write;
    uint16_t ycsb_write_num = 0;
    for (int i = 0; i < YCSBSIZE; i++) {
        uint32_t key = nictxn_rpc_ptr->dd->sample();

        while (ycsb_set.find(key) != ycsb_set.end()) {
            if (i > YCSBSIZE - outx) {
                key = RandomNumber(random_generator, 1, YCSB_TABLE_SIZE) - 1;
            } else {
                key = nictxn_rpc_ptr->dd->sample();
            }
        }
        ycsb_set.insert(key);
        is_write = ((RandomNumber(random_generator, 1, 100) < YCSBWRITE) && i <= YCSBSIZE - outx);
        ADD_TO_YCSB_WRITE_BIT(i, write_bits);

        if (is_write)
            ycsb_write_num++;
        ADD_TO_YCSB_WRITEF(i, nic, key);
    }

    reqPair req[5];
    req[0].addr = (uint64_t)&write_bits;
    req[0].size = sizeof(write_bits);
    req[1].addr = (uint64_t)ycsb_key_arrays;
    req[1].size = sizeof(uint32_t) * (YCSBSIZE);
    req[2].addr = (uint64_t)ycsb_col_arrays;
    req[2].size = sizeof(uint16_t) * (YCSBSIZE);
    // req[3].addr = (uint64_t)feature;
    // req[3].size = 64;
    req[3].addr = (uint64_t)ycsb_ctx;
    req[3].size = ycsb_write_num * COL_WIDTH;
    
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 0 /*YCSB 0*/, tx_id, nic, (uint64_t)feature);
    return;
}

void nictxn_tx::HOT()
{

    ycsb_count++; // DYNAMIC !!!!
    int outx = 0;
    int x = RandomNumber(random_generator, 1, 20) - 1;
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    // nic = x;
    args_count = 0;

    write_bits = 0;

    ctx_offset = 0;
    NEW_CTX_ARRAY(uint32_t, ycsb_key_arrays, my_ctx, ctx_offset, YCSBSIZE);
    NEW_CTX_ARRAY(uint16_t, ycsb_col_arrays, my_ctx, ctx_offset, YCSBSIZE);
    

    uint64_t hot_p = RandomNumber(random_generator, 1, 20) - 1;
    int hot_pair = nictxn_rpc_ptr->hot_record[now_phase % 8][hot_p];
    uint32_t hot_col = hot_pair / 5;
    uint32_t hot_key = hot_pair % 5;

    uint32_t whole_key = hot_col * (YCSB_TABLE_SIZE / YCSBCOL) + hot_key;

    ycsb_set.clear();
    ycsb_set.insert(hot_key);
    ADD_TO_YCSB_WRITE_BIT(0, write_bits);
    uint16_t ycsb_write_num = 1;
    ADD_TO_YCSB_WRITEF(0, hot_col, hot_key);
    uint32_t part_size = (YCSB_TABLE_SIZE / YCSBCOL / 5 - 1);
    uint32_t base = hot_key * part_size + 5;
    bool is_write;
    for (int i = 1; i < YCSBSIZE; i++) {
        uint32_t key = nictxn_rpc_ptr->dd->sample() % part_size;
        uint32_t col = xorshift128() % 20;
        while (ycsb_set.find(key) != ycsb_set.end()) {
            key = nictxn_rpc_ptr->dd->sample();
        }
        ycsb_set.insert(key);
        is_write = ((RandomNumber(random_generator, 1, 100) < YCSBWRITE));
        ADD_TO_YCSB_READ(i, hot_col, key + base);
        if (is_write){
            ADD_TO_YCSB_WRITE_BIT(i, write_bits);
            ycsb_write_num++;
        }
    }
    
    #ifdef SPNIC2
    nic = hot_col; // strife: hot table 
    #endif
    #ifdef SPNICXX
    nic = hot_p;  // schism: partition
    #endif

    reqPair req[5];
    req[0].addr = (uint64_t)&write_bits;
    req[0].size = sizeof(write_bits);
    req[1].addr = (uint64_t)ycsb_key_arrays;
    req[1].size = sizeof(uint32_t) * (YCSBSIZE);
    req[2].addr = (uint64_t)ycsb_col_arrays;
    req[2].size = sizeof(uint16_t) * (YCSBSIZE);
    // req[3].addr = (uint64_t)feature;
    // req[3].size = 64;
    req[3].addr = (uint64_t)ycsb_ctx;
    req[3].size = ycsb_write_num * COL_WIDTH;
    
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 0 /*YCSB 0*/, tx_id, nic, (uint64_t)feature);
    return;
}


void nictxn_tx::null_rpc()
{
    // ycsb_count++; // DYNAMIC !!!!
    int outx = 0;
    memset(feature, 0, sizeof(feature));
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    // nic = global_id % MAX_THREAD;
    args_count = 0;
    write_bits = 0;
    ctx_offset = 0;
    NEW_CTX_ARRAY(uint32_t, ycsb_key_arrays, my_ctx, ctx_offset, YCSBSIZE);
    NEW_CTX_ARRAY(uint16_t, ycsb_col_arrays, my_ctx, ctx_offset, YCSBSIZE);
    
    reqPair req[5];
    req[0].addr = (uint64_t)ycsb_ctx;
    req[0].size = RPCSIZE;
    
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 1, 0 /*YCSB 0*/, tx_id, nic, (uint64_t)feature);
    return;
}

void nictxn_tx::YCSB_partitional()
{
    // ycsb_count++; // DYNAMIC !!!!
    int outx = 0;
    int x = RandomNumber(random_generator, 1, 20) - 1;
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    // nic = x;
    args_count = 0;

    write_bits = 0;

    ctx_offset = 0;
    NEW_CTX_ARRAY(uint32_t, ycsb_key_arrays, my_ctx, ctx_offset, YCSBSIZE);
    NEW_CTX_ARRAY(uint16_t, ycsb_col_arrays, my_ctx, ctx_offset, YCSBSIZE);
    

    ycsb_set.clear();
    bool is_write;
    uint16_t ycsb_write_num = 0;
    for (int i = 0; i < YCSBSIZE; i++) {
        uint32_t key = nictxn_rpc_ptr->dd->sample();

        if (ycsb_count > 49069) { // 1500 * 5 * 10000 / 48 / 4 dynamic
            key += 20012;
            while (ycsb_set.find(key) != ycsb_set.end() || key % 25 == 0) {
                if (i > YCSBSIZE - outx) {
                    key = RandomNumber(random_generator, 1, YCSB_TABLE_SIZE) - 1;
                } else {
                    key = nictxn_rpc_ptr->dd->sample();
                }
                key += 20012;
            }
        } else {
            while (ycsb_set.find(key) != ycsb_set.end()) {
                if (i > YCSBSIZE - outx) {
                    key = (RandomNumber(random_generator, 1, YCSB_TABLE_SIZE) - 1 );
                } else {
                    key = nictxn_rpc_ptr->dd->sample();
                }
            }
        }
        // assert(write_bits)
        // key = key % (YCSB_TABLE_SIZE/YCSBCOL); 
        ycsb_set.insert(key);
        is_write = ((RandomNumber(random_generator, 1, 100) < YCSBWRITE) && i <= YCSBSIZE - outx);
        // ADD_TO_ARGS(args, is_write);
        if (is_write)
            ADD_TO_YCSB_WRITE_BIT(i, write_bits);

        if (is_write)
            ycsb_write_num++;

        
        if (i > YCSBSIZE - outx) {
            if (RandomNumber(random_generator, 1, 100) > (NEWORDERP)) {
                ADD_TO_YCSB_READ(i, nic, key);
            } else {
                ADD_TO_YCSB_WRITEF(i, nic, key);
            }
        } else {
            if (RandomNumber(random_generator, 1, 100) > (NEWORDERP)) {
                ADD_TO_YCSB_READ(i, x, key);
            } else {
                ADD_TO_YCSB_WRITEF(i, x, key);
            }
        }
    }
    // nic = x;
    #ifdef SPNIC2
    nic = x; 
    #endif
    // #ifdef SPNICXX
    //     if (ycsb_set.find(0) != ycsb_set.end() || ycsb_set.find(1) != ycsb_set.end() ){
    //         nic = x; 
    //     }
    //     else{
    //         nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    //     }
    // #endif

    reqPair req[5];
    req[0].addr = (uint64_t)&write_bits;
    req[0].size = sizeof(write_bits);
    req[1].addr = (uint64_t)ycsb_key_arrays;
    req[1].size = sizeof(uint32_t) * (YCSBSIZE);
    req[2].addr = (uint64_t)ycsb_col_arrays;
    req[2].size = sizeof(uint16_t) * (YCSBSIZE);
    // req[3].addr = (uint64_t)feature;
    // req[3].size = 64;
    req[3].addr = (uint64_t)ycsb_ctx;
    req[3].size = ycsb_write_num * COL_WIDTH;
    
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 0 /*YCSB 0*/, tx_id, nic, (uint64_t)feature);
    return;
}

void nictxn_tx::payment_procedure()
{
    tx_main_partition = 0;
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    args_count = 0;

    NEW_CTX_ARRAY(int, main_read_count, my_ctx, ctx_offset, 1) // payment do not need
    *main_read_count = 0;
    NEW_CTX_ARRAY(tx_key_t, main_read, my_ctx, ctx_offset, 6);

    warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    warehouse_id_end_ = GetEndWarehouse(tx_main_partition);

    // uint64_t q_w_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_);
    // uint64_t q_d_id = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse());
    uint64_t q_w_id = (xorshift128()) % scale_factor + 1;
    #ifdef ZIPFTPCC
        q_w_id = nictxn_rpc_ptr->dd->sample();
    #endif

    uint64_t q_d_id = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse_());
    warehouse_id = q_w_id;
    districtID = q_d_id;
    

    #ifdef SPNIC2
    nic = ((warehouse_id % 3 == 1) * 10   + (districtID - 1)) % (MAX_THREAD); 
    // nic = xorshift128() % MAX_THREAD;
    #endif
    #ifdef SPNICXX
    nic = (q_w_id - 1) % (MAX_THREAD);
    #endif

    
    // q_d_id = RandomNumber(random_generator, 1, MAX_THREAD / 2); // quick check

    uint64_t q_c_w_id, q_c_d_id, q_c_id;
    uint64_t last_name_id;
    int x = RandomNumber(random_generator, 1, 100);
    int y = RandomNumber(random_generator, 1, 100);

    bool is_home = (scale_factor == 1) || (x <= 85);
    bool by_name = (y <= 60);
    by_name = false;

    if (is_home) {
        q_c_w_id = q_w_id;
        q_c_d_id = q_d_id;
    } else {
        do {
            q_c_w_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_);
        } while (q_c_w_id == q_w_id);
        q_c_d_id = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse_());
    }

    uint64_t d_key0 = makeDistrictKey(q_w_id, q_d_id);
    uint64_t d_key1 = makeDistrictKey(q_c_w_id, q_c_d_id);
    // ADD_TO_WRITEF(main_read, WARE, q_w_id);
    ADD_TO_WRITEF(main_read, DIST, d_key0);
    // ADD_TO_WRITEF(main_read, WARE, q_c_w_id);
    // ADD_TO_WRITEF(main_read, DIST, d_key1);

    if (by_name) {
        q_c_id = NonUniformRandom(random_generator,
            255, 223 /* XXX 4.3.2.3 magic C number */,
            0, 999);
        // ADD_TO_WRITEF(main_read, ARGS_DATA, q_c_id);
    } else {
        q_c_id = GetCustomerId(random_generator);
        uint64_t c_key0 = makeCustomerKey(q_w_id, q_d_id, q_c_id);
        uint64_t c_key1 = makeCustomerKey(q_c_w_id, q_c_d_id, q_c_id);
        // ADD_TO_WRITEF(main_read, CUST, c_key0);
        // ADD_TO_WRITEF(main_read, CUST, c_key1);
    }

    ADD_TO_ARGS(args, q_w_id);
    ADD_TO_ARGS(args, q_d_id);
    ADD_TO_ARGS(args, q_c_w_id);
    ADD_TO_ARGS(args, q_c_d_id);
    ADD_TO_ARGS(args, is_home);
    ADD_TO_ARGS(args, by_name);
    ADD_TO_ARGS(args, q_c_id);

    reqPair req[4];
    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;

    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 1 /*payment*/, tx_id, nic, (uint64_t)feature);
}

void nictxn_tx::new_order_procedure()
{
    // uint8_t feature[64];
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    tx_main_partition = 0; // the only one partition
    args_count = 0;


    // warehouse_id_start_ = GetStartWarehouse_thread(tx_main_partition,thread_id);
    // warehouse_id_end_ = GetEndWarehouse_thread(tx_main_partition,thread_id);
    warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    warehouse_id_end_ = GetEndWarehouse(tx_main_partition);
    // warehouse_id_start_ = 1;
    // warehouse_id_end_ = 20;

    warehouse_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_); // +++++
    warehouse_id = xorshift128() % scale_factor + 1;
    #ifdef ZIPFTPCC
        warehouse_id = nictxn_rpc_ptr->dd->sample();
    #endif


    districtID = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse_()); // +++++

    customerID = GetCustomerId(random_generator); // +++++
    // assert(customerID!=0);
    c_key = makeCustomerKey(warehouse_id, districtID, customerID); // +++++

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
    // numItems+++++ * s_key
    // for main partition

    ADD_TO_WRITEF(main_read, CUST, c_key)
    ADD_TO_WRITEF(main_read, WARE, warehouse_id)

    uint64_t d_key = makeDistrictKey(warehouse_id, districtID);
    // Debug::notifyError("(%d,%d) = %d ",warehouse_id,districtID,d_key);
    ADD_TO_WRITEF(main_read, DIST, d_key)
    // next phase
    ADD_TO_ARGS(args, warehouse_id);
    ADD_TO_ARGS(args, districtID);
    ADD_TO_ARGS(args, customerID);
    ADD_TO_ARGS(args, numItems);

    for (uint i = 0; i < numItems; i++) {
        bool conflict = false;
        int item_id = GetItemId(random_generator);
        // main partition
        uint64_t s_key;
        uint supplier_warehouse_id;
        if (KEY2NODE_MOD == 1 || RandomNumber(random_generator, 1, 100) > g_new_order_remote_item_pct) {
            // local stock case
            supplier_warehouse_id = warehouse_id;
            s_key = makeStockKey(supplier_warehouse_id, item_id);
            if (stock_set.find(s_key) != stock_set.end()) {
                i--;
                continue;
            } else {
                stock_set.insert(s_key);
            }
            local_supplies[num_local_stocks] = supplier_warehouse_id;
            local_item_ids[num_local_stocks] = item_id;
            local_stocks[num_local_stocks++] = s_key;
        } else {
            // remote stock case

            do {
                supplier_warehouse_id = RandomNumber(random_generator, 1, NumWarehouses());
                s_key = makeStockKey(supplier_warehouse_id, item_id);
                if (stock_set.find(s_key) != stock_set.end()) {
                    conflict = true;
                } else {
                    stock_set.insert(s_key);
                }
            } while (supplier_warehouse_id == warehouse_id);
            allLocal = false;

            if (conflict) {
                i--;
                continue;
            }
            /* if possible, add remote stock to remote stocks */
            local_item_ids[num_local_stocks] = item_id;
            local_supplies[num_local_stocks] = supplier_warehouse_id;
            local_stocks[num_local_stocks++] = s_key;
        }
        ADD_TO_ARGS(args, item_id);
        ADD_TO_ARGS(args, supplier_warehouse_id);
        const uint ol_quantity = RandomNumber(random_generator, 1, 10);
        ADD_TO_ARGS(args, ol_quantity);

        ADD_TO_WRITEF(main_read, ITEM, item_id)
        ADD_TO_WRITEF(main_read, STOC, s_key)
    }
    //   // Execution phase ////////////////////////////////////////////////////

    // for (uint ol_number = 1; ol_number <= num_local_stocks; ol_number++) {
    //     const uint ol_i_id = local_item_ids[ol_number - 1];
    //     const uint ol_quantity = RandomNumber(random_generator, 1, 10);

    //     ADD_TO_WRITEF(main_read, ITEM, ol_i_id)
    //     //auto idx = rtx_->read<ITEM, item::value>(current_partition, ol_i_id, yield);

    //     uint64_t s_key = local_stocks[ol_number - 1];
    //     ADD_TO_WRITEF(main_read, STOC, s_key)
    //     // #if (DEBUG_ON)
    //     // Debug::notifyError("7,%llu 8,%llu,local_supplies(%llu),local_item_ids(%llu)=%llu",
    //     // ol_i_id,s_key,
    //     // local_supplies[ol_number-1],local_item_ids[ol_number-1],
    //     // makeStockKey(local_supplies[ol_number-1], local_item_ids[ol_number-1]));
    //     // #endif
    //     //idx = rtx_->read<STOC, stock::value>(current_partition, s_key, yield);
    // }

    // /* operation remote objects */
    // tx_key_t x;
    reqPair req[4];
    // req[0].addr = (uint64_t)&x;
    // req[0].size = sizeof(tx_key_t);
    // req_num_wait = num_remote_stocks + 1;
    // // TODO
    // for (uint i = 0; i < num_remote_stocks; ++i) {
    //     const uint ol_i_id = remote_item_ids[i];
    //     remote_reply[i] = 0;
    //     ADD_TO_WRITEF(main_read, ITEM, ol_i_id)
    //     uint64_t s_key = remote_stocks[i];
    //     x.table_id_pair = STOC;
    //     x.table_key_pair = s_key;
    //     nictxn_rpc_ptr->asyncCall(WarehouseToPartition(remote_supplies[i]), NORMALREAD, req[0], tx_id, (int)tpcc_txn_type_t::new_order_phase_other, &remote_stocks[i]);
    // }

    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;

    // Debug::notifyError("send main_read_count=%d + args=%d\n", *main_read_count, args_count);
    // nictxn_rpc_ptr->asyncCall(tx_main_partition, TPCCREAD, req, 2, tx_id, (int)tpcc_txn_type_t::new_order_phase_main, NULL);
    // nic = xorshift128() % (MAX_THREAD);
    nic = ((warehouse_id - 1) * 10 + (districtID - 1) ) % (MAX_THREAD); // w & d

    nic = xorshift128() % (MAX_THREAD);
    

    #ifdef SPNIC2
    nic = ((warehouse_id % 3 == 1) * 10 + (districtID - 1)) % (MAX_THREAD); // strife: d_id
    #endif
    #ifdef SPNICXX
    nic = (warehouse_id - 1) % (MAX_THREAD); // schism: w_id
    #endif



    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 0 /*new_order*/, tx_id, nic, (uint64_t)feature);

    // int num = NICTXTESTSIZE;
    // tx_key_t a;
    // uint64_t* tmp = (uint64_t*)my_ctx;
    //

    //     for (int i = 0; i < num; i++) {
    //         get_account_zipf_offset(&state, &tmp[i], nic);

    //         // get_various_account(&seed, &tmp[i], nic);
    // #if (DEBUG_ON)
    //         printf("%llu ", tmp[i]);
    // #endif
    //     }
    //     // if (thread_id == 0 && nic == 0) {
    //     //     printf("[%d]  ",nic);
    //     //     for (int i = 0; i < num; i++) {

    //     //         printf("%llu ", tmp[i]);
    //     //     }
    //     //     puts("");
    //     // }

    //     req_pair[0].addr = (uint64_t)my_ctx;
    //     req_pair[0].size = num * sizeof(uint64_t);
    //     // TODO:
    //     single_node_id = 0;

    //     return;
    return;
}

void nictxn_tx::order_status()
{
    tx_main_partition = 0; // only one partition in NicTxn

    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    args_count = 0;

    NEW_CTX_ARRAY(int, main_read_count, my_ctx, ctx_offset, 1) // payment do not need
    *main_read_count = 0;
    NEW_CTX_ARRAY(tx_key_t, main_read, my_ctx, ctx_offset, 6);

    // uint64_t q_c_w_id, q_c_d_id, q_c_id;
    // uint64_t last_name_id;
    warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    warehouse_id_end_ = GetEndWarehouse(tx_main_partition);
    uint64_t q_w_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_);
    #ifdef ZIPFTPCC
        q_w_id = nictxn_rpc_ptr->dd->sample();
    #endif
    
    uint64_t q_d_id = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse_());
    int x = RandomNumber(random_generator, 1, 100);
    bool by_name = (x <= 60);

    std::string last_name;
    uint64_t q_c_id;

    if (!by_name) {
        q_c_id = GetCustomerId(random_generator);
        // ADD_TO_WRITEF(main_read, ARGS_DATA, q_c_id);
    } else {
        q_c_id = 0;
    }
    uint64_t d_key0 = makeDistrictKey(q_w_id, q_d_id);
    ADD_TO_WRITEF(main_read, DIST, d_key0);

    // holding outputs of the transaction

    ADD_TO_ARGS(args, q_w_id);
    ADD_TO_ARGS(args, q_d_id);
    ADD_TO_ARGS(args, by_name);
    ADD_TO_ARGS(args, q_c_id);

    reqPair req[4];
    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;

    #ifdef SPNIC2
    nic = ((q_w_id % 3 == 1) * 10 + (q_d_id - 1)) % (MAX_THREAD);
    #endif
    #ifdef SPNICXX
    nic = (q_w_id - 1) % (MAX_THREAD);
    #endif

    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 2 /*order status*/, tx_id, nic, (uint64_t)feature);
}

void nictxn_tx::stock_level()
{

    tx_main_partition = 0; // only one partition in NicTxn
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    args_count = 0;

    NEW_CTX_ARRAY(int, main_read_count, my_ctx, ctx_offset, 1) // payment do not need
    *main_read_count = 0;
    NEW_CTX_ARRAY(tx_key_t, main_read, my_ctx, ctx_offset, 6);

    // uint64_t q_c_w_id, q_c_d_id, q_c_id;
    // uint64_t last_name_id;
    warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    warehouse_id_end_ = GetEndWarehouse(tx_main_partition);
    uint64_t q_w_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_);
    #ifdef ZIPFTPCC
        q_w_id = nictxn_rpc_ptr->dd->sample();
    #endif
    uint64_t q_d_id = RandomNumber(random_generator, 1, NumDistrictsPerWarehouse_());

    #ifdef SPNIC2
    nic = ((q_w_id % 3 == 1) * 10  + (q_d_id - 1)) % (MAX_THREAD); // ??????
    // nic = xorshift128() % MAX_THREAD;
    #endif
    #ifdef SPNICXX
    nic = (q_w_id - 1) % (MAX_THREAD);
    #endif

    uint64_t d_key0 = makeDistrictKey(q_w_id, q_d_id);
    ADD_TO_WRITEF(main_read, DIST, d_key0);

    ADD_TO_ARGS(args, q_w_id);
    ADD_TO_ARGS(args, q_d_id);

    reqPair req[4];
    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 3 /*stock level*/, tx_id, nic, (uint64_t)feature);
}

void nictxn_tx::delivery()
{
    tx_main_partition = 0; // only one partition in NicTxn
    memset(feature, 0, sizeof(feature));
    uint16_t feature_offset;
    nic = (RandomNumber(random_generator, 1, MAX_THREAD) - 1) % (MAX_THREAD);
    args_count = 0;

    NEW_CTX_ARRAY(int, main_read_count, my_ctx, ctx_offset, 1) // payment do not need
    *main_read_count = 0;
    NEW_CTX_ARRAY(tx_key_t, main_read, my_ctx, ctx_offset, 1);

    // uint64_t q_c_w_id, q_c_d_id, q_c_id;
    // uint64_t last_name_id;
    warehouse_id_start_ = GetStartWarehouse(tx_main_partition);
    warehouse_id_end_ = GetEndWarehouse(tx_main_partition);
    uint64_t q_w_id = PickWarehouseId(random_generator, warehouse_id_start_, warehouse_id_end_); 
    #ifdef ZIPFTPCC
        q_w_id = nictxn_rpc_ptr->dd->sample();
    #endif
    // q_w_id = PickWarehouseId(random_generator, 1, 2); 

    ADD_TO_WRITEF(main_read, WARE, warehouse_id)
    ADD_TO_ARGS(args, q_w_id);

    reqPair req[4];
    req[0].addr = (uint64_t)main_read_count;
    req[0].size = sizeof(int);
    req[1].addr = (uint64_t)main_read;
    req[1].size = sizeof(tx_key_t) * (*main_read_count);
    req[2].addr = (uint64_t)args;
    req[2].size = sizeof(uint64_t) * (args_count);
    req[3].addr = (uint64_t)feature;
    req[3].size = 64;
    nictxn_rpc_ptr->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req, 4, 4 /*delivery*/, tx_id, nic, (uint64_t)feature);
}
