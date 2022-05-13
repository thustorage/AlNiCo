/*
 * @Date: 2020-10-13 16:09:23
 * @LastEditTime: 2022-01-09 14:34:42
 * @FilePath: /TxSys/test/NICTX_client.cpp
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "nictxn_rpc.h"
using namespace std;
NodeInfo node_i;
std::vector<NodeInfo> node_list;
void node_config()
{
    node_list.clear();
    node_i.server_num = KEY2NODE_MOD;
    node_i.message_size = SLOT_LEN * SLOT_NUM;
    node_i.message_size_rpc_reply = REPLY_LEN * SLOT_NUM + 8 * 1024;
    node_i.double_write_data_size = DATA_LEN * SLOT_NUM;

    for (int i = 0; i < KEY2NODE_MOD; i++) {
        node_i.node_id = i;
        node_i.server_type = SERVERTYPE::SERVER_NIC;
        node_list.push_back(node_i);
    }
    for (int i = KEY2NODE_MOD; i < MAX_SERVER; i++) {
        node_i.node_id = i;
        node_i.server_type = SERVERTYPE::CLIENT_ONLY;
        node_list.push_back(node_i);
    }
}

int main(int argc, char* argv[])
{

    // switch_tx x;
    if (argc != 3) {
        cout << "[server_id] [dev_id]" << endl;
        return 0;
    }
    node_config();
    Debug::notifyError("config over");

    uint16_t server_id = atoi(argv[1]);
    uint16_t dev_id = atoi(argv[2]);
    if (server_id >= MAX_SERVER) {
        return 0;
    }
    // port == thread_id
    srand(server_id + 100);
    init_feature_partition();
    initstorage(server_id, 0);
    nictxn_rpc* test[CLIENT_THREAD];
    std::thread* ths = new std::thread[CLIENT_THREAD];
    memcached_st* memc = connectMemcached();
    
    if (memc == NULL) {
        Debug::notifyError("connect failed");
        // exit(0);
    }
    // freopen(("NICclinet_gdb" + to_string(server_id) + ".txt").c_str(), "a", stdout);
    Debug::notifyError("begin run");
    for (int i = 0; i < CLIENT_THREAD; i++) {
        test[i] = new nictxn_rpc(node_list[server_id], node_list, i, i, memc, dev_id);
    }

    usleep(1000000);

    // Debug::notifyError("begin run");
    printf("\n------Key = %d thread = %d async=%d-------\n", RANGEOFKEY, MAX_THREAD, ASYNCTXNUM);
    fflush(stdout);

    for (int i = 0; i < CLIENT_THREAD; i++) {
        ths[i] = std::thread(&nictxn_rpc::run_server, test[i]);
    }
    for (uint32_t i = 0; i < CLIENT_THREAD; i++) {
        ths[i].detach();
    }
    while (true) {
        sleep(10000);
    };
    return 0;
}