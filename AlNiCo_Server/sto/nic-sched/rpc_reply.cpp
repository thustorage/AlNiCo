/*
 * @Date: 2020-09-21 09:44:14
 * @LastEditTime: 2022-01-12 12:59:41
 * @FilePath: /TxSys/src/control/rpc_init.cpp
 * @Author: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "PlatformFeatures.hh"
#include "Rpc.h"

struct ibv_sge sg[MAX_CLIENT * CLIENT_THREAD][SLOT_NUM];
struct ibv_send_wr wr[MAX_CLIENT * CLIENT_THREAD][SLOT_NUM];
uint32_t last_rpc_id[MAX_CLIENT * CLIENT_THREAD][SLOT_NUM];

struct ibv_sge sg_strife[MAX_CLIENT * CLIENT_THREAD];
struct ibv_send_wr wr_strife[MAX_CLIENT * CLIENT_THREAD];
volatile uint64_t strife_phase[MAX_CORE][8];
volatile uint64_t tpcc_reply_delivery[MAX_CLIENT][CLIENT_THREAD];
volatile uint64_t now_phase;

void init_strife_queue(){
    printf("size = %lld\n",sizeof(strife_queue_t));
    strife_dispatch_queue = (volatile strife_queue_t *)malloc(sizeof(strife_queue_t) * (MAX_THREAD+2));
    for (int core_i = 0; core_i < MAX_THREAD + 2; core_i++)
    {
        strife_dispatch_queue[core_i].init();
    }
}

void init_tpcc_delivery(){
    for (int i = 0; i < MAX_CLIENT; i++){
        for (int j = 0; j < CLIENT_THREAD; j++){
            tpcc_reply_delivery[i][j] = 0;
        }
    }
}

void reply_write()
{
    bindCore(23);
    // printf("last_rpc_id = %d\n", last_rpc_id[0][0]);
    memset(last_rpc_id, 0, sizeof(last_rpc_id));

    for (int i = 0; i < MAX_CLIENT * CLIENT_THREAD; i++) {
        for (int j = 0; j < SLOT_NUM; j++) {
            auto tmp_c = server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD];
            fillSgeWr(sg[i][j], wr[i][j],
                tmp_c->get_send_addr(j),
                sizeof(rpc_id_t), tmp_c->local_reply_key);
            wr[i][j].opcode = IBV_WR_RDMA_WRITE;
            wr[i][j].wr_id = i;
            wr[i][j].wr.rdma.remote_addr = tmp_c->get_remote_reply_addr_by_id(j);
            wr[i][j].wr.rdma.rkey = tmp_c->remote_reply_key;
        }
    }

// #ifdef STRIFE
    init_strife();
// #endif

    RdmaContext* context_ptr;
    while (true) {
        context_ptr = &(server->context);
        for (int i = 0; i < MAX_CLIENT * CLIENT_THREAD; i++) {
            if (i == MAX_CLIENT * CLIENT_THREAD / 2) {
                context_ptr = &(server->context_dual);
            }
            for (int j = 0; j < SLOT_NUM; j++) {
                uint64_t addr = server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD]->get_send_addr(j);

                if ((*(volatile rpc_id_t*)addr).rpc_id != last_rpc_id[i][j]) {
                    
                    bool inline_flag = true;
                    #ifdef YCSBplusT
                        int num = (*(volatile rpc_id_t*)addr).rpc_type;
                        // printf("send %d %d\n",sizeof(rpc_id_t) + num * 384, REPLY_LEN);
                        wr[i][j].sg_list[0].length = sizeof(rpc_id_t) + num * 384;
                        inline_flag = false;
                    #endif
                    #ifdef NULLRPC
                        inline_flag = true;
                    #endif


                    last_rpc_id[i][j] = (*(volatile rpc_id_t*)addr).rpc_id;
                    server->rdma_write_reply_nictx(context_ptr,
                        server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD],
                        &wr[i][j], inline_flag);

                    // printf("reply %d\n",last_rpc_id[i][j]);
                }
            }
        }
#if (CPUBASED==3)
    phase_sync_strife();
#else

#ifndef YCSBplusT
    tpcc_reply();
#endif

#endif

    }
}

void tpcc_reply(){
    RdmaContext* context_ptr = &(server->context);
    for (int i = 0; i < MAX_CLIENT * CLIENT_THREAD; i++) {
        if (i == MAX_CLIENT * CLIENT_THREAD / 2) {
            context_ptr = &(server->context_dual);
        }
        if (tpcc_reply_delivery[i / CLIENT_THREAD][i % CLIENT_THREAD] == 1){
            tpcc_reply_delivery[i / CLIENT_THREAD][i % CLIENT_THREAD] = 0;
            server->rdma_write_reply_nictx(context_ptr,
                        server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD],
                        &wr_strife[i], false);
            
        }
    }
}

void phase_sync_strife()
{
    bool flag = true;
    #ifndef YCSBplusT
    for (int i = 0; i < MAX_THREAD + 2; i++) {
        if (strife_phase[i][0] != now_phase) {
            flag = false;
        }
    }
    #else
    for (int i = 0; i < MAX_THREAD; i++) {
        if (strife_phase[i][0] != now_phase) {
            flag = false;
        }
    }
    #endif 
    if (!flag) {
        // printf("phase = %d\n",);
        return;
    }
   
    switch (now_phase) {
    case 0: {
        // puts("??");
        reply_write_strife();
        // check time and size;
        now_phase = 1;
        break;
    }
    case 1: {
        if (strife_batch.check_batch_size(10000,5))
            now_phase = 2;
        break;
    }
    case 2: {
        // printf("phase=%d\n",now_phase);
        now_phase = 0;
        break;
    }
    }
}

void reply_write_strife()
{
    RdmaContext* context_ptr = &(server->context);
    for (int i = 0; i < MAX_CLIENT * CLIENT_THREAD; i++) {
        if (i == MAX_CLIENT * CLIENT_THREAD / 2) {
            context_ptr = &(server->context_dual);
        }
        server->rdma_write_reply_nictx(context_ptr,
                    server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD],
                    &wr_strife[i], false);
    }
}

void init_strife_phase(){
    now_phase = 0;
    for (int i = 0; i < MAX_THREAD + 2; i++){
        strife_phase[i][0] = 0;
    }
}

void init_strife()
{
    for (int i = 0; i < MAX_CLIENT * CLIENT_THREAD; i++) {
        auto tmp_c = server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD];
        fillSgeWr(sg_strife[i], wr_strife[i],
            tmp_c->get_strife_send_addr(0),
            tmp_c->get_strife_reply_size(), tmp_c->local_reply_key);
        wr_strife[i].opcode = IBV_WR_RDMA_WRITE;
        wr_strife[i].wr_id = i;
        wr_strife[i].wr.rdma.remote_addr = tmp_c->get_remote_reply_addr_by_id_strife(0);
        wr_strife[i].wr.rdma.rkey = tmp_c->remote_reply_key;
    }
    return;
}

// void reply_write_dual(int part)
// {
//     bindCore(22 + part);
//     int part_size = MAX_CLIENT * CLIENT_THREAD / 2;
//     for (int i = part * part_size; i < (part + 1) * part_size; i++) {
//         for (int j = 0; j < SLOT_NUM; j++)
//             last_rpc_id[i][j] = 0;
//     }

//     for (int i = part * part_size; i < (part + 1) * part_size; i++) {
//         for (int j = 0; j < SLOT_NUM; j++) {
//             auto tmp_c = server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD];
//             fillSgeWr(sg[i][j], wr[i][j],
//                 tmp_c->get_send_addr(j),
//                 sizeof(rpc_id_t), tmp_c->local_reply_key);
//             wr[i][j].opcode = IBV_WR_RDMA_WRITE;
//             wr[i][j].wr_id = i;
//             wr[i][j].wr.rdma.remote_addr = tmp_c->get_remote_reply_addr_by_id(j);
//             wr[i][j].wr.rdma.rkey = tmp_c->remote_reply_key;
//         }
//     }
//     RdmaContext* context_ptr;
//     if (part == 0)
//         context_ptr = &(server->context);
//     else
//         context_ptr = &(server->context_dual);

//     while (true) {
//         for (int i = part * part_size; i < (part + 1) * part_size; i++) {
//             for (int j = 0; j < SLOT_NUM; j++) {
//                 uint64_t addr = server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD]->get_send_addr(j);

//                 if ((*(volatile rpc_id_t*)addr).rpc_id != last_rpc_id[i][j]) {

//                     server->rdma_write_reply_nictx(context_ptr,
//                         server->nic_connect[i / CLIENT_THREAD + KEY2NODE_MOD][i % CLIENT_THREAD],
//                         &wr[i][j]);

//                     last_rpc_id[i][j] = (*(volatile rpc_id_t*)addr).rpc_id;
//                 }
//             }
//         }
//     }
// }

bool Rpc::poll_signal(RdmaContext* context_ptr, Connect* connect, bool pollflag, bool sync_op)
{
    if (pollflag || sync_op || (context_ptr->signalNum >= 2 * BatchSignalSend)) {

        int poll_num = min(BatchSignalSend, context_ptr->signalNum + 1 / 2);

        int tmp = pollWithCQN(context_ptr->send_cq, poll_num, context_ptr->wc);
        if (tmp == -1)
            goto POLLCQ_FAIL;

        do {
            // puts("?????????");
            for (int i = 0; i < poll_num; i++) {
                auto& wc = context_ptr->wc[i];
                if (wc.wr_id >= MAX_CLIENT * CLIENT_THREAD) {
                    int c_i = (wc.wr_id - MAX_CLIENT * CLIENT_THREAD) / CLIENT_THREAD;
                    int c_j = (wc.wr_id - MAX_CLIENT * CLIENT_THREAD) % CLIENT_THREAD;

                    nic_connect[c_i + KEY2NODE_MOD][c_j]->sendnum--;
                } else {
                    int c_i = wc.wr_id / CLIENT_THREAD;
                    int c_j = wc.wr_id % CLIENT_THREAD;
                    // printf("ok %d %d %d\n",c_i, c_j, nic_connect[c_i + KEY2NODE_MOD][c_j]->sendnum);
                    nic_connect[c_i + KEY2NODE_MOD][c_j]->sendnum -= BatchPollSend;
                    // context.signal_flag[wc.wr_id] = 0;
                }
            }

            context_ptr->signalNum -= poll_num;
            poll_num = 1;
            if ((connect->sendnum < BatchPollSend) && pollflag) {
                return true;
            }
            if ((connect->signal_num == 0) && sync_op) {
                return true;
            }
            if ((context.signalNum < BatchSignalSend) && (!sync_op) && (!pollflag)) {
                return true;
            }
            tmp = pollWithCQN(context_ptr->send_cq, poll_num, context_ptr->wc);
            if (tmp == -1)
                goto POLLCQ_FAIL;
        } while (true);
    }
    return true;

POLLCQ_FAIL:
    Debug::notifyError(
        "pollWithCQN poll_signal");
    return false;
}

bool Rpc::rdma_write_reply_nictx(RdmaContext* context_ptr, Connect* connect, ibv_send_wr* wr, bool inline_flag)
{

    ibv_qp* qp = connect->qp;
    struct ibv_send_wr* wrBad;

    connect->sendnum += 1;
    bool pollflag = false;
    if (connect->sendnum == BatchPollSend) {
        wr->send_flags = IBV_SEND_SIGNALED;
        connect->signal_num += 1;
        context_ptr->signalNum += 1;
    } else {
        wr->send_flags = 0;
    }
    if (inline_flag)
        wr->send_flags |= IBV_SEND_INLINE;

    if (connect->sendnum == 2 * BatchPollSend - 1) {
        pollflag = true;
    }

    // printf("write from %llx to %llx size = %d, rkey = %d lkey = %d\n data = %d\n",
    //     wr->sg_list->addr, wr->wr.rdma.remote_addr,
    //     wr->sg_list->length, wr->wr.rdma.rkey, wr->sg_list->lkey, *(uint32_t*)(wr->sg_list->addr+4));

    if (ibv_post_send(qp, wr, &wrBad) != 0) {
        Debug::notifyError("rdmaRead Send with RDMA_WRITE(WITH_IMM) failed.");
        exit(1);
        return false;
    }
    poll_signal(context_ptr, connect, pollflag, false);
    return true;
}

void reply_send()
{
}
