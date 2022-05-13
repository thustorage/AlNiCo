
#include "nictxn_rpc.h"


void nictxn_rpc::run_worker()
{
    // read the addr
#ifdef NICNICNIC
    uint32_t now_message = 0;
    uint32_t ip_mess_id[PARALLEL_IP_NUM];
    memset(ip_mess_id, 0, sizeof(ip_mess_id));
    // while (fpga_init_over == 0){
    //     usleep(100);
    //     __asm __volatile("" ::: "memory");
    // }
    // printf("thread %d begin\n",thread_id);
    NicTx_tx_local_buf = (uint8_t*)malloc(8192);
    NicTx_requeset_buf = (uint8_t*)malloc(8192);
    while (true) {
        // continue;
        // partition from thread_id client_thread * [KEY2NODE_MOD,MAX_SERVER)
        /*
        for (int i = thread_id + (KEY2NODE_MOD * CLIENT_THREAD); i < CLIENT_THREAD * (MAX_SERVER); i += MAX_THREAD) {
            for (int j = 0; j < ASYNCTXNUM; j++) {
                uint64_t addr = nic_connect[i / CLIENT_THREAD][i % CLIENT_THREAD]->get_rev_addr(j);
                ReqHead* req_head = (ReqHead*)addr;
                int size = (req_head->read_num + req_head->write_num) * sizeof(uint64_t) + sizeof(ReqHead) + 1 * sizeof(SIZETYPE);
                uint16_t* flag = (uint8_t*)(addr + size);
                // printf("ahhh rpc_id = %d addr = %llx size=%d %d\n", req_head->rpcId.rpc_id, addr, size,(*flag));
                // usleep(1);

                if ((*flag) == 0)
                    continue;

                // execute
                *flag = 0;

                // write the addr
                addr = nic_connect[i / CLIENT_THREAD][i % CLIENT_THREAD]->get_send_addr(j);
                req_head->rpcId.rpc_id += NOTHING_DEBUG;
                printf("I write addr = %llx %d\n", addr, req_head->rpcId.rpc_id);
                *(rpc_id_t*)addr = req_head->rpcId;
            }
        }
        */

        /*logic for NIC
         *
         * poll network from nic [client_id][thread_id][async_id]
         * read from the recv message pool (in FPGA memory) (id -> location, memcpy)
         *
         * process
         *
         * write flag to nic
         * write to the send message pool (in the host memory) (id -> location, store)
         */
        //    fpga_dispatch_queue
        // {
        //     sleep(1);
        //     fpga_dispatch_queue->print();
        //     auto &now_connect = nic_connect[1][0];
        //     uint64_t addr = now_connect->get_rev_addr(0);
        //     ReqHead* req_head = (ReqHead*)addr;
        //     req_head->print_head();
        // }
        // sleep(1);
        for (int ip_i = 0; ip_i < PARALLEL_IP_NUM; ip_i++) {
            now_message = ip_mess_id[ip_i] + ip_i * EACH_IP_MESS_N;
            if (my_fpga_queue->check_now_message(now_message)) {
                uint32_t client_id = my_fpga_queue->get_message_client_id(now_message);
                uint32_t slot_id = my_fpga_queue->get_message_slot_id(now_message);

                // printf("I am %lu get [%lu %lu]\n",thread_id, client_id, slot_id);
                // sleep(1);
                auto& now_connect = nic_connect[client_id / CLIENT_THREAD + KEY2NODE_MOD][client_id % CLIENT_THREAD];
                uint64_t addr = now_connect->get_rev_addr(slot_id);

                // execute
                uint64_t reply_addr = now_connect->get_send_addr(slot_id);
#ifdef TPCCBENCH
                // printf("gg = %d\n", slot_id);
                bool result = execute_tpcc_new_order(addr, reply_addr, now_message);
#else
                bool result = execute_easy_test(addr, reply_addr);
#endif
                // printf("%d read %d\n",thread_id,now_message);
                if (result)
                    my_fpga_queue->set_message_down(now_message);

                // my_fpga_queue->print();
                ip_mess_id[ip_i] = (ip_mess_id[ip_i] + 1) % EACH_IP_MESS_N;
            }
        }
    }
#endif
    return;
}

bool nictxn_rpc::execute_tpcc_new_order(uint64_t req_addr, uint64_t reply_addr, uint32_t now_message)
{
#ifdef NICNICNIC

    ReqHead* volatile req_head = (ReqHead*)req_addr;
    // {
    //     // req_head->rpcId.rpc_id += NOTHING_DEBUG;
    //     volatile uint32_t* r_addr = (volatile uint32_t*)&(((rpc_id_t*)reply_addr)->rpc_id);
    //     (*r_addr) = req_head->rpcId.rpc_id + NOTHING_DEBUG;
    //     performance.count(thread_id);
    //     return true;
    // }

    uint32_t* volatile tail = (uint32_t*)(req_addr + req_head->size);
    // int tail_check_cnt = 0;
    // while (true) {
    //     // if (tail_check_cnt>10000)
    //     tail = (uint32_t* volatile)(req_addr + req_head->size);
    //     if (*tail == req_head->magic) {
    //         // printf("OK = %lx size=%d %d %d=%d %d %d\n", req_addr, req_head->size, req_head->rpcId.rpc_id, thread_id, req_head->partition_hint, *tail, req_head->magic);
    //         // puts("");
    //         break;
    //     }
    //     printf("GGG = %lx size=%d %d %d=%d %d=?%d\n", req_addr, req_head->size, req_head->rpcId.rpc_id, thread_id, req_head->partition_hint, *tail, req_head->magic);
    //     tail_check_cnt++;
    // }
    // if (thread_id != req_head->partition_hint){
    //     print_ctl(thread_id);
    //     print_ctl(req_head->partition_hint);
    //     printf("%d %d\n",thread_id, req_head->partition_hint);
    //     assert(thread_id == req_head->partition_hint);
    //     return true;
    // }

    // *tail = thread_id;

    uint32_t req_size = req_head->size;
    // uint64_t* args_addr = (uint64_t*)(req_addr + sizeof(ReqHead) + sizeof(SIZETYPE) * req_head->req_num);

    switch_tx_status_t& status = switch_tx_status[0][0];
    status.size_addr = (uint64_t)NicTx_requeset_buf;
    status.slot_addr = (uint64_t)NicTx_requeset_buf + sizeof(SIZETYPE) * 2 * (10 + MAX_ITEM * 5);
    uint64_t now_size_addr = status.size_addr;
    uint64_t now_slot_addr = status.slot_addr;

    SwitchTXMessage m;
    m.coordinator_id = 0;
    m.TXID = 0;
    uint16_t slot_id = generate_slot_id(req_head->rpcId.my_nodeid, req_head->rpcId.thread_id, req_head->TXID);
    m.switch_slot = slot_id;

    replyPair reply;
    reqPair reqP[req_head->req_num];
    uint64_t addr_tmp = req_addr + sizeof(ReqHead) + sizeof(SIZETYPE) * req_head->req_num;
    for (int i = 0; i < req_head->req_num; i++) {
        reqP[i].size = *(SIZETYPE*)(req_addr + sizeof(ReqHead) + sizeof(SIZETYPE) * i);
        reqP[i].addr = addr_tmp;
        addr_tmp += reqP[i].size;
    }

    int main_read_count = *(int*)reqP[0].addr;
    uint64_t version_addr = (uint64_t)NicTx_tx_local_buf;
    uint64_t value_addr = (uint64_t)NicTx_tx_local_buf + (sizeof(tx_version_t) + sizeof(SIZETYPE)) * (main_read_count);
    uint64_t keys_addr = reqP[1].addr;
    uint* args_array = (uint*)reqP[2].addr;

#if (DEBUG_ON)
    req_head->print_head();
    for (int i = 0; i < NICTXTESTSIZE; i++) {
        printf("%llu ", args_array[i]);
    }
    // printf("---%d get %d %d ", thread_id, client_id, slot_id);
    my_fpga_queue->print();
#endif

    tx_key_t lock_key_e;
    bool retry = true;
    int cnt = 0;

    GET_LOCATION(c, keys_addr, value_addr, version_addr, customer::value)
    GET_LOCATION(w, keys_addr, value_addr, version_addr, warehouse::value)
    GET_LOCATION(d, keys_addr, value_addr, version_addr, district::value)
    // printf("GG = %d %d\n",thread_id, d_key.second-11);
    oorder::value v_oo; // insert - 1
    new_order::value v_no; // insert - 1
    item::value* i_value[MAX_ITEM]; // read-3
    stock::value* s_value[MAX_ITEM]; // read-4 write
    tx_key_t v_key[MAX_ITEM];
    order_line::value v_ol[MAX_ITEM]; // insert MAX_ITEM
    tx_key_t i_key[MAX_ITEM];
    tx_version_t* i_version[MAX_ITEM];
    tx_key_t s_key[MAX_ITEM];
    tx_version_t* s_version[MAX_ITEM];

    uint warehouse_id = args_array[0];
    uint districtID = args_array[1];
    uint customerID = args_array[2];
    uint numItems = args_array[3];

    uint64_t o_key_second;
    uint32_t my_next_o_id;
    uint64_t no_key_second;
    tx_key_t o_key;
    o_key.table_id_pair = ORDE;
    tx_key_t no_key;
    no_key.table_id_pair = NEWO;

    for (int i = 0; i < numItems; i++) {
        GET_LOCATION_array(i, keys_addr, value_addr, version_addr, item::value, i)
            GET_LOCATION_array(s, keys_addr, value_addr, version_addr, stock::value, i)
    }
    // while (true) {
    // retry = false;
    pair<int, int> abort_shift;

    while (retry) {

        now_size_addr = status.size_addr;
        now_slot_addr = status.slot_addr;
        status.read_slot_end = 0;
        status.write_slot_end = 0;
        uint64_t now_size_addr_write = status.size_addr + 2 * sizeof(SIZETYPE) * (3 + numItems * 2);
        uint64_t now_slot_addr_write = status.slot_addr + 2 * sizeof(size_t) * (3 + numItems * 2);
        retry = false;

        reply = execute_read_tpcc_main(reqP, -3, 0, (uint64_t)NicTx_tx_local_buf);
        // break;
        PROCEDURE_ADD_TO_READ_SET(&c_key, c_version, now_size_addr, now_slot_addr)
        PROCEDURE_ADD_TO_READ_SET(&w_key, w_version, now_size_addr, now_slot_addr)
        PROCEDURE_ADD_TO_READ_SET(&d_key, d_version, now_size_addr, now_slot_addr)

        // printf("d version = %llu\n",d_version);

        // district::value* d_value = (district::value*)value_addr; //= rtx_->get_readset<district::value>(idx,yield);
        my_next_o_id = d_value->d_next_o_id;
        d_value->d_next_o_id++;
        PROCEDURE_ADD_TO_WRITE_SET(&d_key, d_value, now_size_addr_write, now_slot_addr_write, district::value);

        // value_addr += sizeof(district::value); // read-2
        // // value_addr += sizeof(district::value);
        // // rtx_->add_to_write(); // add the last item to readset

        o_key_second = makeOrderKey(warehouse_id, districtID, my_next_o_id);
        o_key.table_key_pair = o_key_second;
        // oorder::value v_oo;
        v_oo.o_c_id = int32_t(customerID);
        v_oo.o_carrier_id = 0;
        v_oo.o_ol_cnt = int8_t(numItems);
        v_oo.o_all_local = true; // allLocal;
        v_oo.o_entry_d = GetCurrentTimeMillis();

        no_key_second = makeNewOrderKey(warehouse_id, districtID, my_next_o_id);
        no_key.table_key_pair = no_key_second;
        // new_order::value v_no;

        PROCEDURE_ADD_TO_WRITE_SET(&no_key, &v_no, now_size_addr_write, now_slot_addr_write, new_order::value);
        PROCEDURE_ADD_TO_WRITE_SET(&o_key, &v_oo, now_size_addr_write, now_slot_addr_write, oorder::value);

        for (uint ol_number = 1; ol_number <= numItems; ol_number++) {
            const uint ol_i_id = args_array[3 + ol_number * 3 - 2];
            const uint supplies = args_array[3 + ol_number * 3 - 1];
            const uint ol_quantity = args_array[3 + ol_number * 3 - 0];

            item::value* i_value_i = i_value[ol_number - 1]; //= rtx_->get_readset<item::value>(idx, yield);
            PROCEDURE_ADD_TO_READ_SET(&i_key[ol_number - 1], i_version[ol_number - 1], now_size_addr, now_slot_addr)
            // value_addr += sizeof(item::value);

            tx_key_t* s_key_i = &(s_key[ol_number - 1]);
            stock::value* s_value_i = s_value[ol_number - 1];

            // // printf("value = %d %d\n",s_value->s_quantity,s_value->s_ytd);

            if (s_value_i->s_quantity - ol_quantity >= 10)
                s_value_i->s_quantity -= ol_quantity;
            else
                s_value_i->s_quantity += (-int32_t(ol_quantity) + 91);

            s_value_i->s_ytd += ol_quantity;
            s_value_i->s_remote_cnt += (supplies == warehouse_id) ? 0 : 1;
            PROCEDURE_ADD_TO_READ_SET(s_key_i, s_version[ol_number - 1], now_size_addr, now_slot_addr)
            PROCEDURE_ADD_TO_WRITE_SET(s_key_i, s_value_i, now_size_addr_write, now_slot_addr_write, stock::value);
            // add_to_write_set_various_size(make_pair(STOC, s_key), (uint64_t)s_value, sizeof(stock::value), tx_main_partition, reply.locality);
            // // rtx_->add_to_write();
            uint64_t ol_key = makeOrderLineKey(warehouse_id, districtID, my_next_o_id, ol_number);
            v_key[ol_number - 1] = make_pair(ORLI, ol_key);
            order_line::value* v_ol_i = &v_ol[ol_number - 1];
            v_ol_i->ol_i_id = int32_t(ol_i_id);
            v_ol_i->ol_delivery_d = 0; // not delivered yet
            v_ol_i->ol_amount = float(ol_quantity) * i_value_i->i_price;
            v_ol_i->ol_supply_w_id = int32_t(supplies);
            v_ol_i->ol_quantity = int8_t(ol_quantity);
            PROCEDURE_ADD_TO_WRITE_SET(&v_key[ol_number - 1], v_ol_i, now_size_addr_write, now_slot_addr_write, stock::value);
            // add_to_write_set_various_size(make_pair(ORLI, ol_key), (uint64_t)&v_ol, sizeof(order_line::value), tx_main_partition, reply.locality);
        }

        status.read_slot_start = 0;
        status.write_slot_start = status.read_slot_end;
        status.write_slot_end += status.read_slot_end;
        bool tmp1 = false, tmp2 = false, tmp3 = false;
        if ((tmp1 = execute_lock(&m)) && (tmp2 = execute_validate(&m)) && (tmp3 = execute_commit_various_size(&m))) {
            // puts("?");
            cnt++;
            break;
        } else {
            // printf("retry %d thread_id cnt = %d %d %d %d\n", thread_id, cnt,tmp1,tmp2,tmp3);
            // printf("abort %d %d %d\n",cnt, abort_key.first, abort_key.second);
            // if (cnt < 5)
            //     my_fpga_queue->add_abort_key(now_message, cnt, abort_key.first, abort_key.second);
            abort_shift = abort_queue.push(abort_key);
            model_update.update_weight(thread_id, abort_shift);
            performance.retry_count(thread_id);
            // cnt++;
            retry = true;
        }
    }

    performance.count(thread_id);
    // }
    // req_head->rpcId.rpc_id += NOTHING_DEBUG;
    volatile uint32_t* r_addr = (volatile uint32_t*)&(((rpc_id_t*)reply_addr)->rpc_id);
    (*r_addr) = req_head->rpcId.rpc_id + NOTHING_DEBUG;
    r_addr[1] = thread_id; // last one
    // printf("%p %d %d %d\n",r_addr, *r_addr, ((rpc_id_t*)reply_addr)->rpc_id, req_head->rpcId.rpc_id);
    // *(rpc_id_t*)reply_addr = req_head->rpcId;
#endif
    return true;
}

bool nictxn_rpc::execute_easy_test(uint64_t req_addr, uint64_t reply_addr)
{
#ifdef NICNICNIC
    ReqHead* req_head = (ReqHead*)req_addr;
#ifdef PCILATENCY
    pcie_sleep();
#endif
    uint32_t req_size = req_head->size;

#ifdef PCILATENCY
    for (int i = 0; i < req_size / 8; i++) {
        pcie_sleep();
    }
#endif
    memcpy((uint8_t*)fpga_memory_buf, (uint8_t*)req_addr, req_size);

    req_head = (ReqHead*)fpga_memory_buf;
    uint64_t* args_addr = (uint64_t*)((uint64_t)fpga_memory_buf + sizeof(ReqHead) + sizeof(SIZETYPE) * req_head->req_num);

#if (DEBUG_ON)
    req_head->print_head();
    for (int i = 0; i < NICTXTESTSIZE; i++) {
        printf("%llu ", args_addr[i]);
    }
    // printf("---%d get %d %d ", thread_id, client_id, slot_id);
    my_fpga_queue->print();
#endif

    tx_key_t lock_key_e;
    uint16_t slot_id = generate_slot_id(req_head->rpcId.my_nodeid, req_head->rpcId.thread_id, req_head->TXID);

    bool retry = true;
    int cnt = 0;

    // execute

    // while (retry) {
    //     retry = false;
    //     for (int i = 0; i < NICTXTESTSIZE; i++) {
    //         lock_key_e.table_key_pair = args_addr[i];
    //         if (!lock_key(lock_key_e, slot_id)) {
    //             retry = true;
    //             break;
    //         }
    //     }
    //     if (retry) {
    //         for (int i = 0; i < NICTXTESTSIZE; i++) {
    //             lock_key_e.table_key_pair = args_addr[i];
    //             free_key(lock_key_e, slot_id);
    //         }
    //     }
    // }
    // //
    // // usleep(1);

    // for (int i = 0; i < NICTXTESTSIZE; i++) {
    //     lock_key_e.table_key_pair = args_addr[i];
    //     write_value(lock_key_e, 666);
    //     free_key(lock_key_e, slot_id);
    // }

    performance.count(thread_id);
    req_head->rpcId.rpc_id += NOTHING_DEBUG;
    *(rpc_id_t*)reply_addr = req_head->rpcId;
#endif
    return true;
}
