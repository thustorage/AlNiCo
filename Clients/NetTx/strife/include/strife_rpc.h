
#ifndef strife_RPC_H
#define strife_RPC_H


#include "nictxn_rpc.h"

struct strife_rpc : public nictxn_rpc {
public:
    
    bool strife_flag[IDMAPADDRSIZE];
    int tpcc_type[IDMAPADDRSIZE];
    strife_rpc(){

    }
    strife_rpc(const NodeInfo& my_node_info,
        const std::vector<NodeInfo>& others,
        uint16_t thread_id_info, uint16_t port, memcached_st* _memc, int _dev_id = -1);
    void run_client_strife();
    bool strife_reply_ok(uint32_t rpc_id);
    // bool update_reply_flag(uint32_t rpc_id, uint16_t server_id);
};

#endif