#include "ControlConnection.h"
#include "Debug.h"
#define LOCALBUF 0
Control_Connection::Control_Connection(RdmaContext& ctx, ibv_cq* cq,
    uint32_t messageNR, const uint8_t mac[6], int myport)
    : messageNR(messageNR)
    , curMessage(0)
    , curSend(0)
    , sendCounter(0)
{
    sendPadding = 42;
    recvPadding = 42;
    ibv_qp_type type = IBV_QPT_RAW_PACKET;

    assert(messageNR % kBatchCount == 0);

    send_cq = ibv_create_cq(ctx.ctx, messageNR, NULL, NULL, 0);
    createQueuePair(&message, type, send_cq, cq, &ctx, messageNR);
    modifyUDtoRTS(message, &ctx);

    messagePool = hugePageAlloc((2 + LOCALBUF) * messageNR * MESSAGE_SIZE);
    // messagePool = malloc((2 + LOCALBUF) * messageNR * MESSAGE_SIZE);
    messageMR = createMemoryRegion((uint64_t)messagePool,
        (2 + LOCALBUF) * messageNR * MESSAGE_SIZE, &ctx);
    sendPool = (char*)messagePool + messageNR * MESSAGE_SIZE;
    messageLkey = messageMR->lkey;
    assert(sizeof(SwitchTXMessage) + sendPadding <= MESSAGE_SIZE);
#ifdef USEERIS
    assert(sizeof(ErisTXMessage) + sendPadding <= MESSAGE_SIZE);
#ifndef ERIS_BOX 
    for (uint16_t i = 0; i < 256; i++) {
        if ((i & bitmap_server[myport]) == bitmap_server[myport])
            steeringWithMacUdp_eris(message, &ctx, mac,
                toBigEndian16(i), toBigEndian16(666));
    }
    return;
#endif
    printf("size = %d\n",sizeof(ErisTXMessage) + sendPadding);
#endif

    steeringWithMacUdp(message, &ctx, mac, toBigEndian16(myport), toBigEndian16(666));
    // steeringWithMacUdp_all(message, &ctx);
    return;
}
/**
 * post_recv
 */
void Control_Connection::initRecv()
{
    subNR = messageNR / kBatchCount;

    for (int i = 0; i < kBatchCount; ++i) {
        recvs[i] = new ibv_recv_wr[subNR];
        recv_sgl[i] = new ibv_sge[subNR];
    }

    for (int k = 0; k < kBatchCount; ++k) {
        for (size_t i = 0; i < subNR; ++i) {
            auto& s = recv_sgl[k][i];
            memset(&s, 0, sizeof(s));
            s.addr = (uint64_t)messagePool + (k * subNR + i) * MESSAGE_SIZE;
            s.length = MESSAGE_SIZE;
            s.lkey = messageLkey;

            auto& r = recvs[k][i];
            memset(&r, 0, sizeof(r));
            r.wr_id = RAW_PACKET_WR_ID;
            r.sg_list = &s;
            r.num_sge = 1;
            r.next = (i == subNR - 1) ? NULL : &recvs[k][i + 1];
        }
    }

    struct ibv_recv_wr* bad;
    for (int i = 0; i < kBatchCount; ++i) {
        if (ibv_post_recv(message, &recvs[i][0], &bad)) {
            // Debug::notifyError("Receive failed.");
        }
    }
}
/**
 * pollCQ over, read the message 
 */
char* Control_Connection::getMessage()
{
    struct ibv_recv_wr* bad;
    char* m = (char*)messagePool + curMessage * MESSAGE_SIZE + recvPadding;

    ADD_ROUND(curMessage, messageNR);

    if (curMessage % subNR == 0) {
        if (ibv_post_recv(
                message,
                &recvs[(curMessage / subNR - 1 + kBatchCount) % kBatchCount][0],
                &bad)) {
            // Debug::notifyError("Receive failed.");
        }
    }

    return m;
}
/**
 * get the message poll 
 */
char* Control_Connection::getSendPool()
{
    char* s = (char*)sendPool + curSend * MESSAGE_SIZE + sendPadding;

    ADD_ROUND(curSend, messageNR * (LOCALBUF + 1));

    return s;
}

/**
 * copy the head to message pool
 */

void Control_Connection::initSend()
{
    char header[] = { DST_MAC, SRC_MAC, ETH_TYPE, IP_HDRS, SRC_IP,
       DST_IP, SRC_PORT, DEST_PORT, UDP_OTHER };
    assert(sizeof(header) == sendPadding);

    for (int i = 0; i < messageNR * (LOCALBUF + 1); ++i) {
        memcpy((char*)sendPool + i * MESSAGE_SIZE, header, sendPadding);
    }
}

inline void Control_Connection::fixport(SwitchTXMessage* m, uint16_t port)
{
    uint16_t* destport = (uint16_t*)((uint64_t)m - 6);
    *destport = toBigEndian16(port);
}
inline void Control_Connection::fixport_eris(ErisTXMessage* m, uint16_t port)
{
    uint16_t* destport = (uint16_t*)((uint64_t)m - 6);
    *destport = toBigEndian16(port);
}
void Control_Connection::sendTXMessage(SwitchTXMessage* m, uint16_t destport)
{

    if ((sendCounter & SIGNAL_BATCH) == 0 && sendCounter > 0) {
        ibv_wc wc;
        pollWithCQ(send_cq, 1, &wc);
    }

#ifdef SWITCHTX_BOX
    m->box_d_n_id = KEY2NODE_MOD;
#endif

    fixport(m, destport);
    // memcpy((char *)sendPool + i * MESSAGE_SIZE, header, sendPadding);
    rdmaRawSend(message, (uint64_t)m - sendPadding,
        sizeof(SwitchTXMessage) + sendPadding, messageLkey,
        (sendCounter & SIGNAL_BATCH) == 0);
    ++sendCounter;
}

void Control_Connection::sendTXMessage_signal(SwitchTXMessage* m, uint16_t destport)
{
    
    if ((sendCounter & SIGNAL_BATCH) == 0 && sendCounter > 0) {
        ibv_wc wc;
        pollWithCQ(send_cq, 1, &wc);
    }

#ifdef SWITCHTX_BOX
    m->box_d_n_id = KEY2NODE_MOD;
#endif

    fixport(m, destport);
    // memcpy((char *)sendPool + i * MESSAGE_SIZE, header, sendPadding);
    rdmaRawSend(message, (uint64_t)m - sendPadding,
        sizeof(SwitchTXMessage) + sendPadding, messageLkey,
        true);
    if ((sendCounter & SIGNAL_BATCH) != 0 && sendCounter > 0){
        ibv_wc wc[2];
        pollWithCQ(send_cq, 2, wc);
    }
    else {
        ibv_wc wc;
        pollWithCQ(send_cq, 1, &wc);
    }
    sendCounter = 0;
}
void Control_Connection::sendTXMessage_eris(ErisTXMessage* m, uint16_t destport)
{

    if ((sendCounter & SIGNAL_BATCH) == 0 && sendCounter > 0) {
        ibv_wc wc;
        pollWithCQ(send_cq, 1, &wc);
    }
    init_bitmap_eris(m);
#ifdef ERIS_BOX
    if (m->is_mc == 0){
        fixport_eris(m,m->c_t_id);
    }
    else{
        fixport_eris(m,m->s_t_id);
    }
#endif
    // printf("is_mc=%d i send port = %d\n", m->is_mc ,*(uint16_t*)((uint64_t)m - 6));
    // fixport_eris(m, destport);
    // memcpy((char *)sendPool + i * MESSAGE_SIZE, header, sendPadding);
    rdmaRawSend(message, (uint64_t)m - sendPadding,
        sizeof(ErisTXMessage) + sendPadding, messageLkey,
        (sendCounter & SIGNAL_BATCH) == 0);

    ++sendCounter;
}