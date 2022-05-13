#include "tx_rpc.h"
replyPair tx_rpc::execute_read(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc, uint8_t tx_id)
{

    if (rpc_name == TPCCREAD) {
        assert(req_num == 2);
        return execute_read_tpcc_main(req_p, mem_alloc, tx_id);
    }
    if (test_type == 0) {
        return execute_read_fixed_size(rpc_name, req_num, req_p, mem_alloc, tx_id);
    } else {
        return execute_read_various_size(rpc_name, req_num, req_p, mem_alloc, tx_id);
    }
}
replyPair tx_rpc::execute_read_tpcc_main(reqPair* req_p, int mem_alloc, uint8_t tx_id, uint64_t local_buf)
{
    int main_read_count = *(int*)req_p[0].addr;
    // printf("main_ra")
    tx_key_t* main_read = (tx_key_t*)req_p[1].addr;
    replyPair RP;
    // calculate the size;
    tx_key_t* x;
    // Debug::notifyError("main_read_count=%d",main_read_count);
    RP.size = main_read_count * (sizeof(SIZETYPE) + sizeof(tx_version_t));
    for (int i = 0; i < main_read_count; i++) {
        x = &main_read[i];
        RP.size += schema_size[x->table_id_pair];
        // Debug::notifyInfo("size = %d\n", schema_size[x->table_id_pair]);
    }
    if (mem_alloc == -1)
        RP.addr = (uint64_t)malloc(RP.size);
    else if (mem_alloc == -2) {
        my_tx[tx_id]->tx_alloc(&(RP.addr), RP.size);
    } else if (mem_alloc == -3) {
        RP.addr = local_buf;
    } else {
        connect[mem_alloc]->getsendslot(RP.size + sizeof(ReplyHead), &(RP.addr));
        RP.addr += sizeof(ReplyHead);
    }
    tx_version_t* version = (tx_version_t*)RP.addr;
    SIZETYPE* size_v = (SIZETYPE*)(RP.addr + main_read_count * sizeof(tx_version_t));
    uint64_t value_addr = (RP.addr + main_read_count * (sizeof(tx_version_t) + sizeof(SIZETYPE)));

    for (int i = 0; i < main_read_count; i++) {
        
        x = &main_read[i];
        get_value_various_size(*x, value_addr, &version[i], &size_v[i], 0);
        // if (x->first == 1){
        //     printf("read %d %llu\n",x->second, version[i]);
        // }
        // uint64_t* test = (uint64_t*)value_addr;
        // for (int j = 0; j < 10; j++) {
        //     printf("%ull ", test[j]);
        // }
        // puts("");
        // Debug::notifyInfo("size = %d\n", size_v[i]);
        value_addr += size_v[i];
    }
    return RP;
}
replyPair tx_rpc::execute_read_fixed_size(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc, uint8_t tx_id)
{
    replyPair RP;
    // Debug::notifyError("I got %d read ",req_num);
    RP.size = req_num * (sizeof(tx_value_t) + sizeof(tx_version_t));
    if (mem_alloc == -1)
        RP.addr = (uint64_t)malloc(RP.size);
    else if (mem_alloc == -2) {
        my_tx[tx_id]->tx_alloc(&RP.addr, RP.size);
    } else {
        connect[mem_alloc]->getsendslot(RP.size + sizeof(ReplyHead), &RP.addr);
        RP.addr += sizeof(ReplyHead);
    }
    tx_version_t* version = (tx_version_t*)RP.addr;
    tx_value_t* value = (tx_value_t*)(RP.addr + req_num * sizeof(tx_version_t));
    tx_key_t* x;
    for (int i = 0; i < req_num; i++) {
        x = (tx_key_t*)req_p[i].addr;
        // assert( (*x) % KEY2NODE_MOD != node_id);
        if (x->table_key_pair % KEY2NODE_MOD != node_id) {
            printf("%llu %llu  \n", (*x), node_id);
        }
        value[i] = get_value(*x);
        version[i] = get_version(*x);
    }
    return RP;
}
replyPair tx_rpc::execute_read_various_size(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc, uint8_t tx_id)
{
    replyPair RP;
    // calculate the size;
    tx_key_t* x;

    RP.size = req_num * (sizeof(SIZETYPE) + sizeof(tx_version_t));
    for (int i = 0; i < req_num; i++) {
        x = (tx_key_t*)req_p[i].addr;
        RP.size += schema_size[x->table_id_pair];
        // Debug::notifyInfo("size = %d\n", schema_size[x->table_id_pair]);
    }
    if (mem_alloc == -1)
        RP.addr = (uint64_t)malloc(RP.size);
    else if (mem_alloc == -2) {
        my_tx[tx_id]->tx_alloc(&RP.addr, RP.size);
    } else {
        connect[mem_alloc]->getsendslot(RP.size + sizeof(ReplyHead), &RP.addr);
        RP.addr += sizeof(ReplyHead);
    }

    tx_version_t* version = (tx_version_t*)RP.addr;
    SIZETYPE* size_v = (SIZETYPE*)(RP.addr + req_num * sizeof(tx_version_t));
    uint64_t value_addr = (RP.addr + req_num * (sizeof(tx_version_t) + sizeof(SIZETYPE)));

    for (int i = 0; i < req_num; i++) {
        x = (tx_key_t*)req_p[i].addr;

        get_value_various_size(*x, value_addr, &version[i], &size_v[i], 0);
        // uint64_t* test = (uint64_t*)value_addr;
        // for (int j = 0; j < 10; j++) {
        //     printf("%ull ", test[j]);
        // }
        // puts("");
        // Debug::notifyInfo("size = %d\n", size_v[i]);
        value_addr += size_v[i];
    }
    return RP;
}
bool tx_rpc::execute_commit(SwitchTXMessage* m)
{
    if (test_type == 0) {
        return execute_commit_fixed_size(m);
    } else {
        return execute_commit_various_size(m);
    }
}
bool tx_rpc::execute_commit_fixed_size(SwitchTXMessage* m)
{
    auto& status = switch_tx_status[m->coordinator_id][m->TXID];
    performance_lock.end(m->coordinator_id * MAXMPL + m->TXID);
    // key version
    // clock_gettime(CLOCK_REALTIME, &status.lock_e);
    // double f = (status.lock_e.tv_nsec - status.lock_s.tv_nsec) * 1.0 + (status.lock_e.tv_sec - status.lock_s.tv_sec) * 1.0 * S2N;
    // status.time_a += f;
    // status.time_count++;;
    SIZETYPE* addr_size = (SIZETYPE*)(status.size_addr);
    uint64_t addr_slot = status.slot_addr + (sizeof(tx_key_t) + sizeof(tx_version_t)) * status.read_slot_end;
#ifdef NICNICNIC

    // usleep(1);
#endif
    for (int i = status.write_slot_start; i < status.write_slot_end; i += 1) {
        tx_key_t lock_key_e = *(tx_key_t*)addr_slot;
        addr_slot += addr_size[i * 2];
        tx_value_t tx_value = *(tx_value_t*)addr_slot;
        addr_slot += addr_size[i * 2 + 1];
        write_value(lock_key_e, tx_value);

        free_key(lock_key_e, m->switch_slot);
        // clock_gettime(CLOCK_REALTIME,&e);
        // tot_time += ((e.tv_nsec-s.tv_nsec)*1.0 + (e.tv_sec - s.tv_sec)*1.0*S2N );
    }
    return true;
}
bool tx_rpc::execute_commit_various_size(SwitchTXMessage* m)
{
    auto& status = switch_tx_status[m->coordinator_id][m->TXID];
    performance_lock.end(m->coordinator_id * MAXMPL + m->TXID);
    // key version
    // clock_gettime(CLOCK_REALTIME, &status.lock_e);
    // double f = (status.lock_e.tv_nsec - status.lock_s.tv_nsec) * 1.0 + (status.lock_e.tv_sec - status.lock_s.tv_sec) * 1.0 * S2N;
    // status.time_a += f;
    // status.time_count++;
    SIZETYPE* addr_size = (SIZETYPE*)(status.size_addr);

#ifdef NICNICNIC
    uint64_t addr_slot = status.slot_addr + (sizeof(size_t) + sizeof(size_t)) * status.read_slot_end;
#else
    uint64_t addr_slot = status.slot_addr + (sizeof(tx_key_t) + sizeof(tx_version_t)) * status.read_slot_end;
#endif

    for (int i = status.write_slot_start; i < status.write_slot_end; i += 1) {

#ifdef NICNICNIC
        tx_key_t lock_key_e = *(tx_key_t*)(*(uint64_t*)addr_slot);
#ifdef TPCCBENCH
        write_value_various_size(lock_key_e, *(uint64_t*)(addr_slot + addr_size[i * 2]), addr_size[i * 2 + 1], *(uint64_t*)addr_slot);
#else
        write_value_various_size(lock_key_e, *(uint64_t*)(addr_slot + addr_size[i * 2]), addr_size[i * 2 + 1]);
#endif
        addr_slot += addr_size[i * 2] + sizeof(size_t);
        // free_key(lock_key_e, m->switch_slot);
#else
        tx_key_t lock_key_e = *(tx_key_t*)addr_slot;
        addr_slot += addr_size[i * 2];
        write_value_various_size(lock_key_e, addr_slot, addr_size[i * 2 + 1]);
        addr_slot += addr_size[i * 2 + 1];
        // free_key(lock_key_e, m->switch_slot);
#endif
    }
    return true;
}

bool tx_rpc::execute_lock(SwitchTXMessage* m)
{

    auto& status = switch_tx_status[m->coordinator_id][m->TXID];
    // key version
    // clock_gettime(CLOCK_REALTIME, &status.lock_s);
    performance_lock.begin(m->coordinator_id * MAXMPL + m->TXID);
    SIZETYPE* addr_size = (SIZETYPE*)(status.size_addr);
#ifdef NICNICNIC
    uint64_t addr_slot = status.slot_addr + (sizeof(size_t) + sizeof(size_t)) * status.read_slot_end;
#else
    uint64_t addr_slot = status.slot_addr + (sizeof(tx_key_t) + sizeof(tx_version_t)) * status.read_slot_end;
#endif
    tx_key_t lock_key_e;
    for (int i = status.write_slot_start; i < status.write_slot_end; i += 1) {

#ifdef NICNICNIC
        lock_key_e = *(tx_key_t*)(*(uint64_t*)addr_slot);
#ifdef TPCCBENCH
        if (!lock_key(lock_key_e, m->switch_slot, addr_slot)) { // save the index searching
            execute_free(m, i);
            abort_key = lock_key_e;
            return false;
        }
#else
        if (!lock_key(lock_key_e, m->switch_slot)) {
            execute_free(m, i);
            return false;
        }
#endif
        addr_slot += addr_size[i * 2] + sizeof(size_t);
#else
        tx_key_t lock_key_e = *(tx_key_t*)addr_slot;
        addr_slot += addr_size[i * 2] + addr_size[i * 2 + 1];
        if (!lock_key(lock_key_e, m->switch_slot)) {
            execute_free(m, i);
            return false;
        }
#endif
    }

    return true;
}
bool tx_rpc::execute_validate(SwitchTXMessage* m)
{
    auto& status = switch_tx_status[m->coordinator_id][m->TXID];
    // clock_gettime(CLOCK_REALTIME, &status.v_e);
    // double f = (status.v_e.tv_nsec - status.v_s.tv_nsec) * 1.0 + (status.v_e.tv_sec - status.v_s.tv_sec) * 1.0 * S2N;
    // status.time_v += f;
    // status.v_count++;

    // key version
    SIZETYPE* addr_size = (SIZETYPE*)(status.size_addr);
    uint64_t addr_slot = status.slot_addr;
    // printf("valid",addr_slot);
    tx_key_t validate_key;
    tx_version_t validate_version;
    for (int i = status.read_slot_start; i < status.read_slot_end; i += 1) {
#ifdef NICNICNIC
        validate_key = *(tx_key_t*)(*(uint64_t*)addr_slot);
        // printf("vv = validate_key=%lld\n",validate_key);
        addr_slot += addr_size[i * 2];
        validate_version = *(tx_version_t*)(*(uint64_t*)addr_slot);
#else
        validate_key = *(tx_key_t*)addr_slot;
        // printf("vv = validate_key=%lld\n",validate_key);
        addr_slot += addr_size[i * 2];
        validate_version = *(tx_version_t*)addr_slot;
#endif

        if (!validation_key(validate_key, validate_version, m->switch_slot)) {
            // #if (DEBUG_ON)
            //     Debug::notifyError("key(%d,%d)",validate_key.first,validate_key.second);
            // #endif
            // Debug::notifyError("free lock key(%d,%d)",validate_key.first,validate_key.second);
            execute_free(m);
            abort_key = validate_key;
            return false;
        }
        addr_slot += addr_size[i * 2 + 1];
    }

    return true;
}

bool tx_rpc::execute_free(SwitchTXMessage* m, uint32_t lock_failed_point)
{

    auto& status = switch_tx_status[m->coordinator_id][m->TXID];
    // if (status.status != ServerTxStatus::LOCKED) return true;
    // key version
    if (status.write_slot_start != status.write_slot_end) {
        performance_lock.end(m->coordinator_id * MAXMPL + m->TXID);
        // clock_gettime(CLOCK_REALTIME, &status.lock_e);
        // double f = (status.lock_e.tv_nsec - status.lock_s.tv_nsec) * 1.0 + (status.lock_e.tv_sec - status.lock_s.tv_sec) * 1.0 * S2N;
        // status.time_a += f;
        // status.time_count++;
    }
    SIZETYPE* addr_size = (SIZETYPE*)(status.size_addr);
#ifdef NICNICNIC
    uint64_t addr_slot = status.slot_addr + (sizeof(size_t) + sizeof(size_t)) * status.read_slot_end;
#else
    uint64_t addr_slot = status.slot_addr + (sizeof(tx_key_t) + sizeof(tx_version_t)) * status.read_slot_end;
#endif
    uint32_t free_point = min(status.write_slot_end, lock_failed_point);
    for (int i = status.write_slot_start; i < free_point; i += 1) {

#ifdef NICNICNIC
        tx_key_t lock_key_e = *(tx_key_t*)(*(uint64_t*)addr_slot);

#ifdef TPCCBENCH
        free_key(lock_key_e, m->switch_slot, *(uint64_t*)addr_slot);
#else
        free_key(lock_key_e, m->switch_slot);
#endif
        addr_slot += addr_size[i * 2] + 8;

#else // not nicnicnic
        tx_key_t lock_key_e = *(tx_key_t*)addr_slot;
        addr_slot += addr_size[i * 2] + addr_size[i * 2 + 1];
        free_key(lock_key_e, m->switch_slot);
#endif
    }
    return true;
}
