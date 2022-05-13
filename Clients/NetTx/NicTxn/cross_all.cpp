/*
 * @Date: 2020-10-14 09:47:43
 * @LastEditTime: 2021-11-19 20:56:41
 * @FilePath: /TxSys/src/transaction/cross_all.cpp
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "nictxn_rpc.h" // TODO:


// void nictxn_tx::txn_single_W()
// {
//     int num = 12;
//     tx_key_t a;
//     nic = ((hrd_fastrand(&seed))) % (MAX_THREAD) + 1;
//     for (int i = 0; i < num; i++) {
//         get_various_account(&seed, &a.table_key_pair, nic);
//         add_to_write_set(a, 1);
//     }

//     a.table_key_pair = nic;
//     add_to_write_set(a, 1);
//     return;
//     // for (int i = 0; i < 1; i++) {
//     //     get_various_account(&seed, &a.table_key_pair,0);
//     //     add_to_write_set(a, 1);
//     // }
// }
void nictxn_tx::single_tx_test()
{
    int num = NICTXTESTSIZE;
    tx_key_t a;
    uint64_t* tmp = (uint64_t*)my_ctx;
    nic = xorshift128() % (MAX_THREAD);

    for (int i = 0; i < num; i++) {
        get_account_zipf_offset(&state, &tmp[i], nic);

        // get_various_account(&seed, &tmp[i], nic);
#if (DEBUG_ON)
        printf("%llu ", tmp[i]);
#endif
    }
    // if (thread_id == 0 && nic == 0) {
    //     printf("[%d]  ",nic);
    //     for (int i = 0; i < num; i++) {

    //         printf("%llu ", tmp[i]);
    //     }
    //     puts("");
    // }

    req_pair[0].addr = (uint64_t)my_ctx;
    req_pair[0].size = num * sizeof(uint64_t);
    // TODO:
    single_node_id = 0;
    // rpc->push_new_call_write(0, RPCTYPE::JUST_COMMIT, req_pair, 1, 0, num, tx_id, nic, 0);

    return;
}
