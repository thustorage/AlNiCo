#ifndef _RPC_H__
#define _RPC_H__
#include "rpc_common.h"
#include <libmemcached/memcached.h>
#include <fcntl.h>
#include <unistd.h>
#include "fpga.h"
typedef std::pair<uint64_t, uint64_t> tx_key_t;
extern latency_evaluation_t performance;
extern latency_evaluation_t performance_listen;

// #define STRIFE

extern latency_evaluation_t strife_batch;
extern volatile uint64_t strife_phase[MAX_CORE][8];
extern volatile uint64_t tpcc_reply_delivery[MAX_CLIENT][CLIENT_THREAD];
extern volatile uint64_t now_phase;

void init_strife_queue();

// 64 + 64 * 255

memcached_st *connectMemcached();
void reply_write();
void reply_write_strife();
void phase_sync_strife();
void tpcc_reply();
void init_strife();
void init_strife_phase();
void init_tpcc_delivery();
// void reply_write_dual(int part); // two thread;

inline int hash_k2f(tx_key_t key)
{
    return feature_partition.offset[key.first] + (key.second % feature_partition.size[key.first]);
}

inline int hash_k2f_ycsb(uint32_t col, uint32_t key)
{
    // return col+10;
    return col * (FEATURE_SIZE / YCSBCOL) + (key % (FEATURE_SIZE / YCSBCOL));
}

struct abort_queue_t
{
    queue<int> abort_key;
    int abort_depth;
    void init()
    {
        abort_depth = 0;
        while (!abort_key.empty())
        {
            abort_key.pop();
        }
    }
    pair<int, int> push(tx_key_t new_key)
    {
        pair<int, int> ret;
        if (abort_depth == ABORT_DEPTH)
        {
            ret.first = abort_key.front();
            abort_key.pop();
        }
        else
            ret.first = FEATURE_SIZE + 1;
        ret.second = hash_k2f(new_key);
        abort_key.push(ret.second);
        return ret;
    }
    pair<int, int> push_ycsb(uint32_t col, uint32_t key)
    {
        pair<int, int> ret;
        if (abort_depth == ABORT_DEPTH)
        {
            ret.first = abort_key.front();
            abort_key.pop();
        }
        else
        {
            ret.first = FEATURE_SIZE + 1;
#ifdef DYNAMIC
            abort_depth++; // 1DYNAMIC !!!!
#endif
        }
        ret.second = hash_k2f_ycsb(col, key);
// puts("??abort");
// printf("%d %d %d\n",col, key, ret.second);
#ifdef DYNAMIC
        abort_key.push(ret.second); // 1DYNAMIC !!!!
#endif
        return ret;
    }
};

struct rdma_memory_t
{
    uint64_t mm_part1;
    uint64_t part1_size;
    ibv_mr *mr_part1;
    uint64_t mm_part2;
    uint64_t part2_size;
    ibv_mr *mr_part2;
    inline void init_single(uint64_t addr, uint64_t size, RdmaContext *ctx)
    {
        return;
    }
    inline void init_dual(uint64_t addr, uint64_t size, RdmaContext *ctx, RdmaContext *ctx_dual)
    {
        memset((uint8_t *)addr, 0, size);
        mm_part1 = addr;
        mm_part2 = addr + size / 2;
        part1_size = part2_size = size / 2;
        mr_part1 = createMemoryRegion(mm_part1, part1_size, ctx);
        mr_part2 = createMemoryRegion(mm_part2, part2_size, ctx_dual);
        printf("key = %d %d\n", mr_part1->rkey, mr_part2->rkey);
        return;
    }
};

struct Rpc
{
private:
    /*Base infof ID*/
    NodeInfo my_node;
    std::vector<NodeInfo> others_server;
    uint16_t thread_id;
    uint16_t node_id;
    uint64_t my_global_id = -1; // node_id + thread_id
    uint8_t dev_id;
    /*For Connection*/

    uint64_t mm;           // message pool address
    uint64_t mm_size;      // message pool size
    rdma_memory_t send_mm; // cpu
    rdma_memory_t recv_mm; // fpga
    rdma_memory_t double_recv_mm;

    volatile fpga_dispatch_queue_t *my_fpga_queue;

    uint64_t mm_message_buffer;

    ibv_mr *message_mr; // memory region
    ibv_cq *cq;         // all recv cq
    ibv_cq *send_cq;    // all send cq
    ibv_cq *send_cq_dual;
    ibv_cq *cq_dual;
    uint32_t message_Lkey; // for control_connection
    memcached_st *memc;    // memcached

    // /*As A Server */
    // latency_evaluation_t performance_lock = latency_evaluation_t(ASYNCTXNUM * MAX_SERVER);
    // latency_evaluation_t performance_total = latency_evaluation_t(ASYNCTXNUM);

    // Records the abort key
    tx_key_t abort_key;
    abort_queue_t abort_queue;

    // switch_tx_status_t switch_tx_status[1][1];

public:
    /*Rpc_init.cpp*/
    RdmaContext context;
    RdmaContext context_dual;

    Connect *nic_connect[MAX_SERVER][CLIENT_THREAD];
    // struct Rpc* NIC;
    Rpc() {}
    Rpc(const NodeInfo &my_node_info, const std::vector<NodeInfo> &others, memcached_st *_memc, int _dev_id);
    Rpc(const NodeInfo &my_node_info, const std::vector<NodeInfo> &others, uint16_t thread_id_info, uint16_t worker_id);
    void post_rec(Connect *connect);

    /*rdma_connect.cpp*/
    void init_connect();
    void server_connect();

    ExchangeMeta localMeta;
    void write_connect(uint16_t remote_id, RdmaContext *context_ptr, Connect *peer,
                       uint16_t client_thread_id = 0);
    void nictxn_new_connect(uint16_t remote_id, uint16_t client_thread_id = 0);
    bool nictxn_fill_connect(uint16_t remote_id, uint16_t client_thread_id = 0);
    bool read_connect(uint16_t remote_id, RdmaContext *context_ptr, Connect *peer, uint16_t client_thread_id = 0);
    bool rdma_write_reply_nictx(RdmaContext *context_ptr, Connect *connect, ibv_send_wr *wr, bool inline_flag = true);
    bool poll_signal(RdmaContext *context_ptr, Connect *connect, bool pollflag, bool sync_op);

    /*Easy Function*/
    inline std::string setKey(uint16_t remote_id)
    {
        return "S" + std::to_string(my_node.node_id) + "S" + std::to_string(remote_id) + "S" + std::to_string(thread_id);
    }

    inline std::string getKey(uint16_t remote_id)
    {
        return "S" + std::to_string(remote_id) + "S" + std::to_string(my_node.node_id) + "S" + std::to_string(thread_id);
    }
    inline std::string setKey_nic(uint16_t remote_id, uint16_t client_thread_id)
    {
        return "S" + std::to_string(my_node.node_id) + "S" + std::to_string(remote_id) + "S" + std::to_string(client_thread_id);
    }
    inline std::string getKey_nic(uint16_t remote_id, uint16_t client_thread_id)
    {
        return "S" + std::to_string(remote_id) + "S" + std::to_string(my_node.node_id) + "S" + std::to_string(client_thread_id);
    }
    // inline uint64_t get_localbase(uint8_t remote_node_id)
    // {
    //     return connect[remote_node_id]->localBase + my_node.message_size;
    // }
};

extern Rpc *server;
/*rpc core*/

#endif