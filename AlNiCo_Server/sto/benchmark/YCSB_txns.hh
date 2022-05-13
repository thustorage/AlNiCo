#pragma once


#include <set>
#include "YCSB_bench.hh"

namespace ycsb {

static constexpr uint64_t max_txns = 200000;

#define YCSB_ABORT(abort, id) if (!abort) {\
        work_msg->abort_key_ycsb(id);\
        }

#define YCSB_RETRY_COUNT(abort, id) if (!abort) {\
        performance.retry_count(id);\
        }

// YCSB NEW TX
template <typename DBParams>
void ycsb_runner<DBParams>::gen_workload(int txn_size) {
    dist_init();
    return;
    for (uint64_t i = 0; i < max_txns; ++i) {
        ycsb_txn_t txn {};
        txn.ops.reserve(txn_size);
        std::set<uint32_t> key_set;
        for (int j = 0; j < txn_size; ++j) {
            uint32_t key;
            do {
                key = dd->sample();
            } while (key_set.find(key) != key_set.end());
            key_set.insert(key);
        }
        bool any_write = false;
        for (auto it = key_set.begin(); it != key_set.end(); ++it) {
            bool is_write = ud->sample() < write_threshold;
            ycsb_op_t op {};
            op.is_write = is_write;
            op.key = *it;
            op.col_n = ud->sample() % (2*HALF_NUM_COLUMNS); /*column number*/
            if (is_write) {
                any_write = true;
                ig.random_ycsb_col_value_inplace(&op.write_value);
            }
            txn.ops.push_back(std::move(op));
        }
        txn.rw_txn = any_write;
        workload.push_back(std::move(txn));
    }
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(workload), std::end(workload), rng);
}
template <typename DBParams>
void ycsb_runner<DBParams>::next_tx(ycsb_txn_t* new_ycsb_txn, int txn_size) {
        new_ycsb_txn->ops.clear();
        // new_ycsb_txn->ops.reserve(txn_size);
        new_ycsb_txn->cnt = txn_size;
        std::set<uint32_t> key_set;
        key_set.clear();
        for (int j = 0; j < txn_size; ++j) {
            uint32_t key;
            do {
                key = dd->sample();
            } while (key_set.find(key) != key_set.end());
            if (key_set.find(key) == key_set.end())
                key_set.insert(key);
            // printf("%d--\n ",key);
        }
        bool any_write = false;
        for (auto it = key_set.begin(); it != key_set.end(); ++it) {
            bool is_write = ud->sample() < write_threshold;
            ycsb_op_t op {};
            op.is_write = is_write;
            // op.is_write = true;
            op.key = *it;
            op.col_n = ud->sample() % (2*HALF_NUM_COLUMNS); /*column number*/
            if (is_write) {
                any_write = true;
                ig.random_ycsb_col_value_inplace(&op.write_value);
            }
            new_ycsb_txn->ops.push_back(std::move(op));
        }
        new_ycsb_txn->rw_txn = any_write;
}
template <typename DBParams>
void ycsb_runner<DBParams>::msg_next_tx(ycsb_txn_t* new_ycsb_txn, worker_msg_t* work_msg){
    // new_ycsb_txn->ops.clear();
    int txn_size = work_msg->now_msg.get_ycsb_size();
    // if (runner_id == 0){
    // printf("size = %d\n",txn_size);
    // }
    bool any_write = false;
    // printf()
    for (int j = 0; j < txn_size; ++j) {
        // ycsb_op_t op {};
        new_ycsb_txn->ops[j].is_write = work_msg->now_msg.get_ycsb_is_write(j);
        new_ycsb_txn->ops[j].key = work_msg->now_msg.get_ycsb_key(j);
        new_ycsb_txn->ops[j].col_n = work_msg->now_msg.get_ycsb_col(j);
        if (new_ycsb_txn->ops[j].is_write){
            any_write = true;
            new_ycsb_txn->ops[j].write_value[0] = 'A';
        }
        #ifndef DYNAMIC
            work_msg->abort_key_ycsb(j);
        #endif
    }
    work_msg->abort_key_ycsb(0);
    new_ycsb_txn->cnt = txn_size;
    new_ycsb_txn->rw_txn = any_write;
}

using bench::access_t;

// YCSB run
template <typename DBParams>
void ycsb_runner<DBParams>::run_txn(ycsb_txn_t * txn, worker_msg_t* work_msg) {
    col_type output;
    typedef ycsb_value::NamedColumn nm;
    // usleep(1000000);
    // assert(txn->ops.size()==16);
    (void)output;
    // int cnt = 0;
    // if (runner_id == 0){
    //     work_msg->now_msg.printff();
    //     printf("\n num=%d %d: ",txn->cnt,txn->ops.size());
    //     for (int i = 0; i < txn->cnt; i++){
    //         printf("%d-%d-%d-%d\n",work_msg->now_msg.get_ycsb_col_origin(i), work_msg->now_msg.get_ycsb_key_origin(i), work_msg->now_msg.get_ycsb_key(i), txn->ops[i].key);
    //     }
    //     puts("");
    // }、
    ycsb_op_t tmps;
    int cnt = 0;
    TRANSACTION {
        if (cnt > 1){
            YCSB_RETRY_COUNT(false, runner_id);
        }
        cnt++;
        if (DBParams::MVCC && txn->rw_txn) {
            Sto::mvcc_rw_upgrade();
        }
        int id = 0;
        for (int i = 0; i < txn->cnt; i++) {
            auto& op = txn->ops[i];
            bool col_parity = op.col_n % 2;
            auto col_group = col_parity ? nm::odd_columns : nm::even_columns;
            (void)col_group;
            if (op.is_write) {
                
                ycsb_key key(op.key);
                auto [success, result, row, value]
                    = db.ycsb_table().select_split_row(key,
                    {{col_group, Commute ? access_t::write : access_t::update}}
                );
                (void)result;
                // YCSB_RETRY_COUNT(success, runner_id);
                TXN_DO(success);
                assert(result);
                // if (runner_id == 0){
                //     printf("%d\n", op.key);
                // }
                if constexpr (Commute) {
                    // op.write_value = tmps.write_value;
                    memcpy(op.write_value.c_str(), tmps.write_value.c_str(), COL_WIDTH);
                    commutators::Commutator<ycsb_value> comm(op.col_n, op.write_value);
                    db.ycsb_table().update_row(row, comm);
                    // std::cout << "78:"  << cnt<< std::endl;
#if TABLE_FINE_GRAINED
                } else if (DBParams::MVCC) {
                    // MVCC loop also does a tx_alloc, so we don't need to do
                    // one here
                    ycsb_value new_val_base;
                    ycsb_value* new_val = &new_val_base;
                    if (col_parity) {
                        new_val->odd_columns = value.odd_columns();
                        new_val->odd_columns[op.col_n/2] = op.write_value;
                    } else {
                        new_val->even_columns = value.even_columns();
                        new_val->even_columns[op.col_n/2] = op.write_value;
                    }
                    db.ycsb_table().update_row(row, new_val);
#endif
                } else {
                    auto new_val = Sto::tx_alloc<ycsb_value>();
                    if (col_parity) {
                        new_val->odd_columns = value.odd_columns();
                        new_val->odd_columns[op.col_n/2] = op.write_value;
                    } else {
                        new_val->even_columns = value.even_columns();
                        new_val->even_columns[op.col_n/2] = op.write_value;
                    }
                    db.ycsb_table().update_row(row, new_val);
                    // std::cout << "hear! no commute"<< std::endl;
                }
                // std::cout << op.write_value << std::endl;
                // break;
            } else {
                ycsb_key key(op.key);
                auto [success, result, row, value]
                    = db.ycsb_table().select_split_row(key, {{col_group, access_t::read}});
                (void)result; (void)row;
                // YCSB_RETRY_COUNT(success, runner_id);
                TXN_DO(success);
                assert(result);

                if (col_parity) {
                    output = value.odd_columns()[op.col_n/2];
                } else {
                    output = value.even_columns()[op.col_n/2];
                }
                // memcpy(tmps.write_value.c_str(), output.c_str(), COL_WIDTH);
            }
            id++;
        }
    } RETRY(true);
}

};
