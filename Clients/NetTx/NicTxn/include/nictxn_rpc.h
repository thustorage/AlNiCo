
#ifndef NICTXN_RPC_H
#define NICTXN_RPC_H

#include "Transaction.h"
#include "fpga.h"
#include "model_update.h"
#include "sample.h"
#include "tx_rpc.h"
#include "utils.h"


using namespace sampling;
extern model_update_t model_update;

#define GET_LOCATION(dst, src0, src1, src2, type)        \
    tx_key_t dst##_key = *(tx_key_t*)(src0);             \
    src0 += sizeof(tx_key_t);                            \
    type* dst##_value = (type*)(src1);                   \
    src1 += sizeof(type);                                \
    tx_version_t* dst##_version = (tx_version_t*)(src2); \
    src2 += sizeof(tx_version_t);

#define GET_LOCATION_array(dst, src0, src1, src2, type, id) \
    dst##_key[id] = *(tx_key_t*)(src0);                     \
    src0 += sizeof(tx_key_t);                               \
    dst##_value[id] = (type*)(src1);                        \
    src1 += sizeof(type);                                   \
    dst##_version[id] = (tx_version_t*)(src2);              \
    src2 += sizeof(tx_version_t);

#define PROCEDURE_ADD_TO_READ_SET(key_addr, version_addr, size_addr, slot_addr) \
    *(SIZETYPE*)size_addr = 8;                                                  \
    size_addr += sizeof(SIZETYPE);                                              \
    *(SIZETYPE*)size_addr = 8;                                                  \
    size_addr += sizeof(SIZETYPE);                                              \
    *(uint64_t*)slot_addr = (uint64_t)(key_addr);                               \
    slot_addr += 8;                                                             \
    *(uint64_t*)slot_addr = (uint64_t)(version_addr);                           \
    slot_addr += 8;                                                             \
    (status.read_slot_end)++;

#define PROCEDURE_ADD_TO_WRITE_SET(key_addr, value_addr, size_addr, slot_addr, type) \
    *(SIZETYPE*)size_addr = 8;                                                       \
    size_addr += sizeof(SIZETYPE);                                                   \
    *(SIZETYPE*)size_addr = sizeof(type);                                            \
    size_addr += sizeof(SIZETYPE);                                                   \
    *(uint64_t*)slot_addr = (uint64_t)(key_addr);                                    \
    slot_addr += 8;                                                                  \
    *(uint64_t*)slot_addr = (uint64_t)(value_addr);                                  \
    slot_addr += 8;                                                                  \
    (status.write_slot_end)++;

struct nictxn_rpc;
struct nictxn_tx : public switch_tx {
    // switchtx_tx()

    nictxn_rpc* nictxn_rpc_ptr;
    // int now_phase;
    nictxn_tx(nictxn_rpc* nictxn_rpc_ptr_, uint8_t _tx_id, uint64_t _global_id, uint8_t _node_id, uint8_t _thread_id);
    void HOT();
    void tpcc_procedure();
    void YCSB_procedure();
    void YCSB_partitional();
    void new_order_procedure();
    void payment_procedure();
    void order_status();
    void stock_level();
    void delivery();
    void sys_execution(uint64_t call_type, void* ctx);
    uint8_t feature[128];
    void null_rpc();
    // nicnic TODO:
    // void txn_single_W();
    void single_tx_test();
};

struct nictxn_rpc : public tx_rpc {
public:
    bool no_delivery_flag[IDMAPADDRSIZE];
    uint64_t strife_cqe[IDMAPADDRSIZE];
    uint16_t MPL;
    RdmaContext context_dual;

    sampling::StoRandomDistribution<>* dd;

    volatile fpga_dispatch_queue_t* my_fpga_queue;

    abort_queue_t abort_queue;

    rdma_memory_t double_recv_mm;

    /* for debug */
    int message_id = 0;
    uint8_t fpga_memory_buf[SLOT_LEN];
    uint8_t* NicTx_tx_local_buf;
    uint8_t* NicTx_requeset_buf;

    Connect* nic_connect[MAX_SERVER][CLIENT_THREAD];

    uint16_t worker_id;
    int hot_record[10][20];
    nictxn_rpc()
    {
    }
    nictxn_rpc(const NodeInfo& my_node_info,
        const std::vector<NodeInfo>& others,
        uint16_t thread_id_info, uint16_t port, memcached_st* _memc, int _dev_id = -1);

    void init_connect();
    void nictxn_new_connect(uint16_t remote_id, uint16_t client_thread_id = 0);
    bool nictxn_fill_connect(uint16_t remote_id, uint16_t client_thread_id = 0);
    void run_server();
    void run_worker();
    void run_client();
    void run_tpcc_client();
    void generate_tx(uint8_t tx_array_id);
    bool execute_tpcc_new_order(uint64_t req_addr, uint64_t reply_addr, uint32_t now_message);
    bool execute_easy_test(uint64_t req_addr, uint64_t reply_addr);
    bool rdma_read_reply_nictx(uint64_t node_id, uint64_t source, uint64_t size, bool async);

    bool double_rdma_write(uint64_t node_id, uint64_t source_d, uint64_t size_d, uint64_t source_m, uint64_t size_m);
    bool rdmaWrite_client(uint64_t node_id, uint64_t source, uint64_t size,
        uint64_t dest, uint32_t rkey);
    rpc_id_t push_new_call_write(uint64_t NodeID, RPCTYPE rpc_type, reqPair* req, uint16_t req_num, uint16_t txn_type,
        uint8_t tx_id, uint32_t partition_hint, uint64_t feature_addr);
    // inline rpc_id_t nic_commit(uint64_t NodeID, TODO: rpc send msg
    //     reqPair* req, uint16_t read_num, uint16_t write_num, uint8_t tx_id, uint8_t thread_ex)
    // {

    //     return push_new_call(NodeID, (RPCTYPE)thread_ex, req, (read_num + write_num) * 2 + 1, read_num, write_num,
    //         tx_id, 0,
    //         0, CallerType::NO_REPLY, NULL);
    // }
    bool update_reply_flag(uint32_t rpc_id, uint16_t server_id);
    bool delivery_reply_ok(uint32_t rpc_id);
};

#endif