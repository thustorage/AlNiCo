/*
 * @Author: your name
 * @Date: 2021-05-18 15:29:47
 * @LastEditTime: 2022-04-09 16:44:08
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /TxSys/include/control/ControlConnection.h
 */
#ifndef __CONTROL_CONNECTION_H__
#define __CONTROL_CONNECTION_H__
#include "SwitchTxCommon.h"

#define DST_MAC (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x98 // 113
#define SRC_MAC (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00
#define ETH_TYPE (char)0x08, (char)0x00 // NICE

#ifdef USEERIS

#define IP_HDRS                                                                \
  (char)0x45, (char)0x00, (char)((32 + sizeof(ErisTXMessage))>>8), (char)((32 + sizeof(ErisTXMessage))&0x00FF), (char)0x00, (char)0x00, (char)0x40, (char)0x00, (char)0x40, (char)0x11, (char)0xaf, (char)0xb6 //NICE (char)0x11= udp afb6 checksum
#define UDP_OTHER (char)((8 + sizeof(ErisTXMessage))>>8), (char)((8 + sizeof(ErisTXMessage))&0x00FF), (char)0x00, (char)0x00 // NICE

#else
#define IP_HDRS                                                                \
  (char)0x45, (char)0x00, (char)0x00, (char)(32 + sizeof(SwitchTXMessage)), (char)0x00, (char)0x00, (char)0x40, (char)0x00, (char)0x40, (char)0x11, (char)0xaf, (char)0xb6 //NICE (char)0x11= udp afb6 checksum
#define UDP_OTHER (char)0x00, (char)(8 + sizeof(SwitchTXMessage)), (char)0x00, (char)0x00 // NICE
#endif


#define SRC_IP (char)0x0d, (char)0x07, (char)0x38, (char)0x66
#define DST_IP (char)0x0d, (char)0x07, (char)0x38, (char)0x7f
#define SRC_PORT (char)0x02, (char)0x9A
#define DEST_PORT (char)0x00, (char)0x00


class  Control_Connection
{
    const static int kBatchCount = 2;
private:
    ibv_qp *message; // ud or raw packet
    uint16_t messageNR;

    ibv_mr *messageMR;
    void *messagePool;
    uint32_t messageLkey;

    uint16_t curMessage;

    void *sendPool;
    uint16_t curSend;

    ibv_recv_wr *recvs[kBatchCount];
    ibv_sge *recv_sgl[kBatchCount];
    uint32_t subNR;

    ibv_cq *send_cq;
    uint64_t sendCounter;

    uint16_t sendPadding; // ud: 0
                          // rp: ?
    uint16_t recvPadding; // ud: 40
                          // rp: ?
public:
    Control_Connection(RdmaContext &ctx, ibv_cq *cq, uint32_t messageNR,
                         const uint8_t mac[6], int myport);
    void initRecv();
    void initSend();
    char *getMessage();
    char *getSendPool();
    inline void fixport(SwitchTXMessage *m, uint16_t port);
    inline void fixport_eris(ErisTXMessage  *m, uint16_t port);
    void sendTXMessage_signal(SwitchTXMessage* m, uint16_t destport); 
    void sendTXMessage(SwitchTXMessage *m, uint16_t destport = 0);

    //inline void fixport_eris(ErisTXMessage *m,uint16_t port);
    void sendTXMessage_eris(ErisTXMessage *m, uint16_t destport = 0);
};


#endif