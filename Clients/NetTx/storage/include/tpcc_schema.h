#ifndef __TPCC_SCHEMA_H__
#define __TPCC_SCHEMA_H__

#include "encoder.h"
#include "inline_str.h"
#include "macros.h"
//#include "framework/bench.h"
#define SHORTKEY 1

#define WARE 0
#define DIST 1
#define CUST 2
#define HIST 3
#define NEWO 4
#define ORDE 5
#define ORLI 6
#define ITEM 7
#define STOC 8
#define CUST_INDEX 9
#define ORDER_INDEX 10
#define ARGS_DATA 11

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


const int cross_node[9][40] = { { 0 }, 
    { 7, 1, 2, 4, 8, 16, 32, 64 },
    { 21, 3, 5, 6, 9, 10, 12, 17, 18, 20, 24, 33, 34, 36, 40, 48, 65, 66, 68, 72, 80, 96 },
    { 35, 7, 11, 13, 14, 19, 21, 22, 25, 26, 28, 35, 37, 38, 41, 42, 44, 49, 50, 52, 56, 67, 69, 70, 73, 74, 76, 81, 82, 84, 88, 97, 98, 100, 104, 112 },
    { 35, 15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120 },
    { 21, 31, 47, 55, 59, 61, 62, 79, 87, 91, 93, 94, 103, 107, 109, 110, 115, 117, 118, 121, 122, 124 },
    { 7, 63, 95, 111, 119, 123, 125, 126 },
    { 1, 127 },
    { 1, 255 }};

#define CUSTOMER_KEY_FIELDS(x, y) \
    x(int64_t, c_id)
#if 0
  x(int32_t,c_w_id) \
  y(int32_t,c_d_id) \
  y(int32_t,c_id)
#endif
#define CUSTOMER_VALUE_FIELDS(x, y)                                                                              \
    x(float, c_discount)                                                                                         \
        y(inline_str_fixed<2>, c_credit)                                                                         \
            y(inline_str_8<16>, c_last)                                                                          \
                y(inline_str_8<16>, c_first)                                                                     \
                    y(float, c_credit_lim)                                                                       \
                        y(uint64_t, snapshot)                                                                    \
                            y(AMOUNT, c_balance)                                                                 \
                                y(AMOUNT, c_balance_1)                                                           \
                                    y(AMOUNT, c_balance_2)                                                       \
                                        y(AMOUNT, c_ytd_payment)                                                 \
                                            y(int32_t, c_payment_cnt)                                            \
                                                y(int32_t, c_delivery_cnt)                                       \
                                                    y(inline_str_8<20>, c_street_1)                              \
                                                        y(inline_str_8<20>, c_street_2)                          \
                                                            y(inline_str_8<20>, c_city)                          \
                                                                y(inline_str_fixed<2>, c_state)                  \
                                                                    y(inline_str_fixed<9>, c_zip)                \
                                                                        y(inline_str_fixed<16>, c_phone)         \
                                                                            y(uint32_t, c_since)                 \
                                                                                y(inline_str_fixed<2>, c_middle) \
                                                                                    y(inline_str_16<500>, c_data)
DO_STRUCT(customer, CUSTOMER_KEY_FIELDS, CUSTOMER_VALUE_FIELDS)

#define CUSTOMER_NAME_IDX_KEY_FIELDS(x, y) \
    x(int32_t, c_index_id)                 \
        y(inline_str_fixed<16>, c_last)    \
            y(inline_str_fixed<16>, c_first)
#if 0
  x(int32_t,c_w_id) \
  y(int32_t,c_d_id)
#endif
#define CUSTOMER_NAME_IDX_VALUE_FIELDS(x, y) \
    x(int64_t, c_id)
#if 0
	x(int32_t,c_id)
#endif
DO_STRUCT(customer_name_idx, CUSTOMER_NAME_IDX_KEY_FIELDS, CUSTOMER_NAME_IDX_VALUE_FIELDS)

#define DISTRICT_KEY_FIELDS(x, y) \
    x(int64_t, d_id)
#if 0
  x(int32_t,d_w_id) \
  y(int32_t,d_id)
#endif
#define DISTRICT_VALUE_FIELDS(x, y)                             \
    x(int64_t, d_ytd)                                           \
        y(float, d_tax)                                         \
            y(int32_t, d_next_o_id)                             \
                y(inline_str_8<10>, d_name)                     \
                    y(inline_str_8<20>, d_street_1)             \
                        y(inline_str_8<20>, d_street_2)         \
                            y(inline_str_8<20>, d_city)         \
                                y(inline_str_fixed<2>, d_state) \
                                    y(inline_str_fixed<9>, d_zip)

DO_STRUCT(district, DISTRICT_KEY_FIELDS, DISTRICT_VALUE_FIELDS)

#define HISTORY_KEY_FIELDS(x, y) \
    x(int64_t, h_id)
#if 0
  x(int32_t,h_c_id) \
  y(int32_t,h_c_d_id) \
  y(int32_t,h_c_w_id) \
  y(int32_t,h_d_id) \
  y(int32_t,h_w_id) \
  y(uint32_t,h_date)
#endif
#define HISTORY_VALUE_FIELDS(x, y) \
    x(uint32_t, h_date)            \
        y(AMOUNT, h_amount)        \
            y(inline_str_8<25>, h_data)
DO_STRUCT(history, HISTORY_KEY_FIELDS, HISTORY_VALUE_FIELDS)

#define ITEM_KEY_FIELDS(x, y) \
    x(int64_t, i_id)
#if 0
  x(int32_t,i_id)
#endif
#define ITEM_VALUE_FIELDS(x, y)         \
    x(inline_str_8<24>, i_name)         \
        y(float, i_price)               \
            y(inline_str_8<50>, i_data) \
                y(int32_t, i_im_id)
DO_STRUCT(item, ITEM_KEY_FIELDS, ITEM_VALUE_FIELDS)

#define NEW_ORDER_KEY_FIELDS(x, y) \
    x(int64_t, no_id)
#if 0
  x(int32_t,no_w_id) \
  y(int32_t,no_d_id) \
  y(int32_t,no_o_id)
#endif
// need dummy b/c our btree cannot have empty values.
// we also size value so that it can fit a key
#define NEW_ORDER_VALUE_FIELDS(x, y) \
    x(inline_str_fixed<12>, no_dummy)
DO_STRUCT(new_order, NEW_ORDER_KEY_FIELDS, NEW_ORDER_VALUE_FIELDS)

#define OORDER_KEY_FIELDS(x, y) \
    x(int64_t, o_id)
#if 0
  x(int32_t,o_w_id) \
  y(int32_t,o_d_id) \
  y(int32_t,o_id)
#endif
#define OORDER_VALUE_FIELDS(x, y)    \
    x(int32_t, o_c_id)               \
        y(int32_t, o_carrier_id)     \
            y(int8_t, o_ol_cnt)      \
                y(bool, o_all_local) \
                    y(uint32_t, o_entry_d)
DO_STRUCT(oorder, OORDER_KEY_FIELDS, OORDER_VALUE_FIELDS)

#define OORDER_C_ID_IDX_KEY_FIELDS(x, y) \
    x(int64_t, o_index_id)
#if 0
  x(int32_t,o_w_id) \
  y(int32_t,o_d_id) \
  y(int32_t,o_c_id) \
  y(int32_t,o_o_id)
#endif
#define OORDER_C_ID_IDX_VALUE_FIELDS(x, y) \
    x(uint8_t, o_dummy)
DO_STRUCT(oorder_c_id_idx, OORDER_C_ID_IDX_KEY_FIELDS, OORDER_C_ID_IDX_VALUE_FIELDS)

#define ORDER_LINE_KEY_FIELDS(x, y) \
    x(int64_t, ol_id)
#if 0
  x(int32_t,ol_w_id) \
  y(int32_t,ol_d_id) \
  y(int32_t,ol_o_id) \
  y(int32_t,ol_number)
#endif
#define ORDER_LINE_VALUE_FIELDS(x, y)      \
    x(int32_t, ol_i_id)                    \
        y(uint32_t, ol_delivery_d)         \
            y(AMOUNT, ol_amount)           \
                y(int32_t, ol_supply_w_id) \
                    y(int8_t, ol_quantity)
DO_STRUCT(order_line, ORDER_LINE_KEY_FIELDS, ORDER_LINE_VALUE_FIELDS)
#define ORDER_LINE_SHORT_VALUE_FIELDS(x, y) \
    x(uint64_t, sn)                         \
        y(AMOUNT, ol_amount)                \
            y(uint32_t, ol_delivery_d)
DO_STRUCT(order_line_short, ORDER_LINE_KEY_FIELDS, ORDER_LINE_SHORT_VALUE_FIELDS)

#define STOCK_KEY_FIELDS_LOG(x, y) \
    x(int64_t, s_id)

#define STOCK_VALUE_FIELDS_LOG(x, y) \
    x(int16_t, s_quantity)           \
        y(AMOUNT, s_ytd)

DO_STRUCT(stock_log, STOCK_KEY_FIELDS_LOG, STOCK_VALUE_FIELDS_LOG)

#define STOCK_KEY_FIELDS(x, y) \
    x(int64_t, s_id)
#if 0
  x(int32_t,s_w_id) \
  y(int32_t,s_i_id)
#endif
#define STOCK_VALUE_FIELDS(x, y)         \
    x(int16_t, s_quantity)               \
        y(AMOUNT, s_ytd)                 \
            y(int32_t, s_order_cnt)      \
                y(int32_t, s_remote_cnt) \
                    y(int32_t, seq)

DO_STRUCT(stock, STOCK_KEY_FIELDS, STOCK_VALUE_FIELDS)

#define LOG_STOCK_KEY_FIELDS(x, y) \
    x(int64_t, s_id)
#define LOG_STOCK_VALUE_FIELDS(x, y) \
    x(int16_t, s_quantity)           \
        y(int32_t, s_remote_cnt)     \
            y(int32_t, seq)

DO_STRUCT(log_stock, LOG_STOCK_KEY_FIELDS, LOG_STOCK_VALUE_FIELDS)

#define STOCK_DATA_KEY_FIELDS(x, y) \
    x(int64_t, s_id)
#if 0
		x(int32_t,s_w_id) \
		y(int32_t,s_i_id)
#endif
#define STOCK_DATA_VALUE_FIELDS(x, y)                                      \
    x(inline_str_8<50>, s_data)                                            \
        y(inline_str_fixed<24>, s_dist_01)                                 \
            y(inline_str_fixed<24>, s_dist_02)                             \
                y(inline_str_fixed<24>, s_dist_03)                         \
                    y(inline_str_fixed<24>, s_dist_04)                     \
                        y(inline_str_fixed<24>, s_dist_05)                 \
                            y(inline_str_fixed<24>, s_dist_06)             \
                                y(inline_str_fixed<24>, s_dist_07)         \
                                    y(inline_str_fixed<24>, s_dist_08)     \
                                        y(inline_str_fixed<24>, s_dist_09) \
                                            y(inline_str_fixed<24>, s_dist_10)
DO_STRUCT(stock_data, STOCK_DATA_KEY_FIELDS, STOCK_DATA_VALUE_FIELDS)

#define WAREHOUSE_KEY_FIELDS(x, y) \
    x(int64_t, w_id)
//  x(int32_t,w_id)

#define WAREHOUSE_VALUE_FIELDS(x, y)                        \
    x(int64_t, w_ytd)                                       \
        y(float, w_tax)                                     \
            y(inline_str_8<10>, w_name)                     \
                y(inline_str_8<20>, w_street_1)             \
                    y(inline_str_8<20>, w_street_2)         \
                        y(inline_str_8<20>, w_city)         \
                            y(inline_str_fixed<2>, w_state) \
                                y(inline_str_fixed<9>, w_zip)
DO_STRUCT(warehouse, WAREHOUSE_KEY_FIELDS, WAREHOUSE_VALUE_FIELDS)

#endif /* __TPCC_SCHEMA_H__ */
