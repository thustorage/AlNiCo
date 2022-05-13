
#ifndef RPC_STRUCT_H
#define RPC_STRUCT_H

#include "Debug.h"
#include "Rdma.h"
#include <stdint.h>
#include <string>
#include <assert.h>

#define IDMAPADDRSIZE 1000

struct rdma_memory_t {
    uint64_t mm_part1;
    uint64_t part1_size;
    ibv_mr* mr_part1;
    uint64_t mm_part2;
    uint64_t part2_size;
    ibv_mr* mr_part2;
    inline void init_single(uint64_t addr, uint64_t size, RdmaContext* ctx){
        
        return;
    }
    inline void init_dual(uint64_t addr, uint64_t size, RdmaContext* ctx, RdmaContext* ctx_dual){
        memset((uint8_t *)addr, 0, size);
        mm_part1 = addr;
        mm_part2 = addr + size/2;
        part1_size = part2_size = size/2;
        mr_part1 = createMemoryRegion(mm_part1, part1_size, ctx);
        mr_part2 = createMemoryRegion(mm_part2, part2_size, ctx_dual);
        return;
    }
};

enum class SERVERTYPE {
    CLIENT_SERVER = 0, // [X]:[X]
    CLIENT_ONLY, // [X]:X | [1]:1+X | [X]:1
    SERVER_ONLY, // X:[X]
    SERVER_NIC, // 1:[1]+X
    WORKER_ONLY, // 1:1+[X]
    SERVER_REGION // X:[1]
};

struct NodeInfo {
    uint16_t node_id;
    uint16_t server_num;
    uint16_t client_only_num;
    uint16_t thread_num;
    uint32_t message_size; // nictxn's single message, switchtx's buf
    uint32_t message_size_rpc_reply;
    uint32_t double_write_data_size;
    uint32_t write_message_size;
    SERVERTYPE server_type; // cs / c / n / w
    uint32_t ddma_client_local_buffer_size;
    uint32_t ddma_total_server_mm_size;
    uint32_t ddma_mm_region;
    char ip[20];
};

enum class CallerType {
    TEMPLATE_CALL,
    REPLY_COPY_CALL,
    CALLBACK_CALL,
    NO_REPLY,
};

struct IdMapAddr {
    uint64_t addr;
    CallerType type;
    

    void* ctx;

    IdMapAddr()
        : addr(0)
    {
    }

    inline bool is_empty() { return addr == 0; }

    inline bool is_no_copy() { return addr == NO_COPY_ADDR(); }

    inline static uint64_t NO_COPY_ADDR() { return ~(0ull); }
};

struct reqPair {
    uint64_t addr;
    SIZETYPE size;

    reqPair(uint64_t _addr, SIZETYPE _size)
    {
        addr = _addr;
        size = _size;
    }

    reqPair() = default;

    inline void set_value(const void* _addr, SIZETYPE _size)
    {
        addr = (uint64_t)_addr;
        size = _size;
    }

    inline void set_string(const std::string& s)
    {
        addr = (uint64_t)s.c_str();
        size = s.size();
    }

    inline void set_interval(void* start, const void* end)
    {
        addr = (uint64_t)start;
        size = (char*)end - (char*)start;
    }

    template <class T>
    inline void set_type(T* v)
    {
        addr = (uint64_t)v;
        size = sizeof(T);
    }

    template <class T>
    inline T* type()
    {
        assert(size == sizeof(T));
        return (T*)addr;
    }
};


struct rpc_id_t {
    uint8_t rpc_type;
    uint8_t my_nodeid;
    uint8_t server_nodeid;
    uint8_t thread_id;
    volatile uint32_t rpc_id;
    rpc_id_t() {};
    rpc_id_t(uint8_t _rpc_type, uint8_t _my_nodeid, uint8_t _server_nodeid, uint8_t _thread_id, uint32_t _rpc_id)
        : rpc_type(_rpc_type)
        , my_nodeid(_my_nodeid)
        , server_nodeid(_server_nodeid)
        , thread_id(_thread_id)
        , rpc_id(_rpc_id)
    {
    }

} __attribute__((packed));


struct batch_sort_t {
    char *control_addr;
    int p;
    int id;
};

inline bool cmp_batch(batch_sort_t wc_1, batch_sort_t wc_2){
        if (wc_2.p == wc_1.p)
            return wc_1.id < wc_2.id;
        return wc_1.p < wc_2.p;
}


inline uint64_t mk_id(uint32_t remoteNodeID, uint32_t thread_id)
{
    return remoteNodeID;
}
inline uint64_t get_id(uint64_t wcid)
{
    return wcid;
}

struct ReqHead {
    rpc_id_t rpcId;
    // uint64_t reserveCode;
    uint16_t read_name; // read type
    uint8_t req_num;
    uint8_t read_num;
    uint8_t write_num;
    uint8_t TXID;
#ifdef NICNICNIC
    uint16_t fpga_padding;
    volatile uint32_t size;
    volatile uint32_t magic;
    volatile uint32_t partition_hint;
#endif
    void print_head()
    {
#ifdef NICNICNIC
        printf("read_name=%d, req_num=%d, read_num=%d, write_num=%d TXID=%d size=%d\n",
            read_name, req_num, read_num, write_num, TXID, size);
#endif
    }
} __attribute__((packed));




struct alignas(64) Connect {

    // meta data
    uint64_t localBase; //
    uint64_t remoteAddr;
    uint32_t remoteRkey; // for local base

    // reply
    uint64_t reply_addr;
    uint64_t remote_reply_addr; // for client in NICTX
    uint32_t remote_reply_key;

    // data
    uint64_t data_addr; // for server in NICTX
    uint64_t remote_data_addr;
    uint32_t remote_data_key;

    ibv_qp* qp;

    uint32_t remoteQPNum;
    uint16_t remoteNodeID;

    uint16_t remoteLid;
    uint8_t remoteGid[16];

    uint64_t recnum;
    volatile bool online;
    uint64_t curAddr;
    uint8_t lock;
    uint64_t sendnum;
    uint64_t signal_num;
    uint64_t tail;
    uint64_t remote_tail;
    uint64_t poolSize;
    uint32_t thread_id;
    struct ibv_recv_wr recv_wr[BatchPostRec];
    struct ibv_sge sgl[BatchPostRec];

    

    Connect()
    {
        curAddr = 0;
        recnum = 0;
        sendnum = 0;
        signal_num = 0;
        online = false;
    }
    void fill_wr(uint32_t lkey)
    {
        uint64_t addr = localBase + poolSize;
        for (size_t i = 0; i < BatchPostRec; ++i) {
            sgl[i].addr = addr;
            sgl[i].length = 0;
            sgl[i].lkey = lkey;
            recv_wr[i].wr_id = mk_id(remoteNodeID, thread_id);
            recv_wr[i].sg_list = &sgl[i];
            recv_wr[i].num_sge = 1;
            recv_wr[i].next = (i == BatchPostRec - 1) ? NULL : &recv_wr[i + 1];
        }
    }
    inline bool getsendslot(uint64_t msize, uint64_t* addr)
    {
        // SHIT
        if (curAddr + msize > poolSize) {
            curAddr = 0;
        }
        // tail means nothing
        if (0 && (tail + poolSize - curAddr) % poolSize < msize) {
            Debug::notifyError("???");
        }

        (*addr) = (curAddr % poolSize) + localBase;
        curAddr += msize;
        return true;
    }
    
    // for nictxn server
    inline uint64_t get_rev_addr(uint16_t id)
    {
        return data_addr + id * DATA_LEN;
    }
    inline uint64_t get_send_addr(uint16_t id)
    {
        return reply_addr + id * REPLY_LEN;
    }
    
    // for nictxn client
    inline uint64_t get_my_reply_addr(uint16_t id) volatile
    {
        return reply_addr + id * REPLY_LEN;
    }

    inline uint64_t get_strife_reply_addr(uint32_t rpc_id) volatile
    {
        // no affect on the original reply support (SLOT_NUM)
        return reply_addr + REPLY_LEN * SLOT_NUM + (rpc_id) * 8;
    }

    inline uint32_t get_strife_reply_size(){
        return sizeof(uint64_t);
    }
    
    inline uint64_t getRemoteAddr(uint64_t localAddr)
    {
        return ((uint64_t)localAddr - localBase + remoteAddr);
    }
    inline uint64_t get_read_remote_addr(uint64_t local_reply_addr)
    {
        return ((uint64_t)local_reply_addr - reply_addr + remote_reply_addr);
    }

    

#ifdef DOUBLERDMA
    inline uint64_t serialization(uint16_t id, uint64_t* size, uint64_t feature_addr)
    {
        // return (localBase - 2);
        uint64_t addr = localBase + id * SLOT_LEN;
        uint64_t data_addr_i = data_addr + id * DATA_LEN;
        ReqHead* reqHead = (ReqHead*)(data_addr_i);
        *size = 4 + 64;
        unsigned short* metadata = (unsigned short*)addr; // 2 byte
        // 小端->大端
        
        if (feature_addr != 0)
            memcpy((char*)addr, (char*)feature_addr, 64);
        // memset((void *)addr, 0 ,64);
        metadata[33] = ~metadata[33]; 
        metadata[32] = reqHead->partition_hint;
        // memset((char *)metadata, 0, 64);
        //     if (reqHead->partition_hint>16){
        //         metadata[0] = 0;
        //         metadata[1] = (1ULL << (reqHead->partition_hint-16));
        //         // printf("%d %d\n",metadata[0], metadata[1]);
        //     }
        //     else {
        //         metadata[1] = 0;
        //         metadata[0] = 1ULL<<reqHead->partition_hint;
        //     }

        // reqHead->partition_hint = 0;
        return addr;
    }

    inline uint64_t get_send_data_addr(uint16_t id)
    {
        return data_addr + id * DATA_LEN;
    }
    inline uint64_t get_remote_data_addr(uint64_t local_data_addr)
    {
        // assert(localAddr - localBase <= poolSize);
        return local_data_addr - data_addr + remote_data_addr; // host
    }
#endif

    
};
#define NORMALREAD 66
#define TPCCREAD 55
#define COMMITRPC 99

struct ReplyHead {
    rpc_id_t rpcId;
    uint8_t TXID;
    SIZETYPE replySize;
} __attribute__((packed));


#define barrier() asm volatile("mfence" \
                               :        \
                               :        \
                               : "memory")

// < uint32_t
enum class RPCTYPE {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    READ_CALL = 64,
    WRITE_READ_TX,
    READONLY_TX,
    JUST_COMMIT,
    RPC_MESSAGE,

    QSTORE_META_Req,
    QSTORE_META_Reply,
    QSTORE_QUEUE_Req,
};




enum class TxControlType {

    LOCK_REQ, // 0 [L]
    LOCK_FAIL, // 1 [H]
    VALIDATE_REQ, // 2 [M]
    VALIDATE_FAIL, // 3 [H]
    COMMIT_REQ, // 4 [H]
    COMMIT_FAIL, // 5 [H]
    COMMIT_OK, // 6 [H]
    INIT_SLOT, // 7
    NORMAL_MESSAGE, // 8 [L]
    LOCK_OK, // 9
    REPLICATION_REQ, // 10
    REPLICATION_OK, // 11
    SHIT, //12
};
enum class FuncType {
    NORMAL_SINGLE_FUNC,
    NORMAL_MULTI_FUNC,
    NEST_SINGLE_FUNC,
    NEST_MULTI_FUNC,
};

enum class ErisKeyType {
    CHECK_VERSION = 1,
    READ_AND_READ_LOCK,
    RET_VALUE,
    WRITE_LOCK,
    WRITE_VALUE,
    FREE_READ_LOCK,
    FREE_WRITE_LOCK,
    WRITE_OK,
    WRITE_LOCK_OK,
    READ_LOCL_OK,
};





struct replyPair {

    using PostFunction = void (*)(const replyPair& reply, void* ctx);

    uint64_t addr;
    SIZETYPE size;
    bool locality;

    PostFunction post_function;
    void* ctx;

    replyPair(void* _addr, SIZETYPE _size)
    {
        addr = (uint64_t)_addr;
        size = _size;
        post_function = nullptr;
        locality = 0;
    }

    replyPair()
    {
        post_function = nullptr;
        locality = 0;
    }

    inline void set(const void* _addr, SIZETYPE _size)
    {
        addr = (uint64_t)_addr;
        size = _size;
    }

    template <class T>
    inline void set_type(T* v)
    { // caller
        addr = (uint64_t)v;
        size = 0;
    }

    template <class T>
    inline T* type() const
    {
        // assert(size == sizeof(T));
        return (T*)addr;
    }

    inline bool is_null() const { return size == 0; }

    inline static replyPair Void() { return replyPair(nullptr, 0); }

    inline static replyPair VoidWithPostFunc(PostFunction func, void* ctx)
    {
        auto ret = replyPair(nullptr, 0);
        ret.post_function = func;
        ret.ctx = ctx;

        return ret;
    }
};



using CallbackFunc = void (*)(const replyPair&, void*);

struct WriteStruct {
    uint64_t addr;
    uint32_t size;
};

replyPair rpc_persist(reqPair);
replyPair rpc_nop(reqPair);
replyPair rpc_copy_persist(reqPair*, uint16_t);

struct ExchangeMeta {

    uint32_t qpNum;
    uint16_t lid;
    uint16_t NodeID;
    uint8_t gid[16];

    uint64_t rAddr; // write mm & write_imm mm
    uint32_t rkey;

    uint64_t reply_rAddr;
    uint32_t reply_rkey;
    
    uint64_t data_rAddr;
    uint32_t data_rkey;
    uint64_t interface_id;

} __attribute__((packed));

// client:nic+worker



// Send | Recv

#endif