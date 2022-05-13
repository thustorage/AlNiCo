
#include "rpc.h"
rpc::rpc() { cmake_test = 66; }

rpc::rpc(const NodeInfo& my_node_info, const std::vector<NodeInfo>& others,
    uint16_t thread_id_info, uint16_t port, memcached_st* _memc,
    int _dev_id)
{
    thread_id = thread_id_info;
    my_node = my_node_info;
    my_global_id = my_node_info.node_id * CLIENT_THREAD + thread_id;
    rpc_seed = my_global_id * 19210817 + 19; // random seed
    node_id = my_node_info.node_id;
    others_server = others;
    dev_id = _dev_id;
    memset(rpcnum, 0, sizeof(rpcnum));

    if (my_node_info.server_type == SERVERTYPE::CLIENT_ONLY || my_node_info.server_type == SERVERTYPE::CLIENT_SERVER) {
        for (size_t j = 0; j < MAX_SERVER; j++) {
            idMapAddr[j] = new IdMapAddr[IDMAPADDRSIZE];
        }
        rpccount = 0;
    }

    memc = _memc;
}

rpc::rpc(const std::vector<NodeInfo>& others, uint16_t thread_id_info){
    others_server = others;
    thread_id = thread_id_info;
    return;
}

void rpc::alloc_rdma_mm()
{
    mm = (uint64_t)(volatile uint64_t*)hugePageAlloc(mm_size);
    // printf("%llx = %llx",mm,*(uint64_t *)mm);
    // mm = (uint64_t)(volatile uint64_t*)malloc(mm_size);
    memset((void*)mm, 0, mm_size); // don't change it!
    message_mr = createMemoryRegion(mm, mm_size, &context);
    message_Lkey = message_mr->lkey;
}

/* TODO: move to Class Connect */
void rpc::post_rec(Connect* connect)
{
    if (connect->recnum == 0) {
        struct ibv_recv_wr* bad_recv_wr;
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.1??");
        }
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.2??");
        }
        connect->recnum = 2 * BatchPostRec;
    }
    if (connect->recnum == BatchPostRec) {
        struct ibv_recv_wr* bad_recv_wr;
        if (ibv_post_recv(connect->qp, &connect->recv_wr[0], &bad_recv_wr)) {
            Debug::notifyError("Receive failed.3??");
        }
        connect->recnum = 2 * BatchPostRec;
    }
    return;
}

void rpc::write_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer, uint16_t client_thread_id)
{

    // Debug::notifyError("remoteid = %d localbase = %llx %llx",remote_id,
    // peer->localBase, mm);
    
    peer->tail = my_node.message_size - 1;

    localMeta.NodeID = my_node.node_id;
    localMeta.lid = context_ptr->lid;
    localMeta.qpNum = peer->qp->qp_num;
    memcpy(localMeta.gid, &(context_ptr->gid), 16);
    printf("qpnum=%d,lid=%d,gid=%d\n", localMeta.qpNum, localMeta.lid,
    localMeta.gid[15]);

    // register meta info
    bool if_server_nic = my_node.server_type == SERVERTYPE::SERVER_NIC || my_node.server_type == SERVERTYPE::SERVER_ONLY;
    memcached_return rc;
    std::string setK;
    setK = if_server_nic ? setKey_nic(remote_id, client_thread_id)
                         : setK = setKey(remote_id);

    int cnt = 10;
    while (true) {
        rc = memcached_set(memc, setK.c_str(), setK.size(), (char*)&localMeta,
            sizeof(localMeta), (time_t)0, (uint32_t)0);
        // Debug::notifyError(setK.c_str());
        if (rc == MEMCACHED_SUCCESS)
            break;
        if (--cnt < 0) {
            break;
        }
        fprintf(stderr, "Couldn't register my(%d, %d %d) info: %s, retry...\n",
            my_node.node_id, remote_id, thread_id,
            memcached_strerror(memc, rc));
        usleep(100);
    }
}


bool rpc::read_connect(uint16_t remote_id, RdmaContext* context_ptr, Connect* peer, uint16_t client_thread_id)
{
    
    bool if_server_nic = (my_node.server_type == SERVERTYPE::SERVER_NIC) || (my_node.server_type == SERVERTYPE::SERVER_ONLY);
    size_t l;
    uint32_t flags;
    std::string getK;
    getK = if_server_nic ? getKey_nic(remote_id, client_thread_id) : getKey(remote_id);

    ExchangeMeta* remoteMeta;
    memcached_return rc;
    int cnt = 1;
    while (cnt--) {
        remoteMeta = (ExchangeMeta*)memcached_get(memc, getK.c_str(), getK.size(), &l, &flags, &rc);
        if (rc == MEMCACHED_SUCCESS) {
            // construct peer data
            memcpy(peer->remoteGid, remoteMeta->gid, 16);
            peer->remoteAddr = remoteMeta->rAddr;
            peer->remoteRkey = remoteMeta->rkey;

            peer->remote_reply_addr = remoteMeta->reply_rAddr;
            peer->remote_reply_key = remoteMeta->reply_rkey;

            peer->remote_data_addr = remoteMeta->data_rAddr;
            peer->remote_data_key = remoteMeta->data_rkey;
            
            // printf("%llx i read\n", remoteMeta->read_rkey);
            peer->remoteQPNum = remoteMeta->qpNum;
            peer->remoteLid = remoteMeta->lid;
            peer->remoteNodeID = remoteMeta->NodeID;
            Debug::notifyError("remoteAddr=%lx rQPNum=%d, rLid=%d, rGid=%x %x %x %x(%d)",peer->remoteAddr, peer->remoteQPNum, peer->remoteLid,
                peer->remoteGid[12], peer->remoteGid[13], peer->remoteGid[14], peer->remoteGid[15], peer->remoteGid[15]);
            
            modifyQPtoInit(peer->qp, context_ptr);
            modifyQPtoRTR(peer->qp, peer->remoteQPNum, peer->remoteLid, peer->remoteGid, context_ptr);
            
            modifyQPtoRTS(peer->qp);
            free(remoteMeta);
            break;
        } else if (cnt == 0)
            return false;
    }

    

    peer->online = true;
    peer->thread_id = thread_id;
    if (!if_server_nic)
        peer->fill_wr(message_mr->lkey);
    return true;
}


