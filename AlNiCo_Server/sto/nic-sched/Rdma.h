#ifndef _RDMA_H__
#define _RDMA_H__

#define forceinline inline __attribute__((always_inline))

#include <assert.h>
#include <cstring>
#include <infiniband/verbs.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <list>
#include <string>

#include "Debug.h"



#define BatchPostRec 24ULL

#define BatchPollSend 24ULL
#define BatchSignalSend 64ULL

#define MAX_POST_LIST 32
#define DCT_ACCESS_KEY 3185
#define UD_PKEY 0x11111111
#define PSN 3185

struct RdmaContext {
    uint8_t devIndex;
    uint8_t port;
    int gidIndex;
    ibv_wc wc[2 * BatchSignalSend];
    ibv_context* ctx;
    ibv_pd* pd;

    uint16_t lid;
    uint64_t signalNum;
    // uint16_t signal_flag[MAX_SERVER];
    union ibv_gid gid;

    RdmaContext()
        : ctx(NULL)
        , pd(NULL)
    {
        // memset(signal_flag, 0, sizeof(signal_flag));
        signalNum = 0;
    }

    ibv_cq* send_cq;
};

struct Region {
    uint64_t source;
    uint32_t size;

    uint64_t dest;
};
static inline void fillSgeWr(ibv_sge& sg, ibv_send_wr& wr, uint64_t source,
    uint64_t size, uint32_t lkey)
{
    memset(&sg, 0, sizeof(sg));
    sg.addr = (uintptr_t)source;
    sg.length = size;
    sg.lkey = lkey;

    memset(&wr, 0, sizeof(wr));
    wr.wr_id = 0;
    wr.sg_list = &sg;
    wr.num_sge = 1;
}

static inline void fillSgeWr(ibv_sge& sg, ibv_recv_wr& wr, uint64_t source,
    uint64_t size, uint32_t lkey)
{
    memset(&sg, 0, sizeof(sg));
    sg.addr = (uintptr_t)source;
    sg.length = size;
    sg.lkey = lkey;

    memset(&wr, 0, sizeof(wr));
    wr.wr_id = 0;
    wr.sg_list = &sg;
    wr.num_sge = 1;
}

static inline void fillSgeWr(ibv_sge& sg, ibv_exp_send_wr& wr, uint64_t source,
    uint64_t size, uint32_t lkey)
{
    memset(&sg, 0, sizeof(sg));
    sg.addr = (uintptr_t)source;
    sg.length = size;
    sg.lkey = lkey;

    memset(&wr, 0, sizeof(wr));
    wr.wr_id = 0;
    wr.sg_list = &sg;
    wr.num_sge = 1;
}
//// Resource.cpp
bool createContext(RdmaContext* context, uint8_t port = 1, int gidIndex = 3, uint8_t devIndex = -1);
bool destoryContext(RdmaContext* context);

ibv_mr* createMemoryRegion(uint64_t mm, uint64_t mmSize, RdmaContext* ctx);

bool createQueuePair(ibv_qp** qp, ibv_qp_type mode, ibv_cq* cq,
    RdmaContext* context, uint32_t qpsMaxDepth = 128,
    uint32_t maxInlineData = 0);

bool createQueuePair(ibv_qp** qp, ibv_qp_type mode, ibv_cq* send_cq,
    ibv_cq* recv_cq, RdmaContext* context,
    uint32_t qpsMaxDepth = 128, uint32_t maxInlineData = 0);

bool createDCTarget(ibv_exp_dct** dct, ibv_cq* cq, RdmaContext* context,
    uint32_t qpsMaxDepth = 128, uint32_t maxInlineData = 0);
void fillAhAttr(ibv_ah_attr* attr, uint32_t remoteLid, uint8_t* remoteGid,
    RdmaContext* context);

//// StateTrans.cpp
bool modifyQPtoInit(struct ibv_qp* qp, RdmaContext* context);
bool modifyQPtoRTR(struct ibv_qp* qp, uint32_t remoteQPN, uint16_t remoteLid,
    uint8_t* gid, RdmaContext* context);
bool modifyQPtoRTS(struct ibv_qp* qp);

bool modifyUDtoRTS(struct ibv_qp* qp, RdmaContext* context);
bool modifyDCtoRTS(struct ibv_qp* qp, uint16_t remoteLid, uint8_t* remoteGid,
    RdmaContext* context);

//// Operation.cpp
int pollWithCQ(ibv_cq* cq, int pollNumber, struct ibv_wc* wc);
int pollOnce(ibv_cq* cq, int pollNumber, struct ibv_wc* wc);
int pollWithCQN(ibv_cq* cq, int pollNumber, struct ibv_wc* wc);
int pollWithCQ5(ibv_cq* cq, int pollNumber, struct ibv_wc* wc);
bool rdmaSend(ibv_qp* qp, uint64_t source, uint64_t size, uint32_t lkey,
    ibv_ah* ah, uint32_t remoteQPN, int32_t imm = -1,
    bool isSignaled = false);

bool rdmaSend(ibv_qp* qp, uint64_t source, uint64_t size, uint32_t lkey,
    int32_t imm = -1);
bool rdmaRawSend(ibv_qp* qp, uint64_t source, uint64_t size, uint32_t lkey,
    bool isSignaled);

bool rdmaReceive(ibv_qp* qp, uint64_t source, uint64_t size, uint32_t lkey, uint64_t wr_id = 0);
bool rdmaReceive(ibv_srq* srq, uint64_t source, uint64_t size, uint32_t lkey);
bool rdmaReceive(ibv_exp_dct* dct, uint64_t source, uint64_t size,
    uint32_t lkey);

bool rdmaRead(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t size,
    uint32_t lkey, uint32_t remoteRKey, uint64_t wrID = 0);
bool rdmaRead(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t size,
    uint32_t lkey, uint32_t remoteRKey, ibv_ah* ah,
    uint32_t remoteDctNumber);



bool rdmaWrite(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t size,
    uint32_t lkey, uint32_t remoteRKey, int32_t imm = -1,
    bool isSignaled = true, uint64_t wrID = 0);

bool rdmaWrite(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t size,
    uint32_t lkey, uint32_t remoteRKey, ibv_ah* ah,
    uint32_t remoteDctNumber, int32_t imm);

bool rdmaFetchAndAdd(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t add,
    uint32_t lkey, uint32_t remoteRKey);
bool rdmaFetchAndAdd(ibv_qp* qp, uint64_t source, uint64_t dest, uint64_t add,
    uint32_t lkey, uint32_t remoteRKey, ibv_ah* ah,
    uint32_t remoteDctNumber);

bool rdmaCompareAndSwap(ibv_qp* qp, uint64_t source, uint64_t dest,
    uint64_t compare, uint64_t swap, uint32_t lkey,
    uint32_t remoteRKey);
bool rdmaCompareAndSwap(ibv_qp* qp, uint64_t source, uint64_t dest,
    uint64_t compare, uint64_t swap, uint32_t lkey,
    uint32_t remoteRKey, ibv_ah* ah,
    uint32_t remoteDctNumber);

//// Batch.cpp
bool rdmaBatchSend(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter,
    bool isInline = false, int32_t imm = -1);
bool rdmaBatchSend(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter, ibv_ah* ah,
    uint32_t remoteQPN, bool isInline = false, int32_t imm = -1);

bool rdmaBatchReceive(ibv_qp* qp, const std::list<Region>& regions,
    uint32_t lkey);
bool rdmaBatchReceive(ibv_srq* srq, const std::list<Region>& regions,
    uint32_t lkey);
bool rdmaBatchReceive(ibv_exp_dct* dct, const std::list<Region>& regions,
    uint32_t lkey);

bool rdmaBatchRead(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter, uint32_t remoteRKey,
    bool isInline = false);
bool rdmaBatchRead(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter, uint32_t remoteRKey,
    ibv_ah* ah, uint32_t remoteDctNumber, bool isInline = false);

bool rdmaBatchWrite(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter,
    uint32_t remoteRKey, bool isInline = false,
    int32_t imm = -1);
bool rdmaBatchWrite(ibv_qp* qp, const std::list<Region>& regions, uint32_t lkey,
    uint32_t signalBatch, uint64_t& counter,
    uint32_t remoteRKey, ibv_ah* ah, uint32_t remoteDctNumber,
    bool isInline = false, int32_t imm = -1);

//// Utility.cpp
void rdmaQueryQueuePair(ibv_qp* qp);
void checkDctSupported(struct ibv_context* ctx);

//// FlowSteering.cpp
void steeringWithMacUdp(ibv_qp* qp, RdmaContext* ctx, const uint8_t mac[6],
    uint16_t dstPort, uint16_t srcPort);
void steeringWithMacUdp_eris(ibv_qp* qp, RdmaContext* ctx, const uint8_t mac[6],
    uint16_t dstPort, uint16_t srcPort);

#endif
