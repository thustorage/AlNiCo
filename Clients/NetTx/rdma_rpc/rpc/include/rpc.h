
#ifndef RPC_H
#define RPC_H

#include "Debug.h"
#include "HugePageAlloc.h"
#include "Rdma.h"
#include "latency_evaluation.h"
#include "rpc_struct.h"
#include <libmemcached/memcached.h>
#include <vector>
struct rpc {
private:
    int cmake_test;

public:
#ifdef BREAKDOWN
    latency_evaluation_t breakdown_issue = latency_evaluation_t(MAXMPL);
    latency_evaluation_t breakdown_total = latency_evaluation_t(MAXMPL);
    latency_evaluation_t breakdown_coordination = latency_evaluation_t(MAXMPL);
#endif
    latency_evaluation_t performance_total = latency_evaluation_t(MAXMPL);

    uint64_t my_global_id = -1; // node_id + thread_id

    NodeInfo my_node;
    std::vector<NodeInfo> others_server;
    uint16_t thread_id;
    uint16_t node_id;
    uint16_t eris_global_id;

    uint8_t dev_id;
    uint64_t rpc_seed;

    RdmaContext context;
    ibv_mr* message_mr; // memory region
    ibv_cq* cq; // all recv cq
    ibv_cq* send_cq; // all send cq
    ibv_cq* cq_dual;
    ibv_cq* send_cq_dual;
    uint32_t message_Lkey; // for control_connection
    memcached_st* memc; // memcached

    // ibv_wc wc;
    ibv_wc wc_a[10];
    batch_sort_t batch_sort[10];

    /*For Connection*/
    uint64_t mm; // message pool address
    uint64_t mm_size; // message pool size
    rdma_memory_t send_mm; // cpu
    rdma_memory_t recv_mm; // fpga

    UD_DC_context_t** UD_DC_context; 
    Connect** connect; // rc rdma
    

    uint64_t rpcnum[MAX_SERVER];
    IdMapAddr* idMapAddr[MAX_SERVER];
    uint64_t rpccount;
    rpc();
    rpc(const NodeInfo& my_node_info, const std::vector<NodeInfo>& others,
        uint16_t thread_id_info, uint16_t port, memcached_st* _memc,
        int _dev_id = -1);
    rpc(const NodeInfo& my_node_info, const std::vector<NodeInfo>& others,
        uint16_t thread_id_info, uint16_t worker_id);
    rpc(const std::vector<NodeInfo>& others,
        uint16_t thread_id_info);
    
    void alloc_rdma_mm();

    void post_rec(Connect* connect);

    ExchangeMeta localMeta;
    void write_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer,
        uint16_t client_thread_id = 0);
    bool read_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer, uint16_t client_thread_id = 0);
    void connect_read();

    /*Easy Function*/
    inline std::string setKey(uint16_t remote_id)
    {
        return "S" + std::to_string(my_node.node_id)
            + "S" + std::to_string(remote_id)
            + "S" + std::to_string(thread_id);
    }

    inline std::string getKey(uint16_t remote_id)
    {
        return "S" + std::to_string(remote_id)
            + "S" + std::to_string(my_node.node_id)
            + "S" + std::to_string(thread_id);
    }
    inline std::string setKey_nic(int16_t remote_id, uint16_t client_thread_id)
    {
        return "S" + std::to_string(my_node.node_id)
            + "S" + std::to_string(remote_id)
            + "S" + std::to_string(client_thread_id); // region id in dct
    }
    inline std::string getKey_nic(uint16_t remote_id, uint16_t client_thread_id)
    {
        return "S" + std::to_string(remote_id)
            + "S" + std::to_string(my_node.node_id)
            + "S" + std::to_string(client_thread_id);
    }
    inline std::string getKey_dct(uint16_t remote_id, uint16_t region_id)
    {
        return "S" + std::to_string(remote_id)
            + "S" + std::to_string(-1)
            + "S" + std::to_string(region_id);
    }
    /* useless.cpp */
    bool poll_signal(bool pollflag, bool sync_op, uint64_t node_id);
    bool rdmaWrite(uint64_t node_id, uint64_t source, uint64_t size, int32_t imm, bool async);


    /* rpc_util.cpp*/
    uint8_t bit_map2num(uint16_t bitmapx);
    uint8_t bit_map2num64(uint32_t* bit_eris_per_server);
    uint8_t bit_map2num64(uint16_t* bit_eris_per_server);
    
    /*rpc interface*/
};

#endif