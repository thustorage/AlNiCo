
/** Version 1 + Functional Model Modification **/

/** Redundance check. **/
#ifndef DEBUG_HEADER
#define DEBUG_HEADER

/** Included files. **/
#include <gperftools/profiler.h>
#include <stdarg.h> /* Standard argument operations. E.g. va_list */
#include <stdio.h> /* Standard I/O operations. E.g. vprintf() */
#include <sys/time.h> /* Time functions. E.g. gettimeofday() */
#include <stdint.h>
/** Defninitions. **/
#define S2N (1e9)
#define MAX_FORMAT_LEN 255
#define DEBUG false
#define TITLE false
#define TIMER false
#define CUR false
#define PROFILE

/*---------------------------DEBUG---------------------------*/
#define DEBUG_ON false
#define TEST_COUNT_ON false
#define NOTHING_DEBUG 100

/*---------------------------DEBUG end---------------------------*/


/*---------------------------machines/threads configuration---------------------------*/
#define KEY2NODE_MOD 1 // server_num (s:8 / n:1)
#define MAX_CLIENT 2 // client_num (s:0 / n:2)
#define MAX_SERVER (KEY2NODE_MOD+MAX_CLIENT) // mystorage.h
// #define CLIENT_THREAD 13
/*---------------------------machines/threads configuration end---------------------------*/


/*---------------------------NicTxn---------------------------*/
#define NICNICNIC   // switchtx vs nictx
#define REAL_FPGA // switchtx vs nictx
#define DOUBLERDMA // switchtx vs nictx
#define CORE_NUM MAX_THREAD
#define PARALLEL_IP_NUM 1
#define EACH_IP_MESS_N (MESS_NUM/PARALLEL_IP_NUM)
#define VIVADO_IP_NUM 1
#define FPGATEST_BUF 4096
#define NICTXTESTSIZE 3
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

#ifndef RPCSIZE
#define RPCSIZE 1024
#endif


#define SLOT_NUM ASYNCTXNUM
#define MAX_CORE 24

#define MESS_LEN 64
#define MESS_NUM 256
#define QUEUE_METADATA 256

#define FPGA_MEMORY "/sys/bus/pci/devices/0000:83:00.0/resource0"
#define FPGA_OPERATION "/sys/bus/pci/devices/0000:83:00.0/resource1"
#define DEVICE_STATUS "/sys/bus/pci/devices/0000:83:00.0/resource2"
// #define DEVICE_MEMORY "/sys/bus/pci/devices/0000:1a:00.0/resource1"
// #define DEVICE_OPERATION "/sys/bus/pci/devices/0000:1a:00.0/resource2"

#define FEATURE_SIZE 512
#define ABORT_DEPTH 32
#define FEATURE_TABLE (FEATURE_SIZE / 10)
#define TABLE_NUM_MAX 15

// NicTxn benchmark
// #define YCSBplusT
#ifndef COL_WIDTH
#define COL_WIDTH 384
#endif

#ifndef YCSBSIZE
#define YCSBSIZE 16
#endif

#ifndef YCSBCOL
#define YCSBCOL 20
#endif 


#define YCSB_TABLE_SIZE 10000000
// #define YCSBCOL 10
// #define YCSBWRITE 50
// #define YCSBZIPF 1.2
#define USE_RAW_PACKET

// #define THREAD_NUM 4 // thread_num
// #define PCILATENCY 3000
/*---------------------------NicTxn END---------------------------*/


/*---------------------------benchmark configuration---------------------------*/

#define tpccmapsize (467)
#define USEZIPF
// #define ZIPFTHETA 0.99


// #define CROSSNNNN 2
// #define SMALLBANK
// #define TPCCBENCH
// #define CROSSALL

#define DEFAULT_NUM_HOT 1
#define TX_HOT 100
/*---------------------------benchmark configuration end---------------------------*/


/*---------------------------baseline configuration---------------------------*/
#define USEQSTORE
// #define USEERIS
// #define ERIS_BOX
// #define ERISMULTI
// #define MULTISWITCH
// #define ADMISSION_CONTROL

#ifndef TPCCBENCH
#define MAXMPL 8
#endif

#ifdef TPCCBENCH
#define MAXMPL 4
#endif

#ifndef ERIS_BOX
#define ERISOFFSET 5
#else
#define ERISOFFSET MAX_THREAD
#endif

#define BOX_THREAD 12

#define ERISCQSIZE 4096
/*---------------------------baseline configuration end---------------------------*/



/*---------------------------performance print---------------------------*/
#ifdef NULLRPC
#define COLD_LATENCY_NUM 3000
#else
#define COLD_LATENCY_NUM 5000
#endif
#define RESULT_PRINT_EPOCH 10000ull
#define RESULT_PRINT_BEGIN 3
#define RESULT_PRINT_END 10
#define RESULT_PRINT_PRINT 40
#define TRY_TIME_COUNT 20000ull
#define BREAKDOWN
/*---------------------------performance print end---------------------------*/

typedef uint32_t SIZETYPE;

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

// void bindCore(uint16_t core);

/** Redundance check. **/
#endif
