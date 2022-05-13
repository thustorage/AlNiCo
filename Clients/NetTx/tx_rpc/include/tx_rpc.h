

#ifndef TX_RPC_H
#define TX_RPC_H

#include "rpc.h"
#include "mystorage.h"
#include "Transaction.h"

struct tx_rpc : public rpc{
public:

    tx_rpc(){}
    tx_rpc(const NodeInfo &my_node_info,
        const std::vector<NodeInfo> &others,
        uint16_t thread_id_info, uint16_t port, memcached_st *_memc, int _dev_id = -1);
    
     /*For Test*/
    timespec s, e;
    double tot_time;
    timespec S, E;
    double tot_time_client;
    uint64_t test_count_client;

    switch_tx* my_tx[MAXMPL];
    
    bool flag = false;
    double anstp = 0.0;
    double ansa1 = 0.0;
    double ansa2 = 0.0;
    uint64_t null_poll = 0;
    uint64_t poll = 0;

    uint64_t ok_count = 0;
    uint64_t fail_count_l = 0;
    uint64_t fail_count_v = 0;

    switch_tx_status_t switch_tx_status[MAX_SERVER][MAXMPL];
    tx_key_t abort_key;

    latency_evaluation_t performance_lock = latency_evaluation_t(MAXMPL * MAX_SERVER);

    sb_txn_type_t* workgen_arr;
    void create_workgen_array() // sb for 
    {
        workgen_arr = new sb_txn_type_t[110];
        int i = 0, j = 0;
        j += FREQUENCY_AMALGAMATE;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::amalgamate;
        j += FREQUENCY_BALANCE;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::balance;
        j += FREQUENCY_DEPOSIT_CHECKING;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::deposit_checking;
        j += FREQUENCY_SEND_PAYMENT;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::send_payment;
        j += FREQUENCY_TRANSACT_SAVINGS;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::transact_saving;
        j += FREQUENCY_WRITE_CHECK;
        for (; i < j; i++)
            workgen_arr[i] = sb_txn_type_t::write_check;
        assert(i == 100 && j == 100);
    }

    tpcc_txn_type_t* workgen_arr_tpcc;
    void create_workgen_array_tpcc()
    {
        workgen_arr_tpcc = new tpcc_txn_type_t[110];
        int i = 0, j = 0;
        j += FREQUENCY_new_order;
        for (; i < j; i++)
            workgen_arr_tpcc[i] = tpcc_txn_type_t::new_order;
        j += FREQUENCY_pay;
        for (; i < j; i++)
            workgen_arr_tpcc[i] = tpcc_txn_type_t::pay;
        j += FREQUENCY_del;
        for (; i < j; i++)
            workgen_arr_tpcc[i] = tpcc_txn_type_t::del;
        j += FREQUENCY_order;
        for (; i < j; i++)
            workgen_arr_tpcc[i] = tpcc_txn_type_t::order;
        j += FREQUENCY_stock;
        for (; i < j; i++)
            workgen_arr_tpcc[i] = tpcc_txn_type_t::stock;
        assert(i == 100 && j == 100);
    }
    void throughput_c(uint8_t tx_id);
    bool throughput_f(uint8_t tx_id, uint8_t msg_type, bool* b);
    
    inline uint64_t get_localbase(uint8_t remote_node_id)
    {
        return connect[remote_node_id]->localBase + my_node.message_size;
    }
    inline uint16_t generate_slot_id(int tx_id) // for myself
    {
        return toBigEndian16((node_id) * (MAX_THREAD * MAXMPL) + thread_id * (MAXMPL) + tx_id * 1);
    }
    inline uint16_t generate_slot_id(int req_node_id, int req_thread_id, int tx_id) // NIC 
    {
        return (req_node_id) * (CLIENT_THREAD * MAXMPL) + req_thread_id * (ASYNCTXNUM) + tx_id * 1 + 1;
    }
    
    replyPair execute_read(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc = -1, uint8_t tx_id = 0);
    bool execute_commit(SwitchTXMessage* m);
    replyPair execute_read_fixed_size(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc, uint8_t tx_id);
    bool execute_commit_fixed_size(SwitchTXMessage* m);
    replyPair execute_read_various_size(uint16_t rpc_name, uint8_t req_num, reqPair* req_p, int mem_alloc, uint8_t tx_id);
    bool execute_commit_various_size(SwitchTXMessage* m);
    bool execute_lock(SwitchTXMessage* m);
    bool execute_validate(SwitchTXMessage* m);
    bool execute_free(SwitchTXMessage* m, uint32_t lock_failed_point = 0x3f3f3f3f);
    replyPair execute_read_tpcc_main(reqPair* req_p, int mem_alloc, uint8_t tx_id, uint64_t local_buf = 0);
    
    
};


#endif