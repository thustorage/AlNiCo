#include "rpc.h"
uint8_t rpc::bit_map2num(uint16_t bitmapx)
{
    int count = 0;
    while (bitmapx) {
        bitmapx = (bitmapx - 1ULL) & bitmapx;
        count++;
    }
    return count;
}


uint8_t rpc::bit_map2num64(uint32_t* bit_eris_per_server)
{
    int count = 0;
    for (int i = 0; i < KEY2NODE_MOD; i++) {
        uint32_t bitmapx = bit_eris_per_server[i];
        while (bitmapx) {
            bitmapx = (bitmapx - 1) & bitmapx;
            count++;
        }
    }
    return count;
}

uint8_t rpc::bit_map2num64(uint16_t* bit_eris_per_server)
{
    int count = 0;
    for (int i = 0; i < KEY2NODE_MOD; i++) {
        uint32_t bitmapx = bit_eris_per_server[i];
        while (bitmapx) {
            bitmapx = (bitmapx - 1) & bitmapx;
            count++;
        }
    }
    return count;
}


// uint64_t Rpc::init_global_id(int i)
// {
//     return my_global_id * MAXMPL + i;
// }
bool rpc::poll_signal(bool pollflag, bool sync_op, uint64_t node_id)
{
    if (pollflag || sync_op || (context.signalNum >= 2 * BatchSignalSend)) {
        
        int poll_num = min(BatchSignalSend, context.signalNum + 1 / 2);
        int tmp = pollWithCQN(send_cq, poll_num, context.wc);
        if (tmp == -1)
            goto POLLCQ_FAIL;

        do {
            for (int i = 0; i < poll_num; i++) {
                auto& wc = context.wc[i];
                if (wc.status != IBV_WC_SUCCESS) {
                    Debug::notifyError("Failed status %s (%d) for wr_id %d",
                        ibv_wc_status_str(wc.status), wc.status,
                        (int)wc.wr_id);
                    // return -1;
                }

                if (wc.wr_id >= MAX_SERVER) {
                    connect[wc.wr_id - MAX_SERVER]->sendnum--;
                    connect[wc.wr_id - MAX_SERVER]->signal_num--;
                } else {
                    connect[wc.wr_id]->sendnum -= BatchPollSend;
                    connect[wc.wr_id]->signal_num--;
                    // context.signal_flag[wc.wr_id] = 0;
                }
            }
            
            context.signalNum -= poll_num;
            poll_num = 1;
            if ((connect[node_id]->sendnum < BatchPollSend) && pollflag) {
                return true;
            }
            if ((connect[node_id]->signal_num == 0) && sync_op) {
                return true;
            }
            if ((context.signalNum < BatchSignalSend) && (!sync_op) && (!pollflag)) {
                return true;
            }
            tmp = pollWithCQN(send_cq, poll_num, context.wc);
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
bool rpc::rdmaWrite(uint64_t node_id, uint64_t source, uint64_t size,
    int32_t imm, bool async)
{
    uint32_t remoteRKey = connect[node_id]->remoteRkey;
    uint32_t lkey = message_Lkey;
    uint64_t dest;
    bool rpcflag = true, pollflag = false;
    if (imm != -1)
        rpcflag = false;
    ibv_qp* qp = connect[node_id]->qp;
    struct ibv_sge sg;
    struct ibv_send_wr wr;
    struct ibv_send_wr* wrBad;

    fillSgeWr(sg, wr, source, size, lkey);
    connect[node_id]->sendnum += 1;
    dest = connect[node_id]->getRemoteAddr(source);
#if (DEBUG_ON)
    printf("%d write %d addr = %llx\n",thread_id,node_id,dest);
#endif
    if (rpcflag) {
        imm = (int32_t)(source - connect[node_id]->localBase);

        wr.imm_data = imm;
        wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;

        if (connect[node_id]->sendnum == BatchPollSend) {
            wr.send_flags = IBV_SEND_SIGNALED;
            context.signalNum += 1;
            // context.signal_flag[node_id] = 1;
        }
        wr.wr_id = node_id;
    } else {
        wr.opcode = IBV_WR_RDMA_WRITE;
        wr.send_flags = IBV_SEND_SIGNALED;
        context.signalNum += 1;
        wr.wr_id = node_id + MAX_SERVER;
    }
    if (connect[node_id]->sendnum == 2 * BatchPollSend - 1) {
        pollflag = true;
    }

    wr.wr.rdma.remote_addr = dest;
    wr.wr.rdma.rkey = remoteRKey;
    if (int bad_ret = ibv_post_send(qp, &wr, &wrBad) != 0) {
        Debug::notifyError(
            "rdmaWrite Send with RDMA_WRITE(WITH_IMM) failed. with bad return %d "
            "error %s ",
            bad_ret, strerror(bad_ret));
        Debug::notifyError("send %d imm=%d", node_id, imm);
        return false;
    }
    poll_signal(pollflag, (!rpcflag) && (!async), node_id);

    return true;
}



