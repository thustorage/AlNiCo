/*
 * @Author: your name
 * @Date: 2020-12-12 10:48:39
 * @LastEditTime: 2021-11-20 15:28:52
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /TxSys/src/control/virtual_fpga.cpp
 */
#include "nictxn_rpc.h"
inline double max(double a, double b) { return a > b ? a : b; }
inline double min(double a, double b) { return a < b ? a : b; }
void pcie_sleep()
{
#ifdef PCILATENCY
    timespec nic_s, nic_e;
    clock_gettime(CLOCK_REALTIME, &nic_s);
    while (true) {
        clock_gettime(CLOCK_REALTIME, &nic_e);
        uint64_t lock_time = (nic_e.tv_sec - nic_s.tv_sec) * 1e9 + nic_e.tv_nsec - nic_s.tv_nsec;
        if (lock_time > PCILATENCY)
            break;
    }
#endif
}

// timespec nic_s, nic_e;
// clock_gettime(CLOCK_REALTIME, &nic_s);
// clock_gettime(CLOCK_REALTIME, &nic_e);
// uint64_t time = (nic_e.tv_sec - nic_s.tv_sec) * 1e9 + nic_e.tv_nsec - nic_s.tv_nsec;

unsigned long get_addr()
{
    int fd;
    char attr[1024];
    unsigned long phys_addr;
    if ((fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY)) != -1) {
        read(fd, attr, 1024);
        sscanf(attr, "%lx", &phys_addr);
        close(fd);
    }
    // printf("udmabuf phys_addr = %lx\n", phys_addr);
    return phys_addr;
}

uint64_t dma_buf(uint64_t size)
{

    uint64_t buf = 0;
    int fd;

    char attr[1024];
    unsigned long sync_mode = 2;
    if ((fd = open("/sys/class/u-dma-buf/udmabuf0/sync_mode", O_WRONLY)) != -1) {
        sprintf(attr, "%d", sync_mode);
        write(fd, attr, strlen(attr));
        close(fd);
    }

    if ((fd = open("/dev/udmabuf0", O_RDWR | O_ASYNC)) != -1) {
        buf = (uint64_t)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        /* Read/write access to the buffer */
        close(fd);
    }
    return buf;
}

size_t virtual_to_physical(size_t addr)
{
    int fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0) {
        printf("open '/proc/self/pagemap' failed!\n");
        return 0;
    }
    size_t pagesize = getpagesize();
    size_t offset = (addr / pagesize) * sizeof(uint64_t);
    if (lseek(fd, offset, SEEK_SET) < 0) {
        printf("lseek() failed!\n");
        close(fd);
        return 0;
    }
    uint64_t info;
    if (read(fd, &info, sizeof(uint64_t)) != sizeof(uint64_t)) {
        printf("read() failed!\n");
        close(fd);
        return 0;
    }
    if ((info & (((uint64_t)1) << 63)) == 0) {
        printf("page is not present!\n");
        close(fd);
        return 0;
    }
    size_t frame = info & ((((uint64_t)1) << 55) - 1);
    size_t phy = frame * pagesize + addr % pagesize;
    close(fd);
    return phy;
}

void test_fpga()
{
    uint64_t mm_nb = parallel_ip_addr;
    int* array = (int*)(fpga_mm);
    array[0] = 100;
    puts("\n----FPGA TEST----");
    print();
    for (int i = 0; i < 16; i++) {
        printf("%d,", array[i]);
        array[i] = i * i;
        printf("%d ", array[i]);
    }
    puts("\n--------");
    uint32_t* ap_start = (uint32_t*)(mm_nb);
    int* con = (int*)(mm_nb + 16);
    uint32_t* base_addr = (uint32_t*)(mm_nb + 24);
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
    for (int i = 0; i < 16; i++) {
        printf("%d ", array[i]);
    }
    puts("");

    memset((uint8_t*)fpga_mm, 0, fpga_rev_size + 4096);
    sleep(1);
    for (int i = 0; i <= 10; i++) {
        printf("%d%c", *(uint32_t*)(host_mm + i * 4), i == 10 ? '\n' : ' ');
    }
    puts("----END----\n");
}
void print_op()
{
    for (int i = 0; i < 9; i++) {
        printf("%d ", control_addr[i]);
    }

    printf(" || ");
    // int* p_op = (int*)parallel_ip_addr;
    // for (int i = 0; i < 26; i++) {
    //     printf("%lx ", p_op[i]);
    // }
    // puts("");
    uint32_t* find_addr = (uint32_t*)fpga_mm;
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
    // print_ctl();
#ifdef DOUBLERDMA
    print_control_0(fpga_mm); // the message in the fpga is the metadata
#else
    print_msg_0(fpga_mm);
#endif
    
    // print_msg_0(host_mm);
    if (reply_mm != 0) {
        print_reply_0(reply_mm);
    }
    print_op();
    puts("");
}
void print_ctl(int thread_id)
{
    printf("----THREAD(%d)----", thread_id);
    int* p_msg = (int*)(host_mm + fpga_rev_size);
    for (int i = thread_id; i <= thread_id; i++) {
        for (int j = 0; j <= 0; j++) {
            int offset = i * sizeof(fpga_dispatch_queue_t) + QUEUE_METADATA;
            printf("offset = %d\n", offset);
            for (int k = 0; k < EACH_IP_MESS_N; k++) {
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
    uint32_t* msg_0 = (uint32_t*)mm_addr;
    uint16_t size = (*msg_0) >> 16;
    uint16_t magic = (*msg_0) & 0xFFFF;
    printf("control0:%d %d//", size, magic);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 13);
    size = (*msg_0) >> 16;
    magic = (*msg_0) & 0xFFFF;
    printf("control13:%d %d//", size, magic);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 8);
    size = (*msg_0) >> 16;
    magic = (*msg_0) & 0xFFFF;
    printf("control8:%d %d /\n", size, magic);
}
void print_reply_0(uint64_t mm_addr)
{
    uint32_t* msg_0 = (uint32_t*)mm_addr;
    uint32_t reply = msg_0[1];
    uint32_t last_one = msg_0[2];
    printf("reply0:%d %d//", reply, last_one);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 13);
    reply = msg_0[1];
    last_one = msg_0[2];
    printf("reply13:%d %d//", reply, last_one);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 8);
    reply = msg_0[1];
    last_one = msg_0[2];
    printf("reply8:%d %d/\n", reply, last_one);
}
void print_msg_i(uint64_t mm_id)
{
    if (mm_id < MAX_CLIENT * CLIENT_THREAD * SLOT_NUM) {
        uint32_t* msg_0 = (uint32_t*)(host_mm + SLOT_LEN * mm_id);
        uint32_t id = msg_0[1];
        uint32_t size = msg_0[4];
        uint32_t magic = msg_0[5];
        uint32_t partition_hint = msg_0[6];
        uint32_t* tail_magic = (msg_0 + size / 4);
        printf("m%d:%d %d %d %d %d || ", mm_id, id, size, magic, *tail_magic, partition_hint);
        msg_0 = (uint32_t*)(fpga_mm + SLOT_LEN * mm_id);
        uint16_t control_partition_hint = (*msg_0) >> 16;
        uint16_t control_id = (*msg_0) & 0xFFFF;
        printf("control0:partition_hint=%d id=%d//\n", control_partition_hint, control_id);
    }
}
void print_msg_0(uint64_t mm_addr)
{
    uint32_t* msg_0 = (uint32_t*)mm_addr;
    uint32_t id = msg_0[1];
    uint32_t size = msg_0[4];
    uint32_t magic = msg_0[5];
    uint32_t partition_hint = msg_0[6];
    uint32_t* tail_magic = (msg_0 + size / 4);
    printf("m0:%d %d %d %d %d//", id, size, magic, *tail_magic, partition_hint);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 13);
    id = msg_0[1];
    size = msg_0[4];
    magic = msg_0[5];
    partition_hint = msg_0[6];
    tail_magic = (msg_0 + size / 4);
    printf("m13:%d %d %d %d %d//", id, size, magic, *tail_magic, partition_hint);
    msg_0 = (uint32_t*)(mm_addr + SLOT_LEN * 8);
    id = msg_0[1];
    size = msg_0[4];
    magic = msg_0[5];
    partition_hint = msg_0[6];
    tail_magic = (msg_0 + size / 4);
    printf("m8:%d %d %d %d %d//\n", id, size, magic, *tail_magic, partition_hint);
}
//

// rev buffer: SLOT_LEN * SLOT_NUM * CLIENT_NUM
// message: MAX_CORE * (MESS_NUM * MESS_LEN + QUEUE_METADATA)
// control 4096
// FPGATEST_BUF 4096
#define RESERVERD_MEM_START (1ull * 1024ull * 1024ull * 1024ull)
void* p_alloc(size_t size, size_t offset)
{
    void* vmem = NULL;

    //FILE * fp= fopen("/dev/mem", "w+");

    int fp = open("/dev/mem", O_RDWR | O_ASYNC);

    if (fp < 0) {

        printf("Open /dev/mem error!\n");

        return NULL;
    }
    printf("RESERVERD_MEM_START = %llx\n", RESERVERD_MEM_START);
    vmem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, RESERVERD_MEM_START + offset);

    if (vmem == NULL) {

        printf("mmap reserver mem on /dev/mem error!\n");

        return NULL;
    }
    return vmem;
}

void set_udma_addr_fpga(uint64_t addr = 0)
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
    *(uint32_t*)(mm + 0x208) = high_addr;
    *(uint32_t*)(mm + 0x20C) = low_addr;
    uint32_t* array = (uint32_t*)(mm + 0x208);

    for (int i = 0; i < 10; i++) {
        printf("%08lx ", array[i]);
    }

    puts("\n----");
    munmap((void*)mm, 4 * (1ULL << 10));
    close(s_fd);
    return;
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

#ifdef REAL_FPGA
    assert(control_size + fpga_rev_size <= 2 * 1024 * 1024);
    fpga_mm = (uint64_t)mmap((void*)0x400000000, fpga_rev_size / 2,
        PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0);
    printf("addr = ?%llx %d\n", fpga_mm, *(uint32_t*)(fpga_mm));
    uint64_t ans = (uint64_t)mmap((void*)(fpga_mm + fpga_rev_size / 2), fpga_rev_size / 2 + 4096,
        PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0x100000); // 1M offset
    printf("addr = ?%llx ans=%llx %d\n", fpga_mm, ans, *(uint32_t*)(fpga_mm));
    perror("fpga_mm mmap");

    host_mm = (uint64_t)p_alloc(control_size + fpga_rev_size, 0);
    for (int i = 0; i <= 10; i++) {
        *(uint32_t*)(host_mm + i * 4) = i + 1000;
        printf("%d%c", *(uint32_t*)(host_mm + i * 4), i == 10 ? '\n' : ' ');
    }

    set_udma_addr_fpga(0);
    control_mm = host_mm + fpga_rev_size;

    memset((uint8_t*)host_mm, 0, control_size + fpga_rev_size);

    control_addr = (uint32_t*)(fpga_mm + fpga_rev_size); // 1M offset

#else
    // fpga_mm = (uint64_t)mmap(NULL, control_size + fpga_rev_size,
    //     PROT_WRITE | PROT_READ, MAP_SHARED, fpga_fd, 0);
    fpga_mm = (uint64_t)(volatile uint64_t*)hugePageAlloc(control_size + fpga_rev_size);
    control_mm = fpga_mm + fpga_rev_size;
    control_addr = (uint32_t*)(fpga_mm + fpga_rev_size + fpga_dispatch_queue_size);
#endif

    memset((uint8_t*)fpga_mm, 0, fpga_rev_size + 4096);
    _mm_mfence();
#ifdef REAL_FPGA
    sleep(3);
#endif
    fpga_dispatch_queue = (volatile fpga_dispatch_queue_t*)control_mm;
    for (int core_i = 0; core_i < MAX_THREAD; core_i++) {
        fpga_dispatch_queue[core_i].init();
    }

#ifdef REAL_FPGA
    sleep(1);
    fpga_operation_mm = (uint64_t)mmap(NULL, 4 * 1024 * VIVADO_IP_NUM, PROT_WRITE | PROT_READ, MAP_SHARED, operation_fd, 0);
    memset((uint8_t*)fpga_operation_mm, 0, 4 * 1024 * VIVADO_IP_NUM);
    // fpga_operation_mm = fpga_operation_mm + 4 * 1024;
    parallel_ip_addr = fpga_operation_mm;
    print();
#endif

    std::atomic_thread_fence(std::memory_order_release);
    printf("i need %ld  + %ld = %ld bytes memory ", control_size, fpga_rev_size,
        control_size + fpga_rev_size);
    printf("bufsize = %lfKB %lfMB\n", (control_size + fpga_rev_size) / 1024.0, (control_size + fpga_rev_size) / 1024.0 / 1024.0);
}

#define MY_COUNT(my_fpga_dispatch_queue) (my_fpga_dispatch_queue[2])
#define MY_HEAD(my_fpga_dispatch_queue) (my_fpga_dispatch_queue[0])
#define MY_TAIL(my_fpga_dispatch_queue) (my_fpga_dispatch_queue[1])

// layout
// [head][4:4][4:size1][4:size2] [4:num][size1:key][size2:args]
// head=[8:rpcid] [2:name][1:req_num][1] [1][1][2] [4:totalsize] [4:magic] [4:partition_hint]

uint8_t status[MAX_CORE][FEATURE_SIZE];
uint8_t m_status[MAX_CORE][MESS_NUM][FEATURE_SIZE];
double weight[FEATURE_SIZE];

// #define FIX_WEIGHT

void virtual_fpga(volatile uint32_t* m, int con, uint32_t BASE_ADDR, uint32_t CLIENT_NUM)
{
    // for test = 1
    // bindCore(11);

    for (int i = 0; i < FEATURE_SIZE; i++) {
        weight[i] = 1.0;
#ifdef FIX_WEIGHT
        if (i / FEATURE_TABLE == 1) {
            weight[i] = 1e11;
        }
#endif
    }
    uint64_t load_count = 0;
    *control_addr = 666;
    uint32_t i, j, k, k_offset;
    uint32_t addr_offset;
    uint32_t size = 0, magic = 0;
    volatile uint32_t* tail_magic;
    uint32_t f_to_c[MAX_CORE];
    uint32_t queue_addr[MAX_CORE];
    uint32_t c_id;
    volatile uint32_t* req_head;
    uint32_t queue_offset;
    volatile uint32_t* total_offset;
    uint32_t* work_count;
    uint32_t* head;
    uint32_t* tail;
    uint32_t* count;
    uint32_t* count_p;
    uint32_t local_head;
    uint32_t local_tail;
    uint32_t local_count;
    uint32_t now_key[FEATURE_SIZE];
    uint64_t finish_count = 0;

    /* schedule */
    bool cold_start = false;
    double factor = 1.1;
#ifdef FIX_WEIGHT
    double min_f = 0, max_f = 1e11;
#else
    double min_f = 0, max_f = 2;
#endif
    double similarity[CORE_NUM];
    const double cold_factor_base = 1e-6;
    double cold_factor;
    memset(m_status, 0, sizeof(m_status));
    memset(status, 0, sizeof(status));
    uint32_t kv_num;
    uint32_t args_num;
    uint32_t key_num;
    uint64_t* key_a;
    uint32_t args_offset;
    const uint32_t cold_requeset_num = 50000;
    int abort_key_num, abort_key_i;
    int key_b, key_e, key_id;
    uint32_t table;
    uint32_t key_mod, key;
    uint32_t filter_id;

    /**/

    if (con == 0x00000002) {
        total_offset = m + (SLOT_LEN * SLOT_NUM * CLIENT_NUM + MAX_CORE * (MESS_NUM * MESS_LEN + QUEUE_METADATA)) / 4;
        addr_offset = BASE_ADDR; // queue buffer
        for (i = 0; i < CORE_NUM; i++) {
            f_to_c[i] = addr_offset / 4;
            queue_addr[i] = addr_offset + QUEUE_METADATA;
            addr_offset += MESS_NUM * MESS_LEN + QUEUE_METADATA;
        }
        while (*total_offset == 666) {
            // init
            addr_offset = 0; // request buff
            if (cold_start == false || load_count == finish_count) {
                // cold start
                /*
                if (cold_start == true && load_count == finish_count) {
                    cold_start = false;
#ifndef FIX_WEIGHT
                    for (int gg = 0; gg < FEATURE_SIZE; gg++) {
                        if (weight[gg] > max_f * 0.5) {
                            weight[gg] = 1e11;
                        }
                    }
                    max_f = 1e11;
#endif
                }
                */
                for (i = 0; i < CLIENT_NUM; i++) {
                    for (j = 0; j < SLOT_NUM; j++) {
                        // Reqhead | data | tail_magic
                        req_head = (m + addr_offset / 4);
                        size = req_head[4]; // byte
                        if (size == 0) {
                            addr_offset += SLOT_LEN;
                            continue;
                        }
                        // printf("i,j = %d,%d %llx %d\n", i, j, (uint64_t)m, *(m + addr_offset / 4));
#if (DEBUG_ON)
                        printf("i,j = %d,%d %llx %d\n", i, j, (uint64_t)m, *(m + addr_offset / 4));
#endif

                        magic = req_head[5];
                        tail_magic = (m + (addr_offset + size) / 4);
                        // printf("size=%d\n",size);
                        if (magic != *tail_magic) {
                            addr_offset += SLOT_LEN;
                            continue;
                        }
                        c_id = req_head[6];
                        // printf("i,j = %d,%d %d\n", i, j, c_id);
                        /*
                        // process: schedule (Core)
                        {
                            kv_num = req_head[10];
                            args_num = req_head[9] >> 2;
                            key_num = req_head[8] >> 4;
                            key_a = (uint64_t*)(req_head + ((4 * 7 + 4 * 4) >> 2));
                            args_offset = (4 * 7 + 3 * 4 + req_head[8]) >> 2;

                            // deserialization
                            for (k = 0; k < CORE_NUM; k++) {
                                similarity[k] = 0;
                            }
                            key_b = 2, key_e = key_num;
                            for (int key_id = key_b; key_id < key_e; key_id++) {
                                table = key_a[key_id * 2];
                                key = key_a[key_id * 2 + 1];
                                filter_id = FEATURE_TABLE * table + key % FEATURE_TABLE;
                                for (int core_id = 0; core_id < CORE_NUM; core_id++) {
                                    if (status[core_id][filter_id] != 0)
                                        similarity[core_id] += weight[filter_id];
                                }
                            }

                            // chose the best one
                            c_id = load_count % CORE_NUM;
                            cold_factor = cold_factor_base / (max_f / 1.1);
                            for (k_offset = 1; k_offset < CORE_NUM; k_offset++) {
                                k = (k_offset + load_count) % CORE_NUM;
                                if (m[f_to_c[k] + 2] == 0) {
                                    if ((similarity[c_id]) < (max_f / 1.1)) {
                                        c_id = k;
                                    }
                                } else if (m[f_to_c[c_id] + 2] == 0) {
                                    if ((similarity[k]) > (max_f / 1.1)) {
                                        c_id = k;
                                    }
                                } else if (similarity[c_id] < similarity[k]) {
                                    c_id = k;
                                }
                                // if ((similarity[c_id] + cold_factor_base) / (m[f_to_c[c_id] + 2] + cold_factor) < (similarity[kk] + cold_factor_base) / (m[f_to_c[kk] + 2] + cold_factor)) {
                                //     c_id = k;
                                // }
                            }
                            if (load_count < cold_requeset_num)
                                c_id = (load_count) % MAX_THREAD;

                            // other schedulers
                            // c_id = req_head[6];
                            // c_id = (load_count) % MAX_THREAD;
                            // c_id = (i*SLOT_NUM + j) % MAX_THREAD;

                            // cold start
                            load_count++;
                            if (load_count == cold_requeset_num) {
                                cold_start = true;
                            }

                            // print
                            if (load_count % 500000 == 0) {
                                for (key_id = 0; key_id < FEATURE_SIZE; key_id++) {
                                    if (key_id / FEATURE_TABLE == 1 && key_id % FEATURE_TABLE <= 30 && key_id % FEATURE_TABLE >= 11) {
                                        printf("max = [%d %d] %lf\n", key_id / FEATURE_TABLE, key_id % FEATURE_TABLE, weight[key_id]);
                                    }
#ifndef FIX_WEIGHT
                                    else if (weight[key_id] > max_f - 3) {
                                        printf("ooo max = [%d %d] %lf\n", key_id / FEATURE_TABLE, key_id % FEATURE_TABLE, weight[key_id]);
                                    }
#endif
                                }
                                printf("%d max=%lf min=%lf factor=%lf\n", load_count, max_f, min_f, factor);
                            }
                        }

                        // process: update the model
                        {
                            local_head = m[f_to_c[c_id]];
                            for (key_id = key_b; key_id < key_e; key_id++) {
                                table = key_a[key_id * 2];
                                key = key_a[key_id * 2 + 1];
                                filter_id = FEATURE_TABLE * table + key % FEATURE_TABLE;
                                status[c_id][filter_id]++;
                                m_status[c_id][local_head][filter_id]++;
                                // printf("head = %d, [%d %d] +1 = %d\n", local_head, c_id, filter_id, status[c_id][filter_id]);
                            }
                        }
                        */

                        // tell the host, change the flag
                        {
                            local_head = m[f_to_c[c_id]];

                            queue_offset = queue_addr[c_id] + local_head * MESS_LEN;

                            // work_count[1] = queue_addr[c_id];
                            // work_count[2] = queue_offset;
                            // work_count[3] = (m[f_to_c[c_id]]);
                            // work_count[4] = (m[f_to_c[c_id]+2]);

                            m[(queue_offset + 4) / 4] = addr_offset;
                            m[(queue_offset + 8) / 4] = i;
                            m[(queue_offset + 12) / 4] = j;
                            m[(queue_offset + 16) / 4] = 0;

                            m[f_to_c[c_id]] = (local_head + 1) % MESS_NUM;
                            m[f_to_c[c_id] + 2]++;

                            *(tail_magic) = magic + 1; //ok
                            m[queue_offset / 4] = 2;

                            // work_count[5] = m[f_to_c[c_id]];
                            // work_count[6] = m[f_to_c[c_id]+2];
                            // work_count[7] = queue_offset / 4;
                            // work_count[8] = head[c_id]-m;
                            // work_count[9] = count[c_id]-m;
                        }
                        addr_offset += SLOT_LEN;
                    }
                }
            }
            for (k = 0; k < CORE_NUM; k++) {
                local_tail = m[f_to_c[k] + 1];
                local_count = m[f_to_c[k] + 2];
                addr_offset = (queue_addr[k] + (local_tail)*MESS_LEN) / 4;
                while (m[addr_offset] == 3) {
                    if (local_count == 0) {
                        break;
                    }

                    // update the model
                    /*
                    {
                        // record the abort key
                        abort_key_num = m[addr_offset + 4];
                        factor = (3) / max(1.0, (max_f - min_f));
                        for (abort_key_i = 0; abort_key_i < abort_key_num; abort_key_i++) {
                            table = m[addr_offset + 5 + abort_key_i * 2];
                            key_mod = m[addr_offset + 6 + abort_key_i * 2];
                            filter_id = FEATURE_TABLE * table + key_mod;
#ifndef FIX_WEIGHT
                            weight[filter_id] += factor;
#endif
                        }
                        m[addr_offset + 4] = 0;

                        // delete the processed request
                        finish_count++;
                        min_f = 1000;
                        max_f = 0;
                        for (key_id = 0; key_id < FEATURE_SIZE; key_id++) {
#ifndef FIX_WEIGHT
                            if (weight[key_id] < 1e10)
                                weight[key_id] *= 0.9999;
#endif
                            min_f = min(weight[key_id], min_f);
                            max_f = max(weight[key_id], max_f);

                            if (m_status[k][local_tail][key_id] == 0)
                                continue;
                            status[k][key_id] -= m_status[k][local_tail][key_id];
                            m_status[k][local_tail][key_id] = 0;
                            // printf("tail = %d [%d %d] -1 = %d\n", local_tail, k, key_id, status[k][key_id]);
                        }
                    }
                    */
                    local_count--;

                    local_tail = (local_tail + 1) % MESS_NUM;
                    if (local_tail == 0) {
                        addr_offset = (queue_addr[k] + (local_tail)*MESS_LEN) / 4;
                    } else {
                        addr_offset += MESS_LEN / 4;
                    }
                }
                m[f_to_c[k] + 1] = local_tail;
                m[f_to_c[k] + 2] = local_count;
            }
            // test bench
            // if (ccc> 10){
            //     break;
            // }
        }
    }
}

void begin_fpga()
{
    // return;

    print();
    uint32_t tmp;
    *control_addr = 666;
    tmp = *control_addr;

    _mm_mfence();
    // sleep(1);
    int client_num_per_ip = (MAX_CLIENT * CLIENT_THREAD + PARALLEL_IP_NUM - 1) / PARALLEL_IP_NUM;
    int core_num_per_ip = (CORE_NUM + PARALLEL_IP_NUM - 1) / PARALLEL_IP_NUM;
    for (int i = 0; i < PARALLEL_IP_NUM; i++) {
        printf("begin %d\n", i);
        parallel_ip_addr = fpga_operation_mm + 4 * 1024 * i;
        int* ap_start = (int*)(parallel_ip_addr);
        // int* con_valid = (int*)(fpga_operation_mm + 16);
        int* con = (int*)(parallel_ip_addr + 16);
        // int* base_addr_valid = (int*)(fpga_operation_mm + 24);
        int* base_addr = (int*)(parallel_ip_addr + 24);
        // int* client_num_valid = (int*)(fpga_operation_mm + 32);
        int* client_num = (int*)(parallel_ip_addr + 32);
        // int* slot_num_valid = (int*)(fpga_operation_mm + 40);
        int* slot_num = (int*)(parallel_ip_addr + 40);
        // int* core_num_valid = (int*)(fpga_operation_mm + 48);
        int* core_num = (int*)(parallel_ip_addr + 48);
        // int* mess_num_valid = (int*)(fpga_operation_mm + 56);
        int* mess_num = (int*)(parallel_ip_addr + 56);
        int* b_client = (int*)(parallel_ip_addr + 64);
        int* e_client = (int*)(parallel_ip_addr + 72);
        int* b_core = (int*)(parallel_ip_addr + 80);
        int* e_core = (int*)(parallel_ip_addr + 88);
        int* ip_id = (int*)(parallel_ip_addr + 96);

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