
#include "Transaction.h"

void initstorage(uint8_t node_id, uint8_t _test_type)
{
#ifdef MULTISWITCH
    init_bitmap(node_id);
#endif

#ifdef SMALLBANK
    test_type = 0;
    num_accounts_global = KEY2NODE_MOD * RANGEOFKEY_STORAGE;

    num_base_hot = 0;
    printf("%d\n", num_accounts_global);
    fflush(stdout);
    num_hot_global = num_accounts_global * DEFAULT_NUM_HOT / 100;
    if (node_id >= KEY2NODE_MOD)
        return;

    for (int i = 0; i < RANGEOFKEY_STORAGE * 2 + KEY2NODE_MOD * 2; i++) {
        small_bank_count[i].lock = false;
        small_bank_count[i].version = 100;
        small_bank_count[i].value = 10000;
        small_bank_count[i].server_lock_id = -1;
    }
#elif defined TPCCBENCH
    num_accounts_global = 1000;
    test_type = 1;
    schema_size[WARE] = sizeof(warehouse::value);
    schema_size[DIST] = sizeof(district::value);
    schema_size[CUST] = sizeof(customer::value);
    schema_size[HIST] = sizeof(history::value);
    schema_size[NEWO] = sizeof(new_order::value);
    schema_size[ORDE] = sizeof(oorder::value);
    schema_size[ORLI] = sizeof(order_line::value);
    schema_size[ITEM] = sizeof(item::value);
    schema_size[STOC] = sizeof(stock::value);
    // for (int i = 0 ; i < 9; i++){
    //     printf("%d ",schema_size[i]);
    // }
    // printf("x %d\n",sizeof(MAP_entry_t));

    if (node_id >= KEY2NODE_MOD) {
        return;
    }
    for (int i = 0; i < 9; i++) {

        for (int j = 0; j < tpccmapsize; j++) {
            tpcc_map[i][j].clear();
            if (i >= 4 && i <= 6) {
                tpcc_map[i][j].reserve(2 * 1024 * 1024);
                // tpcc_map[i][j].rehash(1024 * 1024*2);
            } else if (i > 6) {
                tpcc_map[i][j].reserve(128 * 1024);
            } else if (i > 2) {
                tpcc_map[i][j].reserve(32 * 1024);
            }
        }
    }
    tpcc_load(node_id);

    // exit(0);
#else
    storage_map.clear();
    tx_key_t tmp;
    for (uint64_t i = 0; i <= RANGEOFKEY_STORAGE; i++) {
        tmp.first = 0;
        tmp.second = i * 3 + node_id;
        get_value(tmp);
    }
    return;
#endif
}

void switch_tx::execution(uint64_t call_type, void* ctx)
{
    
    return;
}
