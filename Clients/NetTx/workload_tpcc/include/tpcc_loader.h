/*
 * @Date: 2020-10-02 20:34:49
 * @LastEditTime: 2021-11-17 21:56:03
 * @FilePath: /TxSys/include/app/tpcc_loader.h
 * @Authors: Li Junru
 * @LastEditors: Please set LastEditors
 */
#ifndef __TPCC_LOADER_H__
#define __TPCC_LOADER_H__ 

#include "tpcc_common.h"
#include "mystorage.h"
#include <string>

static inline ALWAYS_INLINE void SanityCheckOrderLine(const order_line::key *k, const order_line::value *v)
  {
    INVARIANT(v->ol_i_id >= 1 && static_cast<size_t>(v->ol_i_id) <= NumItems());
  }
static inline ALWAYS_INLINE void      SanityCheckItem(const item::key *k, const item::value *v)
  {
    INVARIANT(v->i_price >= 1.0 && v->i_price <= 100.0);
  }

void tpcc_load(int partition_id);

// 
void ware_load(int partition_id);

void district_load(int partition_id);

void customer_load(int partition_id);

void order_load(int partition_id);

void stock_load(int partition_id);

void item_load(int partition_id);



#endif /* __TPCC_LOADER_H__ */