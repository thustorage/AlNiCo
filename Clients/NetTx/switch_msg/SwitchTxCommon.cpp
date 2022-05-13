/*
 * @Date: 2020-09-21 15:48:47
 * @LastEditTime: 2022-03-30 16:19:10
 * @FilePath: /TxSys/src/common/SwitchTxCommon.cpp
 * @Author: Li Junru
 * @LastEditors: Please set LastEditors
 */
#include "SwitchTxCommon.h"

// up for gather, down for scatter
uint16_t multi_switch_bitmap_up[4][1024];
uint16_t multi_switch_bitmap_down[4][1024];
uint8_t multi_switch_counter_up[4][1024];
uint8_t multi_switch_counter_down[4][1024];
int node_2_switch_map[20];


uint8_t bit_map2num_4_single4s(uint16_t bitmapx)
{
    int count = 0;
    while (bitmapx) {
        bitmapx = (bitmapx - 1ULL) & bitmapx;
        count++;
    }
    return count;
}

uint16_t assert_switch[4] = { 0, 7, 56, 192 };
void init_bitmap(uint16_t my_node_id)
{
    int old = dup(0);
    FILE* fp = freopen("../p4src_multi/host_cast", "r", stdin);
    uint64_t top = bitmap_server[KEY2NODE_MOD + 1];
    printf("total bitmap number = %lu\n", top);

    int padding;
    for (int i = 0; i < KEY2NODE_MOD; i++) {
        int port, node_id, switch_id;
        std::cin >> padding >> node_id >> switch_id;
        node_2_switch_map[node_id] = switch_id;
    }
    fflush(fp);
    dup2(old, 0);
    int server_flag[10];
    int my_switch = node_2_switch_map[my_node_id];
    for (int i = 0; i < top; i++) {
        memset(server_flag, 0, sizeof(server_flag));
        for (int j = 0; j < KEY2NODE_MOD; j++) {
            server_flag[j] = bitmap_server[j] & i ? 1 : 0;
        }
        uint16_t res_up = 0, res_down = 0;
        uint8_t counter_up = 0, counter_down = 0;

        /*scatter*/
        for (int j = 1; j <= 3; j++) {
            res_down = 0;
            for (int k = 0; k < KEY2NODE_MOD; k++) {
                if (server_flag[k] == 0)
                    continue;
                if (node_2_switch_map[k] == j) {
                    res_down |= bitmap_server[k];
                }
            }
            multi_switch_counter_down[j][i] = 1;
            multi_switch_bitmap_down[j][i] = res_down; // son switch
            // printf("assert switch-%d: %u != %u & %u = %u\n", j, res_down, assert_switch[j], i, assert_switch[j] & i);
            assert(res_down == (assert_switch[j] & i));
        }

        /*gather*/
        if (server_flag[my_node_id] != 0) {
            counter_up = 0;
            res_up = 0;
            for (int k = 0; k < KEY2NODE_MOD; k++) {
                if (server_flag[k] == 0)
                    continue;
                if (node_2_switch_map[k] == my_switch) {
                    counter_up++;
                    res_up |= bitmap_server[KEY2NODE_MOD + my_switch - 1]; // my switch to 0
                }
            }
            multi_switch_counter_up[my_switch][i] = counter_up;
            multi_switch_bitmap_up[my_switch][i] = res_up;
        }

        /*switch 0*/
        int switch_flag[4];
        memset(switch_flag, 0, sizeof(switch_flag));
        counter_up = 0;
        res_down = 0;
        for (int k = 0; k < KEY2NODE_MOD; k++) {
            if (server_flag[k] == 0)
                continue;
            res_down |= bitmap_server[KEY2NODE_MOD + node_2_switch_map[k] - 1 + 3];
            if (switch_flag[node_2_switch_map[k]] == 0) {
                counter_up++;
                switch_flag[node_2_switch_map[k]] = 1;
            }
        }
        multi_switch_counter_up[0][i] = counter_up;
        multi_switch_bitmap_down[0][i] = res_down; // father switch
    }
}
void print_multiswitch(uint16_t my_node_id, uint16_t gather_bitmap, SwitchTXMessage* msg)
{
#ifdef MULTISWITCH
    int my_switch = node_2_switch_map[my_node_id];
    printf("%d from %u to %u\n", msg->appID, gather_bitmap, msg->broadcastServer);
    printf("switch %d : %u %u\n", my_switch, msg->CounterNumber_up[my_switch - 1], msg->broadcastServer_up[my_switch - 1]);
    printf("switch 0 : %u %u\n", msg->CounterNumber_0, msg->broadcastServer_0);
    for (int i = 0; i < 3; i++) {
        printf("switch %d : %u %u\n", i + 1, msg->CounterNumber_down[i], msg->broadcastServer_down[i]);
    }
    fflush(stdout);
#endif
}
void generate_init(uint16_t my_node_id, SwitchTXMessage* msg)
{
#ifdef MULTISWITCH
    int my_switch = node_2_switch_map[my_node_id];
    msg->broadcastServer_up[my_switch - 1] = bitmap_server[KEY2NODE_MOD + my_switch - 1];
    msg->broadcastServer_0 = 14336ull - bitmap_server[KEY2NODE_MOD + my_switch + 2];
    msg->appID = 30;
#if (DEBUG_ON)
    puts("init");
    print_multiswitch(my_node_id, 0, msg);
#endif
#endif
    return;
}
int switch_map[8] = { 1, 1, 1, 2, 2, 2, 3, 3 };
void init_bitmap_eris(ErisTXMessage* msg)
{
    uint16_t node_map = msg->node_map;
    for (int i = 0; i < 8; i++) {
        if ((node_map & eris_bitmap[i]) == eris_bitmap[i]) {
            node_map = (node_map | eris_bitmap[8 + switch_map[i] + 2]);
        }
    }
    // printf("%d %x %x\n",msg->send_id, node_map, msg->node_map);
    fflush(stdout);
#ifdef ERISMULTI
    msg->node_map = node_map;
#endif
}
void generate_bitmap(uint16_t my_node_id, uint16_t gather_bitmap, SwitchTXMessage* msg)
{
    uint16_t scatter_bitmap = msg->broadcastServer;
    int my_switch;
#ifdef SINGLE4S
    my_switch = node_2_switch_map[my_node_id];
    msg->CounterNumber_up[my_switch - 1] = 1;
    msg->broadcastServer_up[my_switch - 1] = bitmap_server[KEY2NODE_MOD + my_switch - 1];
    if (false && (msg->TXMessageType == (uint8_t)TxControlType::VALIDATE_FAIL
        || (uint8_t)TxControlType::LOCK_FAIL == msg->TXMessageType)) {
        uint16_t other_fail_bitmap = scatter_bitmap - multi_switch_bitmap_down[my_switch][scatter_bitmap];
        msg->broadcastServer_up[my_switch - 1] = msg->broadcastServer_up[my_switch - 1]
            | multi_switch_bitmap_down[my_switch][scatter_bitmap];
        msg->CounterNumber_0 = msg->CounterNumber; // == 60
        msg->broadcastServer_0 = multi_switch_bitmap_down[0][other_fail_bitmap];
    }
    else{
        msg->CounterNumber_0 = msg->CounterNumber;
        // scatter switch 0
        msg->broadcastServer_0 = multi_switch_bitmap_down[0][scatter_bitmap];
    }
    for (int j = 1; j <= 3; j++) {
        msg->CounterNumber_down[j - 1] = multi_switch_counter_down[j][scatter_bitmap];
        msg->broadcastServer_down[j - 1] = multi_switch_bitmap_down[j][scatter_bitmap];
    }
    // gather switch 0
    // msg->CounterNumber_0 = multi_switch_counter_up[0][gather_bitmap];
    // msg->CounterNumber_0 = bit_map2num_4_single4s(gather_bitmap);
    
    return;
#endif

#ifdef MULTISWITCH
    my_switch = node_2_switch_map[my_node_id];
    if (multi_switch_bitmap_down[my_switch][scatter_bitmap] == scatter_bitmap
        && multi_switch_bitmap_down[my_switch][gather_bitmap] == gather_bitmap) {
        // gather and scatter
        msg->CounterNumber_up[my_switch - 1] = multi_switch_counter_up[my_switch][gather_bitmap];
        msg->broadcastServer_up[my_switch - 1] = multi_switch_bitmap_down[my_switch][scatter_bitmap];
        msg->CounterNumber_0 = 0;
        msg->broadcastServer_0 = 0;
    } else {
        // gather my switch
        msg->CounterNumber_up[my_switch - 1] = multi_switch_counter_up[my_switch][gather_bitmap];
        if (msg->TXMessageType == (uint8_t)TxControlType::VALIDATE_FAIL
            || (uint8_t)TxControlType::LOCK_FAIL == msg->TXMessageType) {
            uint16_t other_fail_bitmap = scatter_bitmap - multi_switch_bitmap_down[my_switch][scatter_bitmap];
            msg->broadcastServer_up[my_switch - 1] = multi_switch_bitmap_up[my_switch][gather_bitmap]
                | multi_switch_bitmap_down[my_switch][scatter_bitmap];
            msg->CounterNumber_0 = multi_switch_counter_up[0][gather_bitmap]; // == 1
            msg->broadcastServer_0 = multi_switch_bitmap_down[0][other_fail_bitmap];
        } else {
            msg->broadcastServer_up[my_switch - 1] = multi_switch_bitmap_up[my_switch][gather_bitmap];
            // gather switch 0
            msg->CounterNumber_0 = multi_switch_counter_up[0][gather_bitmap];
            // scatter switch 0
            msg->broadcastServer_0 = multi_switch_bitmap_down[0][scatter_bitmap];
        }
        for (int j = 1; j <= 3; j++) {
            msg->CounterNumber_down[j - 1] = multi_switch_counter_down[j][scatter_bitmap];
            msg->broadcastServer_down[j - 1] = multi_switch_bitmap_down[j][scatter_bitmap];
        }
    }
#if (DEBUG_ON)
    print_multiswitch(my_node_id, gather_bitmap, msg);
#endif
#endif

    return;
}
