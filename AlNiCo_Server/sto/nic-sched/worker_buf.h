/*
 * @Author: your name
 * @Date: 2021-07-13 11:14:41
 * @LastEditTime: 2022-01-12 12:31:53
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /sto/benchmark/worker_buf.h
 */
#pragma once
#include "stdio.h"
#include "fpga.h"
#include "Rpc.h"
inline uint32_t stockKeyToWare(uint64_t s_key)
{
    int sid = s_key % 100000;
    if (sid == 0)
        return s_key / 100000 - 1;
    return s_key / 100000;
}
extern bool zipf_report;
struct msg_t
{
    uint64_t addr;
    uint16_t type;
    ReqHead *volatile req_head;
    uint32_t last_num;
    reqPair reqP[10];
    
    int main_read_count;
    tx_key_t *volatile keys_array;
    uint *volatile args_array;
    rpc_id_t* rpc_id_ptr;

    uint32_t* ycsb_key_array;
    uint16_t* ycsb_col_array;
    uint64_t write_bits;

    void deserialization(uint64_t txn_addr)
    {
        addr = txn_addr;
        req_head = (ReqHead *volatile)txn_addr;
        rpc_id_ptr = (rpc_id_t*)&(req_head->rpcId);
        type = req_head->read_name;
        
        #ifdef NULLRPC
            return;
        #endif

        uint64_t addr_tmp = txn_addr + sizeof(ReqHead) + sizeof(SIZETYPE) * req_head->req_num;
        for (int i = 0; i < req_head->req_num; i++)
        {
            reqP[i].size = *(SIZETYPE *)(txn_addr + sizeof(ReqHead) + sizeof(SIZETYPE) * i);
            reqP[i].addr = addr_tmp;
            addr_tmp += reqP[i].size;
        }
        write_bits = *(uint64_t *volatile)reqP[0].addr;
        ycsb_key_array = (uint32_t *volatile)reqP[1].addr;
        ycsb_col_array = (uint16_t *volatile)reqP[2].addr;

        main_read_count = *(int *volatile)reqP[0].addr;
        keys_array = (tx_key_t *volatile)reqP[1].addr;
        args_array = (uint *volatile)reqP[2].addr;
    }
    int get_read_num(){
        return  YCSBSIZE - (reqP[3].size / 384);
    }

    rpc_id_t* get_rpc_id(){
        return (rpc_id_ptr);
    }

    /*NEW ORDER*/
    tx_key_t get_w_key()
    {
        return keys_array[1];
    }
    tx_key_t get_c_key()
    {
        return keys_array[0];
    }
    tx_key_t get_d_key()
    {
        return keys_array[2];
    }

    tx_key_t get_i_key(int id)
    {
        return keys_array[2 * id + 3];
    }
    uint64_t get_i_id(int id)
    {
        return keys_array[2 * id + 3].second;
    }

    tx_key_t get_s_key(int id)
    {
        return keys_array[2 * id + 4];
    }
    uint64_t get_s_w(int num, int id)
    {
        int ret = stockKeyToWare(keys_array[2 * id + 4].second);
        return ret;
    }
    uint64_t get_s_id(int id)
    {
        return keys_array[2 * id + 4].second;
    }

    uint64_t get_w_id()
    {
        return args_array[0];
    }
    uint64_t get_s_num() volatile
    {
        return args_array[3];
    }
    uint64_t get_c_id()
    {
        return args_array[2];
    }
    uint64_t get_d_id()
    {
        return args_array[1];
    }
    void get_last_num()
    {
        last_num = get_s_num();
    }
    void store_feature(int thread_id)
    {
        model_update.store_feature(thread_id, reqP[3].addr);
    }

    /*PAYMENT*/
    tx_key_t get_w_key_pay()
    {
        return keys_array[0];
    }
    tx_key_t get_d_key_pay()
    {
        return keys_array[1];
    }
    tx_key_t get_c_w_key_pay()
    {
        return keys_array[2];
    }
    tx_key_t get_c_d_key_pay()
    {
        return keys_array[3];
    }
    tx_key_t get_c_key_pay()
    {
        return keys_array[4];
    }
    tx_key_t get_c_c_key_pay()
    {
        return keys_array[5];
    }

    uint64_t get_w_id_pay()
    {
        return args_array[0];
    }
    uint64_t get_d_id_pay()
    {
        return args_array[1];
    }
    uint64_t get_c_w_id_pay()
    {
        return args_array[2];
    }
    uint64_t get_c_d_id_pay()
    {
        return args_array[3];
    }
    bool get_is_home_pay()
    {
        return (args_array[4] != 0);
    }
    bool get_by_name_pay()
    {
        return (args_array[5] != 0);
    }
    uint64_t get_c_id_pay()
    {
        return args_array[6];
    }

    uint64_t get_w_id_order()
    {
        return args_array[0];
    }
    uint64_t get_d_id_order()
    {
        return args_array[1];
    }
    bool get_by_name_order()
    {
        return (args_array[2] != 0);
    }
    uint64_t get_c_id_order()
    {
        return args_array[3];
    }


    uint64_t get_w_id_stock()
    {
        return args_array[0];
    }
    uint64_t get_d_id_stock()
    {
        return args_array[1];
    }


    uint64_t get_w_id_del()
    {
        return args_array[0];
    }

    
    /*ORDER STATUS*/
    /*YCSB*/
    int get_ycsb_size()
    {
        return YCSBSIZE;
    }
    inline uint32_t get_ycsb_key(int i)
    {
        // return (ycsb_key_array[i]);
        return (ycsb_key_array[i] + (ycsb_col_array[i]) * (YCSB_TABLE_SIZE / YCSBCOL)) % YCSB_TABLE_SIZE;
    }
    inline uint32_t get_ycsb_key_origin(int i)
    {
        return ycsb_key_array[i];
    }
    inline uint32_t get_ycsb_col_origin(int i)
    {
        // assert(ycsb_col_array[i] < 20);
        return ycsb_col_array[i];
    }
    inline uint32_t get_ycsb_col(int i)
    {
        return 0;
        // return ycsb_col_array[i] % 1;
    }
    bool get_ycsb_is_write(uint i)
    {
        return (write_bits & (1ull<i)) != 0;
    }
    void printff()
    {
        uint8_t *vec = (uint8_t *)(reqP[3].addr);
        for (int i = 0; i < 64; i++)
        {
            printf("%d(%2x) ", i, vec[i]);
        }
        puts(" ");
    }
};
class worker_msg_t
{
public:
    int worker_thread_id;
    uint64_t txn_reply_addr, txn_addr;
    uint16_t client_id, slot_id;
    msg_t now_msg;

    abort_queue_t abort_queue;
    int msg_id;
    volatile fpga_dispatch_queue_t *my_fpga_queue;
    worker_msg_t() {}
    void init(int id);
    bool next_txn_check();
    void reply_now(bool no_perf_flag = false);
    void count_count_only();
    void abort_key(tx_key_t key);
    void abort_key_ycsb(int i);
};

class strife_msg_t: public worker_msg_t
{
public:
    int batch_req_num;
    int last_epoch_num;
    volatile strife_queue_t *my_strife_queue;
    int strife_msg_id;
    int cq_signal_id;
    uint64_t txn_reply_addr_strife;

    uint32_t client_node_id;
    uint32_t client_thread_id;
    uint32_t clinet_slot_id;
    uint32_t client_rpc_id;
    
    strife_msg_t() {}
    void init_strife(int id);
    void reply_now_strife(); // execute done!
    void next_now_strife(); // delivery route done !
    void reply_now_partition(bool no_perf_flag); // execute done! and delivery route done! 
    bool next_txn_check_strife(); // pull queue 
    void count_strife();
    void store_count();
    // todo
    // new buffer addr!
    // next_txn_check_strife();
    // reply_now_strife(); // real reply
};

class central_scheduler_t
{
public:
    bool atomic_add;
    uint64_t message_addr;
    uint64_t thread_num;
    uint64_t run_count[MAX_CORE];
    volatile uint64_t head[MAX_CORE][8];
    fpga_dispatch_queue_t * volatile doorbell_queue[MAX_CORE];
    strife_queue_t * volatile strife_queue[MAX_CORE];

    volatile uint64_t c_feature[MAX_CORE][FEATURE_SIZE];
    volatile uint8_t c_byte_feature[MAX_CORE][FEATURE_SIZE/8];
    volatile uint32_t merge_weights[FEATURE_SIZE/8][256];
    uint8_t old_feature[SLOT_NUM * CLIENT_THREAD * MAX_CLIENT][FEATURE_SIZE / 8];
    uint16_t old_core_id[SLOT_NUM * CLIENT_THREAD * MAX_CLIENT];
    uint8_t * volatile weights;
    uint8_t * volatile filter;
    uint8_t * volatile strife_buffer;
    

    void update_weights();
    void push_new_request(uint16_t core_id, uint32_t slot_id);
    void push_new_request_stife(uint16_t core_id, uint64_t slot_addr);

    void replace_new_request(uint16_t core_id, uint32_t slot_id);
    void delate_old_request(uint32_t slot_id);
    uint16_t get_core_id(uint32_t slot_id, uint32_t default_id);
    uint64_t get_addr(uint32_t slot_id);
    uint64_t get_feature_addr(uint32_t slot_id);

    central_scheduler_t() {}
    void init(uint64_t queue_addr, uint64_t weights_addr, uint64_t filter_addr, uint64_t thread_num_);
    void cpu_alnico_init(uint64_t queue_addr, 
    uint64_t weights_addr, uint64_t filter_addr, uint64_t thread_num_);
    
    
    void run_once(int begin_id, int end_id, int thread_id);
    void run(int begin_id, int end_id, int thread_id);

    void init_strife(uint64_t queue_addr, uint64_t thread_num_);
    void run_strife_once(int begin_id, int end_id, int thread_id);

    void test_reply(int thread_id, uint64_t reply_addr);
};

class tpcc_delivery_t
{
public:
    volatile uint64_t head[24][8];
    uint64_t tail[24][8];
    volatile uint64_t *delivery_queue[24];
    const uint64_t length = 1024*MAX_CLIENT*CLIENT_THREAD;
    tpcc_delivery_t(){
        
    }
    void init(int warehouse_num){
        for (int i = 0; i < warehouse_num; i++){
            delivery_queue[i] = (volatile uint64_t *)malloc(length*sizeof(rpc_id_t)); 
            for (uint64_t j = 0; j < length; j++){
                delivery_queue[i][j] = 0xFFFFFFF;
            }
            tail[i][0] = 0; 
            head[i][0] = 0;
        }
        return;
    }

    bool check(int w_id, int thread_id){
        uint64_t offset = tail[w_id][0];
        uint64_t now = delivery_queue[w_id][offset];
        return (now != 0xFFFFFFF);
    }
    void reply_now_delivery(int w_id, int thread_id){
        uint64_t offset = tail[w_id][0];
        uint64_t now = delivery_queue[w_id][offset];
        delivery_queue[w_id][offset] = 0xFFFFFFF;
        rpc_id_t *rpc_id = (rpc_id_t *)&now;
        tail[w_id][0] = (tail[w_id][0] + 1) % length;
        performance.count(thread_id);
        auto &now_connect = server->nic_connect[rpc_id->my_nodeid][rpc_id->thread_id];
        int client_id = rpc_id->my_nodeid - KEY2NODE_MOD;
        int c_thread_id = rpc_id->thread_id;
        // if (rpc_id->my_nodeid>2 || rpc_id->thread_id>=24 || rpc_id->my_nodeid < 0 || rpc_id->thread_id < 0){
        //     puts("?? fuck");
        //     exit(0);
        //     return;
        // }
        // return;
        uint64_t reply_addr = now_connect->get_strife_send_addr(rpc_id->rpc_id);
        volatile uint64_t *r_addr = (volatile uint64_t *)(reply_addr);
        uint64_t value = (*r_addr);
        (*r_addr) = value + 11;

        tpcc_reply_delivery[client_id][c_thread_id] = 1;
        return;
    }
    void push_new_delivery(int w_id, rpc_id_t* rpc_id){
        uint64_t offset;
        offset = __sync_fetch_and_add(&(head[w_id][0]), 1) % length;
        delivery_queue[w_id][offset] = *(uint64_t *)(rpc_id);
        return;
    }

};

extern central_scheduler_t central_scheduler;
extern tpcc_delivery_t tpcc_delivery;
