#ifndef __SWITCH_COMMON_H__
#define __SWITCH_COMMON_H__
// #define USETXNORMAL

#include "Debug.h"
#include "Rdma.h"
#include "mystorage.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <errno.h>
#include <libmemcached/memcached.h>
#include <memory.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

#define ADD_ROUND(x, n) ((x) = ((x) + 1) % (n))


// #define MAX_THREAD 1
// #define ASYNCTXNUM 1
// #define RANGEOFKEY 100 // mystorage.h

#define RAW_PACKET_WR_ID 0x3f3f3f3f3f3f3f3f

#ifdef USEERIS

#ifndef ERIS_BOX
#define MESSAGE_SIZE 512 // byte
#else
#define MESSAGE_SIZE 1024 // byte
#endif

#else
#ifdef MULTISWITCH

#define MESSAGE_SIZE 96 // byte

#else

#ifdef SWITCH_BOX
#define MESSAGE_SIZE 97 // byte
#else
#define MESSAGE_SIZE 96 // byte
#endif

#endif

#endif

#define SIGNAL_BATCH 7

const uint32_t bitmap_server[24] = { 1, 2, 4, 8, 
16, 32ULL, 64ULL, 128ULL, 
256ULL, 512ULL, 1024ULL, 2048ULL, 
4096ULL, 8192ULL, 16384ULL, 32768ULL,
65536, 131072, 262144, 524288,
1048576, 2097152, 4194304, 8388608};

const uint32_t eris_bitmap[24] = { 1, 2, 4, 8, 
16, 32ULL, 64ULL, 128ULL, 
256ULL, 512ULL, 1024ULL, 2048ULL, 
4096ULL, 8192ULL, 16384ULL, 32768ULL,
65536, 131072, 262144, 524288,
1048576, 2097152, 4194304, 8388608};


struct SwitchTXMessage {
    // dst
    uint16_t switch_slot; // the slot in swtich
    uint8_t appID; // reserved
    uint8_t thread_id; // thread_id
    uint8_t coordinator_id; // node_id
    uint8_t TXID; // tx_id

    // server for counter / broadcast
    uint16_t lockServer;
    uint16_t validateServer;
    uint16_t replicationServer;
    uint8_t TXMessageType;
    uint8_t is_commit;

    uint8_t magic;
    // for debug only
    uint8_t send_id;

#ifdef SWITCH_BOX
    uint8_t box_d_n_id;
#endif

#ifdef MULTISWITCH
    uint16_t broadcastServer_up[3];
    uint16_t broadcastServer_down[3];
    uint8_t CounterNumber_up[3];
    uint8_t CounterNumber_down[3];
    uint16_t broadcastServer_0;
    uint8_t CounterNumber_0;
#endif
    uint16_t broadcastServer;
    uint8_t CounterNumber;

    // state

} __attribute__((packed));
//

struct ErisTXMessage {
    uint8_t tx_id;
    
    uint8_t c_n_id; // coordinator
    uint8_t c_t_id;
    uint8_t is_mc;
    uint8_t s_n_id; // server
    uint8_t s_t_id;
    uint8_t message_type;
    uint16_t node_map;

#ifdef ERIS_BOX
    uint32_t server_map[8]; // server base
#else
    uint16_t server_map[8]; // switch base
#endif

    uint8_t key_slot;
    uint8_t magic;
    // 15B

#ifdef ERIS_BOX
    uint16_t sequencer[192]; // server base 
#else
    uint8_t sequencer[96]; // switch base
#endif
    // 32B
    // key_slot * (25)
    uint16_t switch_slot;

    tx_pair key[10];
    uint64_t version[10];
    double value[10];
    uint8_t key_type[10];

}__attribute__((packed));

class switch_tx;
class rpc;

enum class ServerTxStatus {
    INIT,
    COMMITED,
    VD,
    LOCKED,
    FAILED,
    WAIT_RDMA,
};

struct switch_tx_status_t {
    ServerTxStatus status = ServerTxStatus::COMMITED;
    int slot_num;
    int read_slot_start;
    int read_slot_end;
    int write_slot_start;
    int write_slot_end;
    uint64_t size_addr;
    uint64_t slot_addr; 
    uint64_t wait_magic_num;
    // uint64_t time_count;
    // double time_a;
    // timespec lock_s, lock_e;
    // uint64_t v_count;
    // double time_v;
    // timespec v_s, v_e;
    SwitchTXMessage m;
    switch_tx_status_t()
    {
        // time_a = 0;
        // time_count = 0;
        status = ServerTxStatus::COMMITED;
    }
};


// inline void* hugePageAlloc(size_t size)
// {

//     void* res = mmap(NULL, size, PROT_READ | PROT_WRITE,
//         MAP_PRIVATE | MAP_ANONYMOUS /*| MAP_HUGETLB */, -1, 0);
//     if (res == MAP_FAILED) {
//         LOG_ERR("errno=%d,size = %d,%s mmap failed!\n", errno, size, getIP());
//     }

//     return res;
// }

inline uint16_t toBigEndian16(uint16_t v)
{
    uint16_t res;

    uint8_t* a = (uint8_t*)&v;
    uint8_t* b = (uint8_t*)&res;

    b[0] = a[1];
    b[1] = a[0];

    return res;
}

inline uint32_t toBigEndian32(uint32_t v) { return __builtin_bswap32(v); }

inline uint64_t toBigEndian64(uint64_t v) { return __builtin_bswap64(v); }
void generate_init(uint16_t my_node_id, SwitchTXMessage* msg);
void generate_bitmap(uint16_t my_node_id, uint16_t gather_bitmap, SwitchTXMessage* msg);
void init_bitmap(uint16_t my_node_id);
uint8_t bit_map2num_4_single4s(uint16_t bitmapx);
void init_bitmap_eris(ErisTXMessage* msg);
#endif /* __SWITCH_COMMON_H__ */