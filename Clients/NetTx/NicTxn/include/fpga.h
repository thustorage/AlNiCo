
#ifndef FPGA_H
#define FPGA_H

#include "Debug.h"

#include <atomic>
#include <immintrin.h>

void virtual_fpga(volatile uint32_t* m, int con, uint32_t BASE_ADDR, uint32_t CLIENT_NUM);
void init_fpga();
void begin_fpga();
void print();
void print_ctl(int thread_id);
void print_queue();
void print_msg_0(uint64_t mm_addr);
void print_msg_i(uint64_t mm_id);
void print_control_0(uint64_t mm_addr);
void print_reply_0(uint64_t mm_addr);
void print_op();
void test_fpga();
void pcie_sleep();
unsigned long get_addr();
uint64_t dma_buf(uint64_t size);
size_t virtual_to_physical(size_t addr);


extern int fpga_fd;
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
#ifdef PCILATENCY
        pcie_sleep();
#endif
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
#ifdef PCILATENCY
        pcie_sleep();
        pcie_sleep();
#endif
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
extern volatile fpga_dispatch_queue_t* fpga_dispatch_queue;

#endif