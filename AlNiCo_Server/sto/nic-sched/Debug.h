/*
 * @Date: 2020-10-05 15:22:27
 * @LastEditTime: 2022-01-09 15:17:26
 * @FilePath: /TxSys/include/common/Debug.h
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
/*** Debug header. ***/

/** Version 1 + Functional Model Modification **/

/** Redundance check. **/
#ifndef DEBUG_HEADER
#define DEBUG_HEADER

/** Included files. **/
#include <gperftools/profiler.h>
#include <stdarg.h> /* Standard argument operations. E.g. va_list */
#include <stdio.h> /* Standard I/O operations. E.g. vprintf() */
#include <sys/time.h> /* Time functions. E.g. gettimeofday() */
/** Defninitions. **/
#define MAX_FORMAT_LEN 255
#define DEBUG false
#define TITLE false
#define TIMER false
#define CUR false

#define barrier() asm volatile("mfence" \
                               :        \
                               :        \
                               : "memory")
// #define PROFILE

/*---------------------------DEBUG---------------------------*/
#define DEBUG_ON false
#define TEST_COUNT_ON false
#define KEY2NODE_MOD 1 // server_num
/*---------------------------DEBUG---------------------------*/
#define MAX_CLIENT 2 // client_num
// #define CLIENT_THREAD 13
// #define ASYNCTXNUM 2

#define FEATURE_SIZE 512
#define ABORT_DEPTH (4096)
#define FEATURE_TABLE (FEATURE_SIZE / 10)
#define TABLE_NUM_MAX 15

/*---------------------------TEST---------------------------*/
// #define USEERIS
// #define ERISMULTI
// #define MULTISWITCH
#define tpccmapsize (467)
#define USEZIPF
#define ZIPFTHETA 0.8

#define ERISOFFSET 5
#define CROSSNNNN 6
// #define SMALLBANK
#define TPCCBENCH
// #define CROSSALL

// #define CPUBASED 1
/*---------------------------TEST---------------------------*/

#define NOTHING_DEBUG 100
/** Classes. **/



#define ERISCQSIZE 256


// #define MAX_THREAD 20

// #define YCSBplusT

// #define YCSBCOL 10
#define YCSB_TABLE_SIZE 10000000
// #define YCSBSIZE 16

#define SLOT_LEN 2048

#ifdef YCSBplusT
#define REPLY_LEN 8192
#else
#define REPLY_LEN SLOT_LEN
#endif



#ifdef YCSBplusT
#define DATA_LEN (1024*18)
#else
#define DATA_LEN (2048)
#endif

#define SLOT_NUM ASYNCTXNUM
#define MAX_CORE 24

#define STRIFE_COST 16

#define MESS_LEN 64
#define MESS_NUM 256
#define strife_buf_num (1024*64)
#define QUEUE_METADATA 256


#define USE_RAW_PACKET

/*NIC*/
// #define QUICKCHECK

#define FPGA_MEMORY "/sys/bus/pci/devices/0000:83:00.0/resource0"
#define FPGA_OPERATION "/sys/bus/pci/devices/0000:83:00.0/resource1"
#define DEVICE_STATUS "/sys/bus/pci/devices/0000:83:00.0/resource2"
// #define DEVICE_MEMORY "/sys/bus/pci/devices/0000:1a:00.0/resource1"
// #define DEVICE_OPERATION "/sys/bus/pci/devices/0000:1a:00.0/resource2"

#define NICNICNIC

#define CORE_NUM MAX_THREAD
#define REAL_FPGA
#define PARALLEL_IP_NUM 1
#define EACH_IP_MESS_N (MESS_NUM/PARALLEL_IP_NUM)
#define VIVADO_IP_NUM 1
#define FPGATEST_BUF 4096
#define DOUBLERDMA


#define NICTXTESTSIZE 3
#define MAX_SERVER (KEY2NODE_MOD+MAX_CLIENT) // mystorage.h
// #define THREAD_NUM 4 // thread_num
// #define PCILATENCY 3000


#define COLD_LATENCY_NUM 20000

#define DEFAULT_NUM_HOT 1
#define TX_HOT 100


#define RESULT_PRINT_EPOCH 10000ull
#define RESULT_PRINT_BEGIN 3
#define RESULT_PRINT_END 10
#define RESULT_PRINT_PRINT 40

class Debug {
private:
    static long startTime; /* Last start time in milliseconds. */

public:
    static void debugTitle(const char* str); /* Print debug title string. */
    static void debugItem(const char* format, ...); /* Print debug item string. */
    static void debugCur(const char* format, ...); /* Print debug item string. */
    static void notifyInfo(const char* format, ...); /* Print normal notification. */
    static void notifyError(const char* format, ...); /* Print error information. */
    static void startTimer(const char*); /* Start timer and display information. */
    static void endTimer(); /* End timer and display information. */
};


/** Redundance check. **/
#endif
