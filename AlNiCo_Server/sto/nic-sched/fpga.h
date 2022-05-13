/*
 * @Author: your name
 * @Date: 2021-07-13 16:01:42
 * @LastEditTime: 2022-01-09 16:01:20
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /sto/nic-sched/fpga.h
 */
#pragma once
#include <bits/stdc++.h>
#include "model_update.h"
#include <stdint.h>
#include "Debug.h"
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <xmmintrin.h>

#include <HugePageAlloc.h>
#define RESERVERD_MEM_START (1ull * 1024ull * 1024ull * 1024ull)

#define WARE_SIZE 10
#define DIST_SIZE 20
#define CUST_SIZE 110 
#define HIST_SIZE 10
#define NEWO_SIZE 10
#define ORDE_SIZE 10
#define ORLI_SIZE 10
#define ITEM_SIZE 115
#define STOC_SIZE 177
#define CUST_INDEX_SIZE 10
#define ORDER_INDEX_SIZE 10
#define ARGS_DATA_SIZE 20


const int feature_size_512base[TABLE_NUM_MAX] = { WARE_SIZE, DIST_SIZE, CUST_SIZE, HIST_SIZE, NEWO_SIZE, ORDE_SIZE,
    ORLI_SIZE, ITEM_SIZE, STOC_SIZE, CUST_INDEX_SIZE, ORDER_INDEX_SIZE, ARGS_DATA_SIZE, 0, 0, 0 };


extern int fpga_fd;
extern model_update_t model_update;
extern uint64_t fpga_mm;
extern uint64_t control_mm;
extern uint64_t host_mm;
extern uint64_t ddio_mm;
extern uint64_t reply_mm;
extern uint32_t* control_addr;
// extern volatile uint32_t fpga_init_over;
extern int operation_fd;
extern uint64_t fpga_operation_mm;
extern uint64_t parallel_ip_addr;
extern const uint32_t fpga_rev_size;

void print();
void print_ctl(int thread_id);
void print_queue();
void print_msg_0(uint64_t mm_addr);
void print_msg_i(uint64_t mm_id);
void print_control_0(uint64_t mm_addr);
void print_reply_0(uint64_t mm_addr);
void print_op();

struct fpga_dispatch_queue_t {
    uint32_t head; // 4
    uint32_t tail; // 4
    uint32_t message_count;
    uint8_t padding[244];
    volatile uint32_t queue_message[MESS_NUM][MESS_LEN / 4];
    inline uint32_t get_message_client_id(int id) volatile
    {
        // return fpga_dispatch_queue_cache[2];
        // return (queue_message[id][0]/SLOT_LEN) / (SLOT_NUM);
        // return queue_message[id][1];
#ifdef REAL_FPGA
        return queue_message[id][0] / (SLOT_NUM);
#else
        return queue_message[id][2];
#endif
    }

    inline uint32_t get_message_slot_id(int id) volatile
    {
        // return fpga_dispatch_queue_cache[3];
        // return (queue_message[id][0]/SLOT_LEN)  % (SLOT_NUM);
        // return queue_message[id][2];
#ifdef REAL_FPGA
        return queue_message[id][0] % (SLOT_NUM);
#else
        return queue_message[id][3];
#endif
    }

    inline void add_abort_key(int id, int cnt, uint64_t table, uint64_t key) volatile
    {
        if (queue_message[id][4] <= 5)
            queue_message[id][4]++;
        cnt = cnt % 5;
        queue_message[id][4 + cnt * 2 + 1] = table;
        queue_message[id][4 + cnt * 2 + 2] = key % FEATURE_TABLE;
        return;
    }

    inline void set_message_down(int id) volatile
    {
#ifdef REAL_FPGA
        // queue_message[id][1] = queue_message[id][0];
        queue_message[id][0] = 0xFFFFFFFF;
#else
        queue_message[id][0] = 3;
#endif
        // _mm_mfence();
    }

    inline bool check_now_message(int id) volatile
    {
        // memcpy((uint8_t*)fpga_dispatch_queue_cache, (uint8_t*)queue_message[id], 4 * 4);
#ifdef REAL_FPGA
        return queue_message[id][0] != 0xFFFFFFFF;
#else
        return queue_message[id][0] == 2;
#endif
    }

    inline void init() volatile
    {
        head = 0;
        tail = 0;
        message_count = 0;
        for (int i = 0; i < 52; i++) {
            padding[i] = 0;
        }
        for (int i = 0; i < MESS_NUM; i++) {
            queue_message[i][0] = 0xFFFFFFFF;
            queue_message[i][1] = 0;
            queue_message[i][2] = 0;
            queue_message[i][3] = 0;
        }
        _mm_mfence();
    }

    void print() volatile
    {
        printf("head=%d tail=%d count=%d \n", head, tail, message_count);
    }
} __attribute__((packed));
struct strife_queue_t {
    volatile uint64_t queue_message[strife_buf_num];
    inline bool check_now_message(int id) volatile
    {
        return queue_message[id] != 0xFFFFFFFF;
    }
    inline void set_message_down(int id) volatile
    {
        queue_message[id] = 0xFFFFFFFF;
    }


    inline uint64_t get_message_addr(int id) volatile
    {
        return queue_message[id];

    }

    inline void init() volatile
    {
        for (int i = 0; i < strife_buf_num; i++) {
            queue_message[i] = 0xFFFFFFFF;
        }
        _mm_mfence();
    }

    
} __attribute__((packed));

extern volatile fpga_dispatch_queue_t* fpga_dispatch_queue;
extern volatile strife_queue_t* strife_dispatch_queue;

void test_fpga();
void init_fpga();
void set_udma_addr_fpga(uint64_t addr = 0);
void* p_alloc(size_t size, size_t offset);
void init_feature_partition();
void init_feature_partitionycsb();
void begin_fpga();
