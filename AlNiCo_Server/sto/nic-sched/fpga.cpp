/*
 * @Author: your name
 * @Date: 2021-07-13 16:01:26
 * @LastEditTime: 2022-05-04 11:02:58
 * @LastEditors: Your Name you@example.com
 * @Description: In User Settings Edit
 * @FilePath: /sto/nic-sched/fpga.cpp
 */
#include "fpga.h"

int fpga_fd;
feature_partitiont_t feature_partition;
volatile fpga_dispatch_queue_t *fpga_dispatch_queue;
volatile strife_queue_t *strife_dispatch_queue;
uint32_t *control_addr;
// volatile uint32_t fpga_init_over;
uint64_t fpga_mm;
uint64_t control_mm;
uint64_t ddio_mm; // not sure whether ddio is ok
uint64_t host_mm;
uint64_t reply_mm = 0;
int operation_fd;
uint64_t fpga_operation_mm;
uint64_t parallel_ip_addr;
model_update_t model_update;
const uint32_t fpga_rev_size = SLOT_LEN * SLOT_NUM * MAX_CLIENT * CLIENT_THREAD;

void init_feature_partition()
{
    feature_partition.offset[0] = 0;
    feature_partition.size[0] = feature_size_512base[0];
    for (int i = 1; i < 12; i++)
    {
        feature_partition.offset[i] = feature_partition.offset[i - 1] + feature_size_512base[i - 1];
        feature_partition.size[i] = feature_size_512base[i];
    }
}

void init_feature_partitionycsb()
{
    feature_partition.offset[0] = 0;
    feature_partition.size[0] = FEATURE_SIZE;
    for (int i = 1; i < 12; i++)
    {
        feature_partition.offset[i] = FEATURE_SIZE;
        feature_partition.size[i] = 0;
    }
}

void set_udma_addr_fpga(uint64_t addr)
{
    puts("--set_udma_addr_fpga--");
    int s_fd = open(DEVICE_STATUS, O_RDWR);
    uint64_t mm = (uint64_t)mmap(NULL, 4 * (1ULL << 10), PROT_WRITE | PROT_READ, MAP_SHARED, s_fd, 0);

    // uint64_t addr = get_addr();
    if (addr == 0)
        addr = RESERVERD_MEM_START;
    uint32_t high_addr = (addr >> 32);
    uint32_t low_addr = (addr & ((1ull << 32) - 1));
    printf("addr = %lx %lx\n", high_addr, low_addr);
    *(uint32_t *)(mm + 0x208) = high_addr;
    *(uint32_t *)(mm + 0x20C) = low_addr;
    uint32_t *array = (uint32_t *)(mm + 0x208);

    for (int i = 0; i < 10; i++)
    {
        printf("%08lx ", array[i]);
    }

    puts("\n----");
    munmap((void *)mm, 4 * (1ULL << 10));
    close(s_fd);
    return;
}

void *p_alloc(size_t size, size_t offset)
{
    void *vmem = NULL;

    //FILE * fp= fopen("/dev/mem", "w+");

    int fp = open("/dev/mem", O_RDWR | O_ASYNC);

    if (fp < 0)
    {

        printf("Open /dev/mem error!\n");

        return NULL;
    }
    printf("RESERVERD_MEM_START = %llx\n", RESERVERD_MEM_START);
    vmem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, RESERVERD_MEM_START + offset);

    if (vmem == NULL)
    {

        printf("mmap reserver mem on /dev/mem error!\n");

        return NULL;
    }
    return vmem;
}

void print_op()
{
    for (int i = 0; i < 9; i++)
    {
        printf("%d ", control_addr[i]);
    }

    printf(" || ");
    // int* p_op = (int*)parallel_ip_addr;
    // for (int i = 0; i < 26; i++) {
    //     printf("%lx ", p_op[i]);
    // }
    // puts("");
    uint32_t *find_addr = (uint32_t *)fpga_mm;
    printf("(666==%d)", control_addr[0]);

    // printf("%d (find==%d)",(control_addr[3] + 16) / 4, find_addr[(control_addr[3] + 16) / 4]);
    // for (int i = 0; i < 16384; i++) {
    //     if (find_addr[i] == 10 || find_addr[i] == 323) {
    //         printf("i find %d = %d \n", i, find_addr[i]);
    //     }
    // }
}

void print()
{
    return;
    // print_ctl();
#ifdef DOUBLERDMA
    print_control_0(fpga_mm + 64); // the message in the fpga is the metadata
#else
    print_msg_0(fpga_mm);
#endif

    // print_msg_0(host_mm);
    if (reply_mm != 0)
    {
        print_reply_0(reply_mm);
    }
    print_op();
    puts("");
}
void print_ctl(int thread_id)
{
    printf("----THREAD(%d)----", thread_id);
    int *p_msg = (int *)(host_mm + fpga_rev_size);
    for (int i = thread_id; i <= thread_id; i++)
    {
        for (int j = 0; j <= 0; j++)
        {
            int offset = i * sizeof(fpga_dispatch_queue_t) + QUEUE_METADATA;
            printf("offset = %d\n", offset);
            for (int k = 0; k < EACH_IP_MESS_N; k++)
            {
                printf("%ddoorbell(%d,%d %d)\n", thread_id, k, p_msg[offset / 4], p_msg[offset / 4 + 1]);
                print_msg_i(p_msg[offset / 4]);
                // print_msg_i(p_msg[offset / 4 + 1]);
                offset += MESS_LEN;
            }
            puts("");
        }
    }
    // for (int i = 0; i < 256; i++) {
    //     if (i % 16 == 0) {
    //         printf("|");
    //     }
    //     printf("%d ", p_msg[i]);
    // }
    puts("");
}
// void print_queue(){
//     uint32_t* msg_0 = (uint32_t*)(fpga_mm+);

// }
void print_control_0(uint64_t mm_addr)
{
    puts("metadata on FPGA");
    uint32_t *msg_0 = (uint32_t *)mm_addr;
    uint16_t size = (*msg_0) >> 16;
    uint16_t magic = (*msg_0) & 0xFFFF;
    printf("control0:%d %d// ", size, magic);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 13);
    size = (*msg_0) >> 16;
    magic = (*msg_0) & 0xFFFF;
    printf("control13:%d %d// ", size, magic);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 8);
    size = (*msg_0) >> 16;
    magic = (*msg_0) & 0xFFFF;
    printf("control8:%d %d /\n", size, magic);
}
void print_reply_0(uint64_t mm_addr)
{
    uint32_t *msg_0 = (uint32_t *)mm_addr;
    uint32_t reply = msg_0[1];
    uint32_t last_one = msg_0[2];
    printf("reply0:%d %d//", reply, last_one);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 13);
    reply = msg_0[1];
    last_one = msg_0[2];
    printf("reply13:%d %d//", reply, last_one);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 8);
    reply = msg_0[1];
    last_one = msg_0[2];
    printf("reply8:%d %d/\n", reply, last_one);
}
void print_msg_i(uint64_t mm_id)
{
    if (mm_id < MAX_CLIENT * CLIENT_THREAD * SLOT_NUM)
    {
        uint32_t *msg_0 = (uint32_t *)(host_mm + SLOT_LEN * mm_id);
        uint32_t id = msg_0[1];
        uint32_t size = msg_0[4];
        uint32_t magic = msg_0[5];
        uint32_t partition_hint = msg_0[6];
        uint32_t *tail_magic = (msg_0 + size / 4);
        printf("m%d:%d %d %d %d %d || ", mm_id, id, size, magic, *tail_magic, partition_hint);
        msg_0 = (uint32_t *)(fpga_mm + SLOT_LEN * mm_id);
        uint16_t control_partition_hint = (*msg_0) >> 16;
        uint16_t control_id = (*msg_0) & 0xFFFF;
        printf("control0:partition_hint=%d id=%d//\n", control_partition_hint, control_id);
    }
}
void print_msg_0(uint64_t mm_addr)
{
    uint32_t *msg_0 = (uint32_t *)mm_addr;
    uint32_t id = msg_0[1];
    uint32_t size = msg_0[4];
    uint32_t magic = msg_0[5];
    uint32_t partition_hint = msg_0[6];
    uint32_t *tail_magic = (msg_0 + size / 4);
    printf("m0:%d %d %d %d %d//", id, size, magic, *tail_magic, partition_hint);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 13);
    id = msg_0[1];
    size = msg_0[4];
    magic = msg_0[5];
    partition_hint = msg_0[6];
    tail_magic = (msg_0 + size / 4);
    printf("m13:%d %d %d %d %d//", id, size, magic, *tail_magic, partition_hint);
    msg_0 = (uint32_t *)(mm_addr + SLOT_LEN * 8);
    id = msg_0[1];
    size = msg_0[4];
    magic = msg_0[5];
    partition_hint = msg_0[6];
    tail_magic = (msg_0 + size / 4);
    printf("m8:%d %d %d %d %d//\n", id, size, magic, *tail_magic, partition_hint);
}

void init_fpga()
{

    uint32_t fpga_dispatch_queue_size = sizeof(fpga_dispatch_queue_t) * MAX_CORE;
    assert(sizeof(fpga_dispatch_queue_t) == MESS_LEN * (MESS_NUM + 4));
    uint32_t control_size = fpga_dispatch_queue_size + 4096 + FPGATEST_BUF;

    puts(FPGA_MEMORY);
    fpga_fd = open(FPGA_MEMORY, O_RDWR);
    perror("open the fpga_fd");
    operation_fd = open(FPGA_OPERATION, O_RDWR);
    perror("open the operation_fd");
    printf("fpga=%d op=%d\n", fpga_fd, operation_fd);

    assert(control_size + fpga_rev_size <= 2 * 1024 * 1024);
    // TODO SIZE IS SMALL
    
    fpga_mm = (uint64_t)mmap(NULL, 0x80000,
                                  PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0x100000);
    memset((void *)fpga_mm, 0, 0x80000);
    _mm_mfence();
    munmap((void *)fpga_mm, 0x80000);
 
    fpga_mm = (uint64_t)mmap((void *)0x400000000, fpga_rev_size / 2,
                             PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0);
    printf("addr = ?%llx %d\n", fpga_mm, *(uint32_t *)(fpga_mm));
    uint64_t ans = (uint64_t)mmap((void *)(fpga_mm + fpga_rev_size / 2), fpga_rev_size / 2 + 4096,
                                  PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0x100000); // 1M offset
    printf("addr = ?%llx ans=%llx %d\n", fpga_mm, ans, *(uint32_t *)(fpga_mm));
    perror("fpga_mm mmap");

#ifdef CPUBASED
    // host_mm = (uint64_t)p_alloc(control_size + fpga_rev_size, 1024ull * 1024 * 4);
    host_mm = (uint64_t)malloc(control_size + fpga_rev_size);
#else 
    host_mm = (uint64_t)p_alloc(control_size + fpga_rev_size, 0);
#endif

    for (int i = 0; i <= 10; i++)
    {
        *(uint32_t *)(host_mm + i * 4) = i + 1000;
        
        printf("%d%c", *(uint32_t *)(host_mm + i * 4), i == 10 ? '\n' : ' ');
    }

    set_udma_addr_fpga(0);
    control_mm = host_mm + fpga_rev_size;

    memset((uint8_t *)host_mm, 0, control_size + fpga_rev_size);
    puts("ok0");
    control_addr = (uint32_t *)(fpga_mm + fpga_rev_size); // 1M offset
    puts("ok1");
    memset((uint8_t *)fpga_mm, 0, fpga_rev_size + 4096);
    puts("ok2");
    _mm_mfence();
#ifdef REAL_FPGA
    sleep(3);
#endif
    fpga_dispatch_queue = (volatile fpga_dispatch_queue_t *)control_mm;
    for (int core_i = 0; core_i < MAX_THREAD; core_i++)
    {
        fpga_dispatch_queue[core_i].init();
    }

#ifdef REAL_FPGA
    sleep(1);
    fpga_operation_mm = (uint64_t)mmap(NULL, 4 * 1024 * VIVADO_IP_NUM, PROT_WRITE | PROT_READ, MAP_SHARED, operation_fd, 0);
    memset((uint8_t *)fpga_operation_mm, 0, 4 * 1024 * VIVADO_IP_NUM);
    // fpga_operation_mm = fpga_operation_mm + 4 * 1024;
    parallel_ip_addr = fpga_operation_mm;
    print();
#endif

    // std::atomic_thread_fence(std::memory_order_release);
    printf("i need %ld  + %ld = %ld bytes memory ", control_size, fpga_rev_size,
           control_size + fpga_rev_size);
    printf("bufsize = %lfKB %lfMB\n", (control_size + fpga_rev_size) / 1024.0, (control_size + fpga_rev_size) / 1024.0 / 1024.0);
}

void test_fpga()
{
    uint64_t mm_nb = parallel_ip_addr;
    int *array = (int *)(fpga_mm);
    array[0] = 100;
    puts("\n----FPGA TEST----");
    print();
    for (int i = 0; i < 16; i++)
    {
        printf("%d,", array[i]);
        array[i] = i * i;
        printf("%d ", array[i]);
    }
    puts("\n--------");
    uint32_t *ap_start = (uint32_t *)(mm_nb);
    int *con = (int *)(mm_nb + 16);
    uint32_t *base_addr = (uint32_t *)(mm_nb + 24);
    // int* con_valid = (int*)(mm_nb + 16);
    // int* base_addr_valid = (int*)(mm_nb + 24);

    *con = 1;
    *base_addr = (uint64_t)array - fpga_mm;
    // *con_valid = 0x01;
    // *base_addr_valid = 0x01;
    _mm_mfence();
    barrier();
    *ap_start = 0x01;
    // _mm_mfence();

    sleep(1);

    puts("\n--------");
    print();
    for (int i = 0; i < 16; i++)
    {
        printf("%d ", array[i]);
    }
    puts("");

    memset((uint8_t *)fpga_mm, 0, fpga_rev_size + 4096);
    sleep(1);
    for (int i = 0; i <= 10; i++)
    {
        printf("%d%c", *(uint32_t *)(host_mm + i * 4), i == 10 ? '\n' : ' ');
    }
    puts("----END----\n");
}

void begin_fpga()
{
    // return;

    print();
    *control_addr = 666;
    // tmp = *control_addr;

    _mm_mfence();
    // sleep(1);
    int client_num_per_ip = (MAX_CLIENT * CLIENT_THREAD + PARALLEL_IP_NUM - 1) / PARALLEL_IP_NUM;
    int core_num_per_ip = (CORE_NUM + PARALLEL_IP_NUM - 1) / PARALLEL_IP_NUM;
    for (int i = 0; i < PARALLEL_IP_NUM; i++)
    {
        printf("begin %d\n", i);
        parallel_ip_addr = fpga_operation_mm + 4 * 1024 * i;
        int *ap_start = (int *)(parallel_ip_addr);
        // int* con_valid = (int*)(fpga_operation_mm + 16);
        int *con = (int *)(parallel_ip_addr + 16);
        // int* base_addr_valid = (int*)(fpga_operation_mm + 24);
        int *base_addr = (int *)(parallel_ip_addr + 24);
        // int* client_num_valid = (int*)(fpga_operation_mm + 32);
        int *client_num = (int *)(parallel_ip_addr + 32);
        // int* slot_num_valid = (int*)(fpga_operation_mm + 40);
        int *slot_num = (int *)(parallel_ip_addr + 40);
        // int* core_num_valid = (int*)(fpga_operation_mm + 48);
        int *core_num = (int *)(parallel_ip_addr + 48);
        // int* mess_num_valid = (int*)(fpga_operation_mm + 56);
        int *mess_num = (int *)(parallel_ip_addr + 56);
        int *b_client = (int *)(parallel_ip_addr + 64);
        int *e_client = (int *)(parallel_ip_addr + 72);
        int *b_core = (int *)(parallel_ip_addr + 80);
        int *e_core = (int *)(parallel_ip_addr + 88);
        int *ip_id = (int *)(parallel_ip_addr + 96);

        print();

        _mm_mfence();

        *base_addr = fpga_rev_size;
        *client_num = MAX_CLIENT * CLIENT_THREAD;
        *slot_num = SLOT_NUM;
        *core_num = CORE_NUM;
        *mess_num = MESS_NUM / PARALLEL_IP_NUM;

        *b_client = client_num_per_ip * i;
        *e_client = min(client_num_per_ip * (i + 1), MAX_CLIENT * CLIENT_THREAD);
        *b_core = 0;
        *e_core = 0;
        *ip_id = i * 2;

        *con = 2;

        _mm_mfence();
        print();
        *ap_start = 0x01;
        // *base_addr_valid = 0x01;
        // *client_num_valid = 0x01;
        // *slot_num_valid = 0x01;
        // *core_num_valid = 0x01;
        // *mess_num_valid = 0x01;

        // *con_valid = 0x01;

        print();
    }
    puts("-----init end-----");
}