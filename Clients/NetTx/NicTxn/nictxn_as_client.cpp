/*
 * @Date: 2020-10-03 17:33:40
 * @LastEditTime: 2022-04-03 21:22:27
 * @FilePath: /TxSys/src/control/rpc_core.cpp
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "nictxn_rpc.h"

bool nictxn_rpc::delivery_reply_ok(uint32_t rpc_id)
{
    rpc_id = rpc_id % IDMAPADDRSIZE;
    return no_delivery_flag[rpc_id];
}

void nictxn_rpc::run_tpcc_client()
{   
    int test_count_debug = 0;
    double wait_length = 100000.0;

    latency_evaluation_t delivery_latency(IDMAPADDRSIZE);
    latency_evaluation_t new_order_latency(SLOT_NUM);
    
    for (int i = 0; i < SLOT_NUM; i++) {
        if (thread_id == 0)
            new_order_latency.init_thread(i);
    }
    for (int i = 0; i < IDMAPADDRSIZE; i++) {
        strife_cqe[i] = 0;
        if (thread_id == 0)
            delivery_latency.init_thread(i);
        no_delivery_flag[i] = true;
    }

    int d_i = 0;
    while (true) {
        // if (thread_id!=0){
        //     continue;
        // }
        for (int i = 0; i < SLOT_NUM; i++) {
            // sleep(5);
            
            if (my_tx[i]->is_busy == false) {
                if (!(delivery_reply_ok(rpcnum[0]) && delivery_reply_ok(rpcnum[0] + 1))) {
                    break;
                }
                // switch_tx* new_tx = my_tx[i];

                test_count_client++;
                uint64_t addr = connect[my_tx[i]->single_node_id]->get_my_reply_addr(i);
                generate_tx(i); // call single one
                
                if (my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::del){
                    // if (thread_id == 0){
                    //     printf("send %d %d\n",my_tx[i]->single_rpc_id, rpcnum[0]);
                    //     fflush(stdout);
                    // }
                    no_delivery_flag[my_tx[i]->single_rpc_id] = false;
                    if (thread_id == 0)
                        delivery_latency.begin(my_tx[i]->single_rpc_id);
                }
                else if (my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::new_order && thread_id == 0){
                    new_order_latency.begin(i);
                }

            } else {
                uint64_t addr = connect[my_tx[i]->single_node_id]->get_my_reply_addr(i);
                test_count_debug++;
                // if (test_count_debug%1000000 == 0)
                //     printf("%lx: %d!=%d\n", addr, (*(volatile rpc_id_t*)addr).rpc_id, my_tx[i]->single_rpc_id + NOTHING_DEBUG);
                
                if ((*(volatile rpc_id_t*)addr).rpc_id == my_tx[i]->single_rpc_id + NOTHING_DEBUG) {
                    my_tx[i]->now_phase = (*(volatile rpc_id_t*)addr).my_nodeid;
                    if ((ok_count > 1000) && my_tx[i]->tpcc_latency_type == (int)tpcc_txn_type_t::new_order && thread_id == 0){
                        new_order_latency.end(i);
                    } 
                    throughput_c(i);
                    if (node_id == 1 && thread_id == 0 && (ok_count == 2 * COLD_LATENCY_NUM)){
                        #ifndef YCSBplusT
                            new_order_latency.merge_print("_tpcc_new_order",0);
                            delivery_latency.merge_print("_tpcc_delivery",0);
                        #else
                            #ifdef NULLRPC
                                #ifdef SPNIC2
                                    new_order_latency.merge_print("null_rpc_" + std::to_string(RPCSIZE),0);
                                    // delivery_latency.merge_print("nullrpc",0);
                                #else
                                    new_order_latency.merge_print("nic_rpc_" + std::to_string(RPCSIZE),0);
                                #endif
                            #else
                                new_order_latency.merge_print("_ycsb_latency",0);
                            #endif
                        #endif


                    }
                }
            }
        }

        if ((no_delivery_flag[d_i] == false) && update_reply_flag(d_i, 0)) {
            no_delivery_flag[d_i] = true;
            // if (thread_id == 0){
            //     printf("ok %d\n",d_i);
            //     fflush(stdout);
            // }
            if (ok_count >= 1000 && thread_id == 0){
                delivery_latency.end(d_i);
            }
        }
        d_i = (d_i + 1) % IDMAPADDRSIZE;
    }
    return;
}

bool nictxn_rpc::update_reply_flag(uint32_t rpc_id, uint16_t server_id)
{
    uint64_t addr = connect[server_id]->get_strife_reply_addr(rpc_id);
    uint64_t reply = *(volatile uint64_t*)addr;
    if (reply == strife_cqe[rpc_id] + 11) {
        strife_cqe[rpc_id] += 11;
        return true;
    } else {
        return false;
    }
}

void nictxn_rpc::run_client()
{
    int test_count_debug = 0;
    double wait_length = 100000.0;

    while (true) {

        for (int i = 0; i < SLOT_NUM; i++) {
            // sleep(5);
            if (my_tx[i]->is_busy == false) {
                test_count_client++;

                uint64_t addr = connect[my_tx[i]->single_node_id]->get_my_reply_addr(i);
                // (*(rpc_id_t*)addr).rpc_id = -1;
                // Debug::notifyError("o nono addr=%llx %d=?=%d ",addr,(*(rpc_id_t*)addr).rpc_id, my_tx[i]->single_rpc_id);

                generate_tx(i); // call single one
                my_tx[i]->wait_cnt = 0;

            } else {
                uint64_t addr = connect[my_tx[i]->single_node_id]->get_my_reply_addr(i);
                test_count_debug++;

                // uint64_t xx = wait_retry.end(i);
                // if (xx < wait_length) {
                //     // printf("time = %d %d\n", xx, wait_retry.end(i));
                //     continue;
                // }
                // rdma_read_reply_nictx(my_tx[i]->single_node_id, addr, sizeof(rpc_id_t), 0);
                
                if ((*(volatile rpc_id_t*)addr).rpc_id == my_tx[i]->single_rpc_id + NOTHING_DEBUG) {
                    throughput_c(i);
                }
            }
        }
    }
    return;
}

void nictxn_rpc::generate_tx(uint8_t tx_array_id)
{

    switch_tx* new_tx = my_tx[tx_array_id];
    // if (!new_tx->is_retry) {
    //     return;
    // }

    new_tx->start();

    new_tx->sys_execution(SINGLE_TX_TPCC_NEW_ORDER, NULL);
    // if (new_tx->is_retry == 0) {
    //     performance_total.begin(tx_array_id);
    // }
    // new_tx.execution(SINGLE_TX_TEST, NULL);
    return;
}

rpc_id_t nictxn_rpc::push_new_call_write(uint64_t NodeID, RPCTYPE rpc_type, reqPair* req, uint16_t req_num, uint16_t txn_type,
    uint8_t tx_id, uint32_t partition_hint, uint64_t feature_addr)
{
#ifdef NICNICNIC
    auto& addr = idMapAddr[NodeID][(rpcnum[NodeID]) % IDMAPADDRSIZE];
    if (addr.addr != 0) {
        Debug::notifyError("read rpcid is not enough?");
        exit(-1);
    }
    addr.addr = 0;
    addr.type = CallerType::NO_REPLY;
    addr.ctx = NULL;

    uint64_t sendAddr;

    sendAddr = connect[NodeID]->get_send_data_addr(tx_id);
    ReqHead* reqHead = (ReqHead*)sendAddr;
    // skip the same id.
    if (rpcnum[NodeID] % IDMAPADDRSIZE == reqHead->rpcId.rpc_id || rpcnum[NodeID] % IDMAPADDRSIZE + NOTHING_DEBUG== reqHead->rpcId.rpc_id) { 
        rpcnum[NodeID] = (rpcnum[NodeID] + 1) % IDMAPADDRSIZE;
    }
    rpc_id_t rpc_id_i((uint8_t)rpc_type, my_node.node_id, NodeID, thread_id, (rpcnum[NodeID]) % IDMAPADDRSIZE);

    rpcnum[NodeID] = (rpcnum[NodeID]+1) % IDMAPADDRSIZE;
    // printf("rpcnum=%d\n",rpcnum[0]);
    // fflush(stdout);
    my_tx[tx_id]->single_rpc_id = rpc_id_i.rpc_id;

    rpccount++;

    reqHead->read_name = txn_type;
    reqHead->rpcId = rpc_id_i;
    reqHead->TXID = tx_id;
    reqHead->req_num = req_num;
    #ifdef YCSBplusT
    reqHead->read_num = YCSBSIZE-req[3].size/COL_WIDTH;
    #endif

    // reqHead->write_num = write_num;

    reqHead->partition_hint = partition_hint;
    // printf("partition_hint = %d\n",partition_hint);

    // memcpy((uint8_t*)(sendAddr), (uint8_t*)&reqHead, sizeof(ReqHead));

    uint32_t location = sizeof(ReqHead);
    for (int i = 0; i < req_num; i++) {
        *(SIZETYPE*)(sendAddr + location) = req[i].size;
        location += sizeof(SIZETYPE);
    }
    for (int i = 0; i < req_num; i++) {
        if (req[i].size != 0) {
            memcpy((uint8_t*)(sendAddr + location), (uint8_t*)req[i].addr,
                req[i].size);
        }
        location += req[i].size;
    }
    uint32_t* tail_magic = (uint32_t*)(location + sendAddr);
    if (*tail_magic == 111) {
        *tail_magic = 222;
    } else {
        *tail_magic = 111;
    }

    reqHead->size = location;
    reqHead->magic = *tail_magic;
#if (DEBUG_ON)
    printf("size = %d\n", reqHead->size);
#endif

    uint64_t metadata_size;
    uint64_t metadata_addr = connect[NodeID]->serialization(tx_id, &metadata_size, feature_addr);

// #ifdef NULLRPC
//     null_rpc_write(NodeID, sendAddr, location + sizeof(reqHead->magic), metadata_addr, metadata_size);
// #endif

#ifndef SPNIC2
    double_rdma_write(NodeID, sendAddr, location + sizeof(reqHead->magic), metadata_addr, metadata_size); // data to host and nic
#else
    rdmaWrite_client(NodeID, sendAddr, location + sizeof(reqHead->magic), connect[NodeID]->get_remote_data_addr(sendAddr), connect[NodeID]->remote_data_key); // data to host
    // rdmaWrite_client(NodeID, metadata_addr, metadata_size, connect[NodeID]->getRemoteAddr(metadata_addr), connect[NodeID]->remoteRkey);
#endif

    return rpc_id_i;
#endif
    return rpc_id_t();
}

bool nictxn_rpc::rdma_read_reply_nictx(uint64_t node_id, uint64_t source, uint64_t size, bool async)
{
    // Debug::notifyError("rdmaRead1 signalNum=%d sendnum=%d",Context->signalNum,Context->sendnum);
    uint32_t remote_reply_key = connect[node_id]->remote_reply_key;
    uint32_t lkey = message_Lkey;
    uint64_t dest;
    ibv_qp* qp = connect[node_id]->qp;
    dest = connect[node_id]->get_read_remote_addr(source);
    // printf("I wait for %llx\n",dest);
    struct ibv_sge sg;
    struct ibv_send_wr wr;
    struct ibv_send_wr* wrBad;
    fillSgeWr(sg, wr, source, size, lkey);

    wr.opcode = IBV_WR_RDMA_READ;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr_id = node_id + MAX_SERVER;
    wr.wr.rdma.remote_addr = dest;
    wr.wr.rdma.rkey = remote_reply_key;

    context.signalNum += 1;
    bool pollflag = false;
    connect[node_id]->sendnum += 1;
    connect[node_id]->signal_num++;

    if (connect[node_id]->sendnum >= 2 * BatchPollSend - 2) {
        pollflag = true;
    }

    if (ibv_post_send(qp, &wr, &wrBad) != 0) {
        Debug::notifyError("rdmaRead Send with RDMA_WRITE(WITH_IMM) failed.");
        exit(1);
        return false;
    }
    uint32_t* shit = (uint32_t*)source;
    // printf("read %d %d\n", shit[0], shit[1]);
    // Debug::notifyError("rdmaRead2 signalNum=%d sendnum=%d",Context->signalNum,Context->sendnum);
    poll_signal(pollflag, true, node_id);
    //
    // Debug::notifyError("rdmaRead3 signalNum=%d sendnum=%d",Context->signalNum,Context->sendnum);
    return true;
}
bool nictxn_rpc::rdmaWrite_client(uint64_t node_id, uint64_t source, uint64_t size,
    uint64_t dest, uint32_t rkey)
{
    uint32_t lkey = message_Lkey;
    bool pollflag = false;
    ibv_qp* qp = connect[node_id]->qp;
    struct ibv_sge sg;
    struct ibv_send_wr wr;
    struct ibv_send_wr* wrBad;

    fillSgeWr(sg, wr, source, size, lkey);
    {
        wr.opcode = IBV_WR_RDMA_WRITE;
        wr.send_flags = IBV_SEND_SIGNALED;
        wr.wr_id = node_id + MAX_SERVER;
        wr.wr.rdma.remote_addr = dest;
        wr.wr.rdma.rkey = rkey;
        context.signalNum += 1;
        connect[node_id]->signal_num++;
        connect[node_id]->sendnum += 1;
    }
    
    if (connect[node_id]->sendnum >= 2 * BatchPollSend - 2) {
        pollflag = true;
    }

    if (int bad_ret = ibv_post_send(qp, &wr, &wrBad) != 0) {
        Debug::notifyError(
            "rdmaWrite Send with RDMA_WRITE(WITH_IMM) failed. with bad return %d "
            "error %s ",
            bad_ret, strerror(bad_ret));
        Debug::notifyError("double send %d", node_id);
        return false;
    }

    poll_signal(pollflag, false, node_id);

    return true;
}

bool nictxn_rpc::double_rdma_write(uint64_t node_id, uint64_t source_d, uint64_t size_d, uint64_t source_m, uint64_t size_m)
{
#ifdef NICNICNIC

    uint32_t remoteRKey_d = connect[node_id]->remote_data_key;
    uint32_t remoteRKey_m = connect[node_id]->remoteRkey;
    uint32_t lkey = message_Lkey;

    ibv_qp* qp = connect[node_id]->qp;
    struct ibv_sge sg_m;
    struct ibv_sge sg_d;
    struct ibv_send_wr wr[2];
    struct ibv_send_wr* wrBad;

    // fillSgeWr_1024(sg_d, wr[0], source_d, 1333, lkey);
    fillSgeWr(sg_d, wr[0], source_d, size_d, lkey);
    fillSgeWr(sg_m, wr[1], source_m, size_m, lkey);

    uint64_t dest_d = connect[node_id]->get_remote_data_addr(source_d);
    uint64_t dest_m = connect[node_id]->getRemoteAddr(source_m);
    // printf("metadata in %llx , write addr my %llx from %llx to %llx key=%d\n",dest_m, source_d, dest_d, dest_d + size_d, remoteRKey_d);

#if (DEBUG_ON)
    printf("addr = %llx\n", dest);
#endif
    {
        wr[0].opcode = IBV_WR_RDMA_WRITE;
        wr[0].send_flags = 0;
        wr[0].wr_id = node_id;
        wr[0].wr.rdma.remote_addr = dest_d;
        wr[0].wr.rdma.rkey = remoteRKey_d;

        wr[1].opcode = IBV_WR_RDMA_WRITE;
        wr[1].send_flags = 0;
        wr[1].wr_id = node_id;
        wr[1].wr.rdma.remote_addr = dest_m;
        wr[1].wr.rdma.rkey = remoteRKey_m;
        wr[0].next = &(wr[1]);
        connect[node_id]->sendnum += 2;
    }
    if (connect[node_id]->sendnum == BatchPollSend) {
        wr[1].send_flags = IBV_SEND_SIGNALED;
        connect[node_id]->signal_num++;
        context.signalNum += 1;
    }

    bool pollflag = false;
    if (connect[node_id]->sendnum >= 2 * BatchPollSend - 2) {
        pollflag = true;
    }

    if (int bad_ret = ibv_post_send(qp, wr, &wrBad) != 0) {
        Debug::notifyError(
            "rdmaWrite Send with RDMA_WRITE(WITH_IMM) failed. with bad return %d "
            "error %s ",
            bad_ret, strerror(bad_ret));
        Debug::notifyError("double send %d", node_id);
        return false;
    }

    poll_signal(pollflag, false, node_id);

#endif
    return true;
}
