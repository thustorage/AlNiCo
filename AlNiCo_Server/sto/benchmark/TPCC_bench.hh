#pragma once

#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <thread>

#include "barrier.hh" // pthread_barrier_t

#include "worker_buf.h"
#include "compiler.hh"
#include "clp.h"
#include "SystemProfiler.hh"
#include "DB_structs.hh"
#include "TPCC_structs.hh"
#include "TPCC_commutators.hh"

#if TABLE_FINE_GRAINED
#include "TPCC_selectors.hh"
#endif

#include "DB_index.hh"
#include "DB_params.hh"
#include "DB_profiler.hh"
#include "PlatformFeatures.hh"

#if TABLE_FINE_GRAINED
#include "tpcc_split_params_ts.hh"
#else
#include "tpcc_split_params_default.hh"
#endif

#define A_GEN_CUSTOMER_ID           1023
#define A_GEN_ITEM_ID               8191

#define C_GEN_CUSTOMER_ID           259
#define C_GEN_ITEM_ID               7911

// @section: clp parser definitions
enum {
    opt_dbid = 1, opt_nwhs, opt_nthrs, opt_time, opt_perf, opt_pfcnt, opt_gc,
    opt_gr, opt_node, opt_comm, opt_verb, opt_mix
};

extern const char* workload_mix_names[];
extern const Clp_Option options[];
extern const size_t noptions;
extern void print_usage(const char *);
// @endsection: clp parser definitions

extern int tpcc_d(int, char const* const*);
extern int tpcc_dc(int, char const* const*);
extern int tpcc_dn(int, char const* const*);
extern int tpcc_dcn(int, char const* const*);

extern int tpcc_o(int, char const* const*);
extern int tpcc_oc(int, char const* const*);

extern int tpcc_m(int, char const* const*);
extern int tpcc_mc(int, char const* const*);
extern int tpcc_mn(int, char const* const*);
extern int tpcc_mcn(int, char const* const*);

extern int tpcc_s(int, char const* const*);

extern int tpcc_t(int, char const* const*);
extern int tpcc_tc(int, char const* const*);
extern int tpcc_tn(int, char const* const*);
extern int tpcc_tcn(int, char const* const*);

namespace tpcc {

using namespace db_params;
using bench::db_profiler;

class tpcc_input_generator {
public:
    static const char * last_names[];

    tpcc_input_generator(int id, int num_whs)
            : gen(id), num_whs_(uint64_t(num_whs)) {}
    explicit tpcc_input_generator(int num_whs)
            : gen(0), num_whs_(uint64_t(num_whs)) {}

    uint64_t nurand(uint64_t a, uint64_t c, uint64_t x, uint64_t y) {
        uint64_t r1 = (random(0, a) | random(x, y)) + c;
        return (r1 % (y - x + 1)) + x;
    }
    uint64_t random(uint64_t x, uint64_t y) {
        std::uniform_int_distribution<uint64_t> dist(x, y);
        return dist(gen);
    }
    uint64_t num_warehouses() const {
        return num_whs_;
    }

    uint64_t gen_warehouse_id() {
        return random(1, num_whs_);
    }
    uint64_t gen_customer_id() {
        return nurand(A_GEN_CUSTOMER_ID, C_GEN_CUSTOMER_ID, 1, NUM_CUSTOMERS_PER_DISTRICT);
    }

    int gen_customer_last_name_num(bool run) {
        return (int)nurand(255,
                           run ? 223 : 157/* XXX 4.3.2.3 magic C number */,
                           0, 999);
    }

    std::string to_last_name(int gen_n) {
        assert(gen_n <= 999 && gen_n >= 0);
        std::string str;

        int q = gen_n / 100;
        int r = gen_n % 100;
        assert(q < 10);
        str += std::string(last_names[q]);

        q = r / 10;
        r = r % 10;
        assert(r < 10 && q < 10);
        str += std::string(last_names[q]) + std::string(last_names[r]);

        return str;
    }

    std::string gen_customer_last_name_run() {
        return to_last_name(gen_customer_last_name_num(true));
    }

    uint64_t gen_item_id() {
        return nurand(A_GEN_ITEM_ID, C_GEN_ITEM_ID, 1, NUM_ITEMS);
    }
    uint32_t gen_date() {
        return (uint32_t)random(1505244122, 1599938522);
    }
    std::mt19937& random_generator() {
        return gen;
    }

private:
    std::mt19937 gen;
    uint64_t num_whs_;
};

template <typename DBParams>
class tpcc_access;

#ifndef TPCC_HASH_INDEX
#define TPCC_HASH_INDEX 1
#endif

template <typename DBParams>
class tpcc_db {
public:
    template <typename K, typename V>
    using OIndex = typename std::conditional<DBParams::MVCC,
          mvcc_ordered_index<K, V, DBParams>,
          ordered_index<K, V, DBParams>>::type;

#if TPCC_HASH_INDEX
    template <typename K, typename V>
    using UIndex = typename std::conditional<DBParams::MVCC,
          mvcc_unordered_index<K, V, DBParams>,
          unordered_index<K, V, DBParams>>::type;
#else
    template <typename K, typename V>
    using UIndex = OIndex<K, V>;
#endif

    // partitioned according to warehouse id
    typedef UIndex<warehouse_key, warehouse_value>       wh_table_type;
    typedef UIndex<district_key, district_value>         dt_table_type;
    typedef UIndex<customer_key, customer_value>         cu_table_type;
    typedef OIndex<order_key, order_value>               od_table_type;
    typedef OIndex<orderline_key, orderline_value>       ol_table_type;
    typedef UIndex<stock_key, stock_value>               st_table_type;
    typedef UIndex<customer_idx_key, customer_idx_value> ci_table_type;
    typedef OIndex<order_cidx_key, bench::dummy_row>     oi_table_type;
    typedef OIndex<order_key, bench::dummy_row>          no_table_type;
    typedef UIndex<item_key, item_value>                 it_table_type;
    typedef OIndex<history_key, history_value>           ht_table_type;

    explicit inline tpcc_db(int num_whs);
    explicit inline tpcc_db(const std::string& db_file_name) = delete;
    inline ~tpcc_db();
    void thread_init_all();

    int num_warehouses() const {
        return static_cast<int>(num_whs_);
    }
    wh_table_type& tbl_warehouses() {
        return tbl_whs_;
    }
    dt_table_type& tbl_districts(uint64_t w_id) {
        return tbl_dts_[w_id - 1];
    }
    cu_table_type& tbl_customers(uint64_t w_id) {
        return tbl_cus_[w_id - 1];
    }
    od_table_type& tbl_orders(uint64_t w_id) {
        return tbl_ods_[w_id - 1];
    }
    ol_table_type& tbl_orderlines(uint64_t w_id) {
        return tbl_ols_[w_id - 1];
    }
    st_table_type& tbl_stocks(uint64_t w_id) {
        return tbl_sts_[w_id - 1];
    }
    ci_table_type& tbl_customer_index(uint64_t w_id) {
        return tbl_cni_[w_id - 1];
    }
    oi_table_type& tbl_order_customer_index(uint64_t w_id) {
        return tbl_oci_[w_id - 1];
    }
    no_table_type& tbl_neworders(uint64_t w_id) {
        return tbl_nos_[w_id - 1];
    }
    it_table_type& tbl_items() {
        return *tbl_its_;
    }
    ht_table_type& tbl_histories(uint64_t w_id) {
        return tbl_hts_[w_id - 1];
    }
    tpcc_oid_generator& oid_generator() {
        return oid_gen_;
    }
    tpcc_delivery_queue& delivery_queue() {
        return dlvy_queue_;
    }

private:
    size_t num_whs_;
    it_table_type *tbl_its_;

    wh_table_type tbl_whs_;
    std::vector<dt_table_type> tbl_dts_;
    std::vector<cu_table_type> tbl_cus_;
    std::vector<od_table_type> tbl_ods_;
    std::vector<ol_table_type> tbl_ols_;
    std::vector<st_table_type> tbl_sts_;

    std::vector<ci_table_type> tbl_cni_;
    std::vector<oi_table_type> tbl_oci_;
    std::vector<no_table_type> tbl_nos_;
    std::vector<ht_table_type> tbl_hts_;

    tpcc_oid_generator oid_gen_;
    tpcc_delivery_queue dlvy_queue_;

    friend class tpcc_access<DBParams>;
};

template <typename DBParams>
class tpcc_runner {
public:
    static constexpr bool Commute = DBParams::Commute;
    enum class txn_type : int {
        new_order = 1,
        payment,
        order_status,
        delivery,
        stock_level
    };

    tpcc_runner(int id, tpcc_db<DBParams>& database, uint64_t w_start, uint64_t w_end, uint64_t w_own, int mix)
        : ig(id, database.num_warehouses()), db(database), mix(mix), runner_id(id),
          w_id_start(w_start), w_id_end(w_end), w_id_owned(w_own) {}

    inline txn_type next_transaction() {
        uint64_t x = ig.random(1, 100);
        if (mix == 0) {
            if (x <= 45)
                return txn_type::new_order;
            else if (x <= 88)
                return txn_type::payment;
            else if (x <= 92)
                return txn_type::order_status;
            else if (x <= 96)
                return txn_type::delivery;
            else
                return txn_type::stock_level;
        } else if (mix == 1) {
            return txn_type::new_order;
        }
        assert(mix == 2);
        if (x <= 51)
            return txn_type::new_order;
        else
            return txn_type::payment;
    }
    inline txn_type next_transaction2() {
        uint64_t x = ig.random(1, 100);
        if (x <= 51)
            return txn_type::new_order;
        else
            return txn_type::payment;
    }

    inline void run_txn_neworder(int did, worker_msg_t *work_msg);
    inline void run_txn_payment(int did, worker_msg_t *work_msg);
    inline void run_txn_orderstatus(int did, worker_msg_t *work_msg);
    // inline void run_txn_orderstatus();
    inline void run_txn_delivery(uint64_t wid,
        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE>& last_delivered);
    inline void run_txn_stocklevel(int did, worker_msg_t *work_msg);

    inline uint64_t owned_warehouse() const {
        return w_id_owned;
    }

private:
    tpcc_input_generator ig;
    tpcc_db<DBParams>& db;
    int mix;
    int runner_id;
    uint64_t w_id_start;
    uint64_t w_id_end;
    uint64_t w_id_owned;

    friend class tpcc_access<DBParams>;
};

template <typename DBParams>
class tpcc_prepopulator {
public:
    static pthread_barrier_t sync_barrier;

    tpcc_prepopulator(int id, tpcc_db<DBParams>& database)
        : ig(id, database.num_warehouses()), db(database), worker_id(id) {}

    inline void fill_items(uint64_t iid_begin, uint64_t iid_xend);
    inline void fill_warehouses();
    inline void expand_warehouse(uint64_t wid);
    inline void expand_districts(uint64_t wid);
    inline void expand_customers(uint64_t wid);

    inline void run();

private:
    inline std::string random_a_string(int x, int y);
    inline std::string random_n_string(int x, int y);
    inline std::string random_state_name();
    inline std::string random_zip_code();
    inline void random_shuffle(std::vector<uint64_t>& v);

    tpcc_input_generator ig;
    tpcc_db<DBParams>& db;
    int worker_id;
};

template <typename DBParams>
pthread_barrier_t tpcc_prepopulator<DBParams>::sync_barrier;


template <typename DBParams>
tpcc_db<DBParams>::tpcc_db(int num_whs)
    : num_whs_(num_whs),
      tbl_whs_(256),
      oid_gen_() {
    //constexpr size_t num_districts = NUM_DISTRICTS_PER_WAREHOUSE;
    //constexpr size_t num_customers = NUM_CUSTOMERS_PER_DISTRICT * NUM_DISTRICTS_PER_WAREHOUSE;

    tbl_its_ = new it_table_type(999983/*NUM_ITEMS * 2*/);
    for (auto i = 0; i < num_whs; ++i) {
        tbl_dts_.emplace_back(32/*num_districts * 2*/);
        tbl_cus_.emplace_back(999983/*num_customers * 2*/);
        tbl_ods_.emplace_back(999983/*num_customers * 10 * 2*/);
        tbl_ols_.emplace_back(999983/*num_customers * 100 * 2*/);
        tbl_sts_.emplace_back(999983/*NUM_ITEMS * 2*/);
        tbl_cni_.emplace_back(999983/*num_customers * 2*/);
        tbl_oci_.emplace_back(999983/*num_customers * 2*/);
        tbl_nos_.emplace_back(999983/*num_customers * 10 * 2*/);
        tbl_hts_.emplace_back(999983/*num_customers * 2*/);
    }
}

template <typename DBParams>
tpcc_db<DBParams>::~tpcc_db() {
    delete tbl_its_;
}

template <typename DBParams>
void tpcc_db<DBParams>::thread_init_all() {
    tbl_its_->thread_init();
    tbl_whs_.thread_init();
    for (auto& t : tbl_dts_)
        t.thread_init();
    for (auto& t : tbl_cus_)
        t.thread_init();
    for (auto& t : tbl_ods_)
        t.thread_init();
    for (auto& t : tbl_ols_)
        t.thread_init();
    for (auto& t : tbl_sts_)
        t.thread_init();
    for (auto& t : tbl_cni_)
        t.thread_init();
    for (auto& t : tbl_oci_)
        t.thread_init();
    for (auto& t : tbl_nos_)
        t.thread_init();
    for (auto& t : tbl_hts_)
        t.thread_init();
}

// @section: db prepopulation functions
template<typename DBParams>
void tpcc_prepopulator<DBParams>::fill_items(uint64_t iid_begin, uint64_t iid_xend) {
    for (auto iid = iid_begin; iid < iid_xend; ++iid) {
        item_key ik(iid);
        item_value iv;

        iv.i_im_id = ig.random(1, 10000);
        iv.i_price = ig.random(100, 10000);
        iv.i_name = random_a_string(14, 24);
        iv.i_data = random_a_string(26, 50);
        if (ig.random(1, 100) <= 10) {
            auto pos = ig.random(0, iv.i_data.length() - 8);
            auto placed = iv.i_data.place("ORIGINAL", pos);
            assert(placed);
            (void)placed;
        }

        db.tbl_items().nontrans_put(ik, iv);
    }
}

template<typename DBParams>
void tpcc_prepopulator<DBParams>::fill_warehouses() {
    for (uint64_t wid = 1; wid <= ig.num_warehouses(); ++wid) {
        warehouse_key wk(wid);
        warehouse_value wv {};
        wv.w_name = random_a_string(6, 10);
        wv.w_street_1 = random_a_string(10, 20);
        wv.w_street_2 = random_a_string(10, 20);
        wv.w_city = random_a_string(10, 20);
        wv.w_state = random_state_name();
        wv.w_zip = random_zip_code();
        wv.w_tax = ig.random(0, 2000);
        wv.w_ytd = 30000000;

        db.tbl_warehouses().nontrans_put(wk, wv);
    }
}

template<typename DBParams>
void tpcc_prepopulator<DBParams>::expand_warehouse(uint64_t wid) {
    for (uint64_t iid = 1; iid <= NUM_ITEMS; ++iid) {
        stock_key sk(wid, iid);
        stock_value sv;

        sv.s_quantity = ig.random(10, 100);
        for (auto i = 0; i < NUM_DISTRICTS_PER_WAREHOUSE; ++i)
            sv.s_dists[i] = random_a_string(24, 24);
        sv.s_ytd = 0;
        sv.s_order_cnt = 0;
        sv.s_remote_cnt = 0;
        sv.s_data = random_a_string(26, 50);
        if (ig.random(1, 100) <= 10) {
            auto pos = ig.random(0, sv.s_data.length() - 8);
            auto placed = sv.s_data.place("ORIGINAL", pos);
            assert(placed);
            (void)placed;
        }

        db.tbl_stocks(wid).nontrans_put(sk, sv);
    }

    for (uint64_t did = 1; did <= NUM_DISTRICTS_PER_WAREHOUSE; ++did) {
        district_key dk(wid, did);
        district_value dv;

        dv.d_name = random_a_string(6, 10);
        dv.d_street_1 = random_a_string(10, 20);
        dv.d_street_2 = random_a_string(10, 20);
        dv.d_city = random_a_string(10, 20);
        dv.d_state = random_state_name();
        dv.d_zip = random_zip_code();
        dv.d_tax = ig.random(0, 2000);
        dv.d_ytd = 3000000;
        dv.d_next_o_id = 3001;

        db.tbl_districts(wid).nontrans_put(dk, dv);
    }
}

template<typename DBParams>
void tpcc_prepopulator<DBParams>::expand_districts(uint64_t wid) {
    std::unordered_map<customer_idx_key, std::list<uint64_t>> cids_map;

    for (uint64_t did = 1; did <= NUM_DISTRICTS_PER_WAREHOUSE; ++did) {
        for (uint64_t cid = 1; cid <= NUM_CUSTOMERS_PER_DISTRICT; ++cid) {
            int last_name_num = (cid <= 1000) ? int(cid - 1)
                                              : ig.gen_customer_last_name_num(false/*run time*/);
            customer_key ck(wid, did, cid);
            customer_value cv;

            cv.c_last = ig.to_last_name(last_name_num);
            cv.c_middle = "OE";
            cv.c_first = random_a_string(8, 16);
            cv.c_street_1 = random_a_string(10, 20);
            cv.c_street_2 = random_a_string(10, 20);
            cv.c_city = random_a_string(10, 20);
            cv.c_zip = random_zip_code();
            cv.c_phone = random_n_string(16, 16);
            cv.c_since = ig.gen_date();
            cv.c_credit = (ig.random(1, 100) <= 10) ? "BC" : "GC";
            cv.c_credit_lim = 5000000;
            cv.c_discount = ig.random(0, 5000);
            cv.c_balance = -1000;
            cv.c_ytd_payment = 1000;
            cv.c_payment_cnt = 1;
            cv.c_delivery_cnt = 0;
            cv.c_data = random_a_string(300, 500);

            db.tbl_customers(wid).nontrans_put(ck, cv);

            customer_idx_key cik(wid, did, cv.c_last);
            cids_map[cik].push_front(cid);
        }
    }

    for (auto kv : cids_map) {
        customer_idx_value civ;
        civ.c_ids = kv.second;
        db.tbl_customer_index(wid).nontrans_put(kv.first, civ);
    }
}

template<typename DBParams>
void tpcc_prepopulator<DBParams>::expand_customers(uint64_t wid) {
    for (uint64_t did = 1; did <= NUM_DISTRICTS_PER_WAREHOUSE; ++did) {
        for (uint64_t cid = 1; cid <= NUM_CUSTOMERS_PER_DISTRICT; ++cid) {
            history_value hv;

            hv.h_c_id = cid;
            hv.h_c_d_id = did;
            hv.h_c_w_id = wid;
            hv.h_date = ig.gen_date();
            hv.h_amount = 1000;
            hv.h_data = random_a_string(12, 24);

#if HISTORY_SEQ_INSERT
            history_key hk(db.tbl_histories(wid).gen_key());
#else
            history_key hk(wid, did, cid, db.tbl_histories(wid).gen_key());
#endif
            db.tbl_histories(wid).nontrans_put(hk, hv);
        }
    }

    for (uint64_t did = 1; did <= NUM_DISTRICTS_PER_WAREHOUSE; ++did) {
        std::vector<uint64_t> cid_perm;
        for (uint64_t n = 1; n <= NUM_CUSTOMERS_PER_DISTRICT; ++n)
            cid_perm.push_back(n);
        random_shuffle(cid_perm);

        for (uint64_t i = 1; i <= NUM_CUSTOMERS_PER_DISTRICT; ++i) {
            uint64_t oid = i;
            order_key ok(wid, did, oid);
            auto ol_count = (uint32_t) ig.random(5, 15);
            auto entry_date = ig.gen_date();
            order_value ov;

            ov.o_c_id = cid_perm[i - 1];
            ov.o_carrier_id = (oid < 2101) ? ig.random(1, 10) : 0;
            ov.o_entry_d = entry_date;
            ov.o_ol_cnt = ol_count;
            ov.o_all_local = 1;

            order_cidx_key ock(wid, did, ov.o_c_id, oid);

            db.tbl_orders(wid).nontrans_put(ok, ov);
            db.tbl_order_customer_index(wid).nontrans_put(ock, {});

            for (uint64_t on = 1; on <= ol_count; ++on) {
                orderline_key olk(wid, did, oid, on);
                orderline_value olv;

                olv.ol_i_id = ig.random(1, 100000);
                olv.ol_supply_w_id = wid;
                olv.ol_delivery_d = (oid < 2101) ? entry_date : 0;
                olv.ol_quantity = 5;
                olv.ol_amount = (oid < 2101) ? 0 : (int) ig.random(1, 999999);
                olv.ol_dist_info = random_a_string(24, 24);

                db.tbl_orderlines(wid).nontrans_put(olk, olv);
            }

            if (oid >= 2101) {
                order_key nok(wid, did, oid);
                db.tbl_neworders(wid).nontrans_put(nok, {});
            }
        }
    }
}
// @endsection: db prepopulation functions

template<typename DBParams>
void tpcc_prepopulator<DBParams>::run() {
    int r;

    always_assert(worker_id >= 1, "prepopulator worker id range error");
    // set affinity so that the warehouse is filled at the corresponding numa node
    set_affinity(worker_id - 1);

    if (worker_id == 1) {
        fill_items(1, 100001);
        fill_warehouses();
    }

    // barrier
    r = pthread_barrier_wait(&sync_barrier);
    always_assert(r == PTHREAD_BARRIER_SERIAL_THREAD || r == 0, "pthread_barrier_wait");

    expand_warehouse((uint64_t) worker_id);

    // barrier
    r = pthread_barrier_wait(&sync_barrier);
    always_assert(r == PTHREAD_BARRIER_SERIAL_THREAD || r == 0, "pthread_barrier_wait");

    expand_districts((uint64_t) worker_id);

    // barrier
    r = pthread_barrier_wait(&sync_barrier);
    always_assert(r == PTHREAD_BARRIER_SERIAL_THREAD || r == 0, "pthread_barrier_wait");

    expand_customers((uint64_t) worker_id);
}

// @section: prepopulation string generators
template<typename DBParams>
std::string tpcc_prepopulator<DBParams>::random_a_string(int x, int y) {
    size_t len = ig.random(x, y);
    std::string str;
    str.reserve(len);

    for (auto i = 0u; i < len; ++i) {
        auto n = ig.random(0, 61);
        char c = (n < 26) ? char('a' + n) :
                 ((n < 52) ? char('A' + (n - 26)) : char('0' + (n - 52)));
        str.push_back(c);
    }
    return str;
}

template<typename DBParams>
std::string tpcc_prepopulator<DBParams>::random_n_string(int x, int y) {
    size_t len = ig.random(x, y);
    std::string str;
    str.reserve(len);

    for (auto i = 0u; i < len; ++i) {
        auto n = ig.random(0, 9);
        str.push_back(char('0' + n));
    }
    return str;
}

template<typename DBParams>
std::string tpcc_prepopulator<DBParams>::random_state_name() {
    std::string str = "AA";
    for (auto &c : str)
        c += ig.random(0, 25);
    return str;
}

template<typename DBParams>
std::string tpcc_prepopulator<DBParams>::random_zip_code() {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << ig.random(0, 9999);
    return ss.str() + "11111";
}

template<typename DBParams>
void tpcc_prepopulator<DBParams>::random_shuffle(std::vector<uint64_t> &v) {
    std::shuffle(v.begin(), v.end(), ig.random_generator());
}
// @endsection: prepopulation string generators

template <typename DBParams>
class tpcc_access {
public:
    static void prepopulation_worker(tpcc_db<DBParams> &db, int worker_id) {
        tpcc_prepopulator<DBParams> pop(worker_id, db);
        db.thread_init_all();
        pop.run();
    }

    static void prepopulate_db(tpcc_db<DBParams> &db) {
        int r;
        r = pthread_barrier_init(&tpcc_prepopulator<DBParams>::sync_barrier, nullptr, db.num_warehouses());
        always_assert(r == 0, "pthread_barrier_init failed");

        std::vector<std::thread> prepop_thrs;
        for (int i = 1; i <= db.num_warehouses(); ++i)
            prepop_thrs.emplace_back(prepopulation_worker, std::ref(db), i);
        for (auto &t : prepop_thrs)
            t.join();

        r = pthread_barrier_destroy(&tpcc_prepopulator<DBParams>::sync_barrier);
        always_assert(r == 0, "pthread_barrier_destroy failed");
    }

    static void tpcc_runner_thread_strife_hack(tpcc_db<DBParams>& db, db_profiler& prof, int runner_id, uint64_t w_start,
                                   uint64_t w_end, uint64_t w_own, double time_limit, int mix, uint64_t& txn_cnt){
        int did = runner_id % 10 + 1;
        tpcc_runner<DBParams> runner(runner_id, db, w_start, w_end, w_own, mix);
        typedef typename tpcc_runner<DBParams>::txn_type txn_type;

        uint64_t local_cnt = 0;

        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE> last_delivered;
        std::fill(last_delivered.begin(), last_delivered.end(), 0);

        ::TThread::set_id(runner_id);
        set_affinity(runner_id);
        db.thread_init_all();

        uint64_t tsc_diff = (uint64_t)(time_limit * constants::processor_tsc_frequency * constants::billion);
        auto start_t = prof.start_timestamp();

        /*new worker add 1*/
        strife_msg_t* worker_msg_ptr = NULL;
        auto own_w_id = runner.owned_warehouse();
        uint16_t slot_size = (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD + (MAX_THREAD + 2) - 1) / (MAX_THREAD+2);
        uint16_t slot_base = slot_size * runner_id;
        uint16_t slot_end = min(slot_base + slot_size, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
        //printf("%d %d %d\n",runner_id, slot_base, slot_end);
        
        if (own_w_id != 0){
            for (uint64_t q_d_id = 1; q_d_id <= 10; ++q_d_id) 
                last_delivered[q_d_id - 1] = 2100;
            // printf("delivery id = %d %d\n",runner_id, own_w_id);
        }
        strife_batch.init_thread(runner_id);
        performance.init_thread(runner_id);
        performance_listen.init_thread(runner_id);
        
        // if (runner_id < MAX_THREAD){
            worker_msg_ptr = new strife_msg_t();
            worker_msg_ptr->init(runner_id);
            worker_msg_ptr->init_strife(runner_id);
        // }
        
        while (true) {
            // if (runner_id == 0)
                // central_scheduler.run_strife_once(0, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD, runner_id); // recv and push
            
            central_scheduler.run_strife_once(slot_base, slot_end, runner_id); // recv and push
            // if (runner_id >= MAX_THREAD)
            //         continue;           
            // reply 
            // strife check next 
            worker_msg_ptr->count_strife(); // count;
            
            if (now_phase == 1 && strife_phase[runner_id][0] == 0){
                strife_phase[runner_id][0] = 1;
                // continue;
            }
            if (strife_phase[runner_id][0] == 1 && now_phase == 2){
                worker_msg_ptr->store_count();
                strife_phase[runner_id][0] = 2;
            }
            if (!(now_phase == 0 && strife_phase[runner_id][0] == 2)){
                continue;
            }
            // printf("runner_id=%d, worker_msg_ptr->COUNT%d \n", runner_id, worker_msg_ptr->batch_req_num);
            
            if (own_w_id != 0){
                // printf("deliver= %d %d\n",runner_id, own_w_id);
                auto num_to_run = db.delivery_queue().read(own_w_id);
                uint64_t num_run = 0;

                if (num_to_run > 0) {
                    // num_to_run = 1;
                    int real_run = 0;
                    for (num_run = 0; num_run < num_to_run; ++num_run) {
                        if (!tpcc_delivery.check(own_w_id-1, runner_id)){
                            break;
                        }
                        runner.run_txn_delivery(own_w_id, last_delivered);
                        tpcc_delivery.reply_now_delivery(own_w_id-1, runner_id);
                        real_run++;
                    }
                    db.delivery_queue().dequeue(own_w_id, real_run);
                    // continue;
                }
                // if (runner_id >= MAX_THREAD)
                //     continue;
            }
            

            if (worker_msg_ptr->batch_req_num == 0){
                strife_phase[runner_id][0] = 0;
                continue;
            }
            
            
            if (!worker_msg_ptr->next_txn_check_strife()) {
                continue;
            }
            // puts("???");

            txn_type t;

            // worker_msg_ptr->batch_req_num--;
            // worker_msg_ptr->reply_now_strife();
            // // worker_msg_ptr->next_now_strife();

            // continue;

            t = txn_type::new_order;
            if (worker_msg_ptr->now_msg.type == 0){ // /NetTx/NicTxn/tpcc_procedure.cpp
                t = txn_type::new_order;
            }
            else if(worker_msg_ptr->now_msg.type == 1){
                t = txn_type::payment;
            }
            else if(worker_msg_ptr->now_msg.type == 2){
                t = txn_type::order_status;
            }
            else if(worker_msg_ptr->now_msg.type == 3){
                t = txn_type::stock_level;
            }
            else if (worker_msg_ptr->now_msg.type == 4){
                t = txn_type::delivery;
            }

            //     t = txn_type::new_order;
            switch (t) {
                case txn_type::new_order:
                    runner.run_txn_neworder(did, worker_msg_ptr);
                    break;
                case txn_type::payment:
                    runner.run_txn_payment(did, worker_msg_ptr);
                    break;
                case txn_type::order_status:
                    runner.run_txn_orderstatus(did, worker_msg_ptr);
                    break;
                case txn_type::delivery: {
                    uint64_t q_w_id = worker_msg_ptr->now_msg.get_w_id_del();
                    // All warehouse delivery transactions are delegated to
                    // its "owner" thread. This is in line with the actual
                    // TPC-C spec with regard to deferred execution.
                    // Do not count enqueued transactions as executed.
                    db.delivery_queue().enqueue(q_w_id);
                    tpcc_delivery.push_new_delivery(q_w_id-1, worker_msg_ptr->now_msg.get_rpc_id());
                    --local_cnt;
                    break;
                }
                case txn_type::stock_level:
                    runner.run_txn_stocklevel(did, worker_msg_ptr);
                    break;
                default:
                    fprintf(stderr, "r:%d unknown txn type\n", runner_id);
                    assert(false);
                    break;
            };
            worker_msg_ptr->batch_req_num--;
            if (t != txn_type::delivery){
                worker_msg_ptr->reply_now_strife();
            }
            else {
                worker_msg_ptr->next_now_strife();
            }
            
            ++local_cnt;
        }

        txn_cnt = local_cnt;
    }

    static void static_hack(tpcc_db<DBParams>& db, db_profiler& prof, int runner_id, uint64_t w_start,
                                   uint64_t w_end, uint64_t w_own, double time_limit, int mix, uint64_t& txn_cnt){
        int did = runner_id % 10 + 1;
        tpcc_runner<DBParams> runner(runner_id, db, w_start, w_end, w_own, mix);
        typedef typename tpcc_runner<DBParams>::txn_type txn_type;

        uint64_t local_cnt = 0;

        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE> last_delivered;
        std::fill(last_delivered.begin(), last_delivered.end(), 0);

        // reserving CPU
        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE> last_delivered_multi[10];
        for (int i = 0; i < 10; i++)
            std::fill(last_delivered_multi[i].begin(), last_delivered_multi[i].end(), 0);

        ::TThread::set_id(runner_id);
        set_affinity(runner_id);
        db.thread_init_all();

        uint64_t tsc_diff = (uint64_t)(time_limit * constants::processor_tsc_frequency * constants::billion);
        auto start_t = prof.start_timestamp();

        /*new worker add 1*/
        strife_msg_t* worker_msg_ptr = NULL;
        auto own_w_id = runner.owned_warehouse();
        uint16_t slot_size = (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD + (MAX_THREAD + 2) - 1) / (MAX_THREAD+2);
        uint16_t slot_base = slot_size * runner_id;
        uint16_t slot_end = min(slot_base + slot_size, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
        
        if (own_w_id != 0){
            for (uint64_t q_d_id = 1; q_d_id <= 10; ++q_d_id) 
                last_delivered[q_d_id - 1] = 2100;
            for (int i = 0; i < 10; i++)
                for (uint64_t q_d_id = 1; q_d_id <= 10; ++q_d_id) 
                    last_delivered_multi[i][q_d_id - 1] = 2100;
        }


        strife_batch.init_thread(runner_id);
        performance.init_thread(runner_id);
        performance_listen.init_thread(runner_id);

        
        worker_msg_ptr = new strife_msg_t();
        worker_msg_ptr->init(runner_id);
        worker_msg_ptr->init_strife(runner_id);

        int x = 0;
        while (true) {
        #if (CPUBASED!=2)
            central_scheduler.run_strife_once(slot_base, slot_end, runner_id); // recv and push
        #endif
            if (!worker_msg_ptr->next_txn_check_strife()) {
                if (true){ // no reserve
                    if (own_w_id != 0){
                    // if (runner_id > MAX_THREAD)
                        // printf("deliver= %d %d\n",runner_id, own_w_id);
                        auto num_to_run = db.delivery_queue().read(own_w_id);
                        uint64_t num_run = 0;
                        if (num_to_run > 0) {
                            // num_to_run = 1;
                            int real_run = 0;
                            for (num_run = 0; num_run < num_to_run; ++num_run) {
                                if (!tpcc_delivery.check(own_w_id - 1, runner_id)){
                                    break;
                                }
                                runner.run_txn_delivery(own_w_id, last_delivered);
                                tpcc_delivery.reply_now_delivery(own_w_id-1, runner_id);
                                real_run++;
                            }
                            db.delivery_queue().dequeue(own_w_id, real_run);
                            continue;
                        }
                    }
                }
                continue;
            }
            // puts("???");

            txn_type t;

            t = txn_type::new_order;
            if (worker_msg_ptr->now_msg.type == 0){ // /NetTx/NicTxn/tpcc_procedure.cpp
                t = txn_type::new_order;
            }
            else if(worker_msg_ptr->now_msg.type == 1){
                t = txn_type::payment;
            }
            else if(worker_msg_ptr->now_msg.type == 2){
                t = txn_type::order_status;
            }
            else if(worker_msg_ptr->now_msg.type == 3){
                t = txn_type::stock_level;
            }
            else if (worker_msg_ptr->now_msg.type == 4){
                t = txn_type::delivery;
            }

            bool delivery_flag = false;
            switch (t) {
                case txn_type::new_order:
                    runner.run_txn_neworder(did, worker_msg_ptr);
                    break;
                case txn_type::payment:
                    runner.run_txn_payment(did, worker_msg_ptr);
                    break;
                case txn_type::order_status:
                    runner.run_txn_orderstatus(did, worker_msg_ptr);
                    break;
                case txn_type::delivery: {
                    uint64_t q_w_id = worker_msg_ptr->now_msg.get_w_id_del();
                    delivery_flag = true;
                    tpcc_delivery.push_new_delivery(q_w_id-1, worker_msg_ptr->now_msg.get_rpc_id());
                    db.delivery_queue().enqueue(q_w_id);
                    --local_cnt;
                    break;
                }
                case txn_type::stock_level:
                    runner.run_txn_stocklevel(did, worker_msg_ptr);
                    break;
                default:
                    fprintf(stderr, "r:%d unknown txn type\n", runner_id);
                    assert(false);
                    break;
            };
            worker_msg_ptr->reply_now_partition(delivery_flag);
            
            ++local_cnt;
        }

        txn_cnt = local_cnt;
    }

    static void tpcc_runner_thread(tpcc_db<DBParams>& db, db_profiler& prof, int runner_id, uint64_t w_start,
                                   uint64_t w_end, uint64_t w_own, double time_limit, int mix, uint64_t& txn_cnt) {
        #if (CPUBASED==4)
            static_hack(db, prof, runner_id, w_start, w_end, w_own, time_limit, mix, txn_cnt);
            return;
        #endif

        #if (CPUBASED==1)
            static_hack(db, prof, runner_id, w_start, w_end, w_own, time_limit, mix, txn_cnt);
            return;
        #endif

        #if (CPUBASED==2)
            static_hack(db, prof, runner_id, w_start, w_end, w_own, time_limit, mix, txn_cnt);
            return;
        #endif
        
        #ifdef STRIFE
        tpcc_runner_thread_strife_hack(db, prof, runner_id, w_start, w_end, w_own, time_limit, mix, txn_cnt);
        return;
        #endif

        
        int did = runner_id % 10 + 1;
        tpcc_runner<DBParams> runner(runner_id, db, w_start, w_end, w_own, mix);
        typedef typename tpcc_runner<DBParams>::txn_type txn_type;

        uint64_t local_cnt = 0;

        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE> last_delivered;
        std::fill(last_delivered.begin(), last_delivered.end(), 0);

        std::array<uint64_t, NUM_DISTRICTS_PER_WAREHOUSE> last_delivered_multi[10];
        for (int i = 0; i < 10; i++)
            std::fill(last_delivered_multi[i].begin(), last_delivered_multi[i].end(), 0);

        auto own_w_id = runner.owned_warehouse();
        ::TThread::set_id(runner_id);
        set_affinity(runner_id);
        db.thread_init_all();

        uint64_t tsc_diff = (uint64_t)(time_limit * constants::processor_tsc_frequency * constants::billion);
        auto start_t = prof.start_timestamp();

        /*new worker add 1*/
        worker_msg_t worker_msg;
        

        #if (CPUBASED==1)
        uint16_t slot_size = (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD + MAX_THREAD - 1) / MAX_THREAD;
        uint16_t slot_base = slot_size * runner_id;
        uint16_t slot_end = min(slot_base + slot_size, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
        //printf("%d %d %d\n",runner_id, slot_base, slot_end);
        #endif
        
        fflush(stdout);
        performance.init_thread(runner_id);
        performance_listen.init_thread(runner_id);
        if (own_w_id != 0){
            for (uint64_t q_d_id = 1; q_d_id <= 10; ++q_d_id) 
                last_delivered[q_d_id - 1] = 2100;
            for (int i = 0; i < 10; i++)
                for (uint64_t q_d_id = 1; q_d_id <= 10; ++q_d_id) 
                    last_delivered_multi[i][q_d_id - 1] = 2100;
        }
        else {
            worker_msg.init(runner_id);
        }
        while (true) {
            // Executed enqueued delivery transactions, if any
            // network;
            #if (CPUBASED==1)
                central_scheduler.run_once(slot_base, slot_end, runner_id);
            // runner_thrs.emplace_back(&central_scheduler_t::run, &central_scheduler, 0, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD/2, 0);
            #endif
           if (own_w_id != 0){
                // printf("%d\n",runner_id, own_w_id);
                for (int o_w_id = own_w_id; o_w_id <= db.num_warehouses(); o_w_id+=2){
                    auto num_to_run = db.delivery_queue().read(o_w_id);
                    uint64_t num_run = 0;
                    if (num_to_run > 0) {
                        int real_run = 0;
                        for (num_run = 0; num_run < num_to_run; ++num_run) {
                            if (!tpcc_delivery.check(o_w_id-1, runner_id)){
                                continue;
                            }
                            runner.run_txn_delivery(o_w_id, last_delivered_multi[(o_w_id-1)/2]);
                            tpcc_delivery.reply_now_delivery(o_w_id-1, runner_id);
                            real_run++;
                        }
                        db.delivery_queue().dequeue(o_w_id, real_run);
                        continue;
                    }
                }
                continue;
                
           }
           
#ifndef QUICKCHECK
            if (!worker_msg.next_txn_check()) {
                continue;
            }
#endif
            

            auto curr_t = read_tsc();

//             if ((curr_t - start_t) >= tsc_diff){

// #ifndef QUICKCHECK
//                 worker_msg.reply_now();
// #endif

//                 break;
//             }
            // worker_msg.reply_now();
            // continue;
            txn_type t;
#ifdef QUICKCHECK
            t = runner.next_transaction2();
#else
            t = txn_type::new_order;
            if (worker_msg.now_msg.type == 0){ // /NetTx/NicTxn/tpcc_procedure.cpp
                t = txn_type::new_order;
            }
            else if(worker_msg.now_msg.type == 1){
                t = txn_type::payment;
            }
            else if(worker_msg.now_msg.type == 2){
                t = txn_type::order_status;
            }
            else if(worker_msg.now_msg.type == 3){
                t = txn_type::stock_level;
            }
            else if (worker_msg.now_msg.type == 4){
                t = txn_type::delivery;
            }
#endif
            //     t = txn_type::new_order;
            bool delivery_flag = false;
            switch (t) {
                case txn_type::new_order:
                    runner.run_txn_neworder(did, &worker_msg);
                    break;
                case txn_type::payment:
                    runner.run_txn_payment(did, &worker_msg);
                    break;
                case txn_type::order_status:
                    runner.run_txn_orderstatus(did, &worker_msg);
                    break;
                case txn_type::delivery: {
                    uint64_t q_w_id = worker_msg.now_msg.get_w_id_del();
                    // All warehouse delivery transactions are delegated to
                    // its "owner" thread. This is in line with the actual
                    // TPC-C spec with regard to deferred execution.
                    // Do not count enqueued transactions as executed.
                    db.delivery_queue().enqueue(q_w_id);
                    delivery_flag = true;
                    tpcc_delivery.push_new_delivery(q_w_id-1, worker_msg.now_msg.get_rpc_id());
                    --local_cnt;
                    break;
                }
                case txn_type::stock_level:
                    runner.run_txn_stocklevel(did, &worker_msg);
                    break;
                default:
                    fprintf(stderr, "r:%d unknown txn type\n", runner_id);
                    assert(false);
                    break;
            };

#ifndef QUICKCHECK
            worker_msg.reply_now(delivery_flag);
#else
            worker_msg.count_count_only();
#endif

            ++local_cnt;

        }

        txn_cnt = local_cnt;
    }
    static void thread_perf_func()
    {
        // set_affinity(21);
        int cnt = 0;
        while (cnt < 10) {
            usleep(10000);
            if (performance.try_print_throughput()) {
                #ifndef STRIFE
                if (cnt == 2){
                    // puts("begin scheduling");
                    model_update.merge_weights(true);
                }
                #endif
                #if (CPUBASED == 1)
                if (cnt == 2){
                    // puts("begin scheduling");
                    model_update.merge_weights(true);
                }
                #endif

                #if (CPUBASED == 2)
                if (cnt == 2){
                    // puts("begin scheduling");
                    model_update.merge_weights(true);
                }
                #endif
                cnt++;
            }
        }
        exit(0);
        return;
    }
    static uint64_t run_benchmark(tpcc_db<DBParams>& db, db_profiler& prof, int num_runners,
                                  double time_limit, int mix, const bool verbose) {
        // puts("????");
        int q = db.num_warehouses() / num_runners;
        int r = db.num_warehouses() % num_runners;
        std::vector<std::thread> runner_thrs;
        std::vector<uint64_t> txn_cnts(size_t(num_runners), 0);

        int nwh = db.num_warehouses();
        auto calc_own_w_id = [nwh](int rid) {
            return (rid >= nwh) ? 0 : (rid + 1);
        };

        #ifdef CPUBASED
        puts("init central_scheduler");
        central_scheduler.cpu_alnico_init(control_mm, host_mm,
                                      host_mm + FEATURE_SIZE, MAX_THREAD);

        #endif 

        #ifdef STRIFE
        central_scheduler.init_strife((uint64_t)strife_dispatch_queue, MAX_THREAD + 2);
        #endif

        tpcc_delivery.init(db.num_warehouses());


        // ProfilerStart(("/tmp/tpcc" + to_string(0) + ".prof").c_str());
        int reserver = MAX_THREAD;
        // int reserver = num_runners;
        if (true) {
            // puts("ok22");
            q = (num_runners + db.num_warehouses() - 1) / db.num_warehouses();
            int qq = q;
            int wid = 1;
            #if  (CPUBASED==1) || (CPUBASED==2) || (!defined(CPUBASED))
            for (int i = 0; i < num_runners + 2; ++i) { // worker_set
                if ((qq--) == 0) {
                    ++wid;
                    qq += q;
                }
                if (verbose) {
                    fprintf(stdout, "runner %d: [%d, %d], own: %d\n", i, wid, wid, calc_own_w_id(i));
                }
                if (i < reserver){
                    // printf("new thread %d %d %d\n", i, wid, 0);
                    runner_thrs.emplace_back(tpcc_runner_thread, std::ref(db), std::ref(prof),
                                         i, wid, wid, 0, time_limit, mix, std::ref(txn_cnts[i]));
                
                }
                else {
                    runner_thrs.emplace_back(tpcc_runner_thread, std::ref(db), std::ref(prof),
                                         i, wid, wid, i - reserver + 1, time_limit, mix, std::ref(txn_cnts[i]));
                }
            }
            #else
            for (int i = 0; i < num_runners + 2; ++i) { // worker_set
                if ((qq--) == 0) {
                    ++wid;
                    qq += q;
                }
                if (i < num_runners){
                    int x = 0;
                    if (i < db.num_warehouses()){
                        x = i + 1;
                    }
                    // printf("cpu new thread %d %d %d\n", i, wid, x);
                    runner_thrs.emplace_back(tpcc_runner_thread, std::ref(db), std::ref(prof),
                                         i, wid, wid, x, time_limit, mix, std::ref(txn_cnts[i]));
                }
                else {
                    int x = 0;
                    // x = i - num_runners + 1;
                    runner_thrs.emplace_back(tpcc_runner_thread, std::ref(db), std::ref(prof),
                                         i, wid, wid, x, time_limit, mix, std::ref(txn_cnts[i]));
                }
            }
            #endif
        } else {
            puts("what22");
            int last_xend = 1;

            for (int i = 0; i < num_runners; ++i) {
                int next_xend = last_xend + q;
                if (r > 0) {
                    ++next_xend;
                    --r;
                }
                if (verbose) {
                    fprintf(stdout, "runner %d: [%d, %d], own: %d\n", i, last_xend, next_xend - 1, calc_own_w_id(i));
                }
                runner_thrs.emplace_back(tpcc_runner_thread, std::ref(db), std::ref(prof),
                                         i, last_xend, next_xend - 1, calc_own_w_id(i), time_limit, mix,
                                         std::ref(txn_cnts[i]));
                last_xend = next_xend;
            }

            always_assert(last_xend == db.num_warehouses() + 1, "warehouse distribution error");
        }
        #if (CPUBASED==2)
        runner_thrs.emplace_back(&central_scheduler_t::run, &central_scheduler, 0, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD/2, 0);
        runner_thrs.emplace_back(&central_scheduler_t::run, &central_scheduler, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD/2, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD, 1);
        #endif 

        runner_thrs.emplace_back(reply_write);
        // runner_thrs.emplace_back(reply_write_dual, 0);
        // runner_thrs.emplace_back(reply_write_dual, 1);
        // central_scheduler.run(0, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
        
        sleep(1);
        thread_perf_func();

        for (auto &t : runner_thrs)
            t.join();

        uint64_t total_txn_cnt = 0;
        for (auto& cnt : txn_cnts)
            total_txn_cnt += cnt;
        return total_txn_cnt;
    }
    // HEAR EXECUTE
    static int execute(int argc, const char *const *argv) {
        std::cout << "*** DBParams::Id = " << DBParams::Id << std::endl;
        std::cout << "*** DBParams::Commute = " << std::boolalpha << DBParams::Commute << std::endl;
        int ret = 0;

        bool spawn_perf = false;
        bool counter_mode = false;
        int num_warehouses = 1;
        int num_threads = 1;
        int mix = 0;
        double time_limit = 80.0;
        bool enable_gc = false;
        unsigned gc_rate = Transaction::get_epoch_cycle();
        bool verbose = false;

        Clp_Parser *clp = Clp_NewParser(argc, argv, noptions, options);

        int opt;
        bool clp_stop = false;
        while (!clp_stop && ((opt = Clp_Next(clp)) != Clp_Done)) {
            switch (opt) {
                case opt_dbid:
                    break;
                case opt_nwhs:
                    num_warehouses = clp->val.i;
                    break;
                case opt_nthrs:
                    num_threads = clp->val.i;
                    break;
                case opt_time:
                    time_limit = clp->val.d;
                    break;
                case opt_perf:
                    spawn_perf = !clp->negated;
                    break;
                case opt_pfcnt:
                    counter_mode = !clp->negated;
                    break;
                case opt_gc:
                    enable_gc = !clp->negated;
                    break;
                case opt_gr:
                    gc_rate = clp->val.i;
                    break;
                case opt_node:
                    break;
                case opt_comm:
                    break;
                case opt_verb:
                    verbose = !clp->negated;
                    break;
                case opt_mix:
                    mix = clp->val.i;
                    if (mix > 2 || mix < 0) {
                        mix = 0;
                    }
                    break;
                default:
                    ::print_usage(argv[0]);
                    ret = 1;
                    clp_stop = true;
                    break;
            }
        }

        Clp_DeleteParser(clp);
        if (ret != 0)
            return ret;

        std::cout << "Selected workload mix: " << std::string(workload_mix_names[mix]) << std::endl;

        auto profiler_mode = counter_mode ?
                             Profiler::perf_mode::counters : Profiler::perf_mode::record;

        if (counter_mode && !spawn_perf) {
            // turns on profiling automatically if perf counters are requested
            spawn_perf = true;
        }

        if (spawn_perf) {
            std::cout << "Info: Spawning perf profiler in "
                      << (counter_mode ? "counter" : "record") << " mode" << std::endl;
        }

        db_profiler prof(spawn_perf);
        tpcc_db<DBParams> db(num_warehouses);

        std::cout << "Prepopulating database..." << std::endl;
        prepopulate_db(db);
        std::cout << "Prepopulation complete." << std::endl;

        std::thread advancer;
        std::cout << "Garbage collection-: ";
        if (enable_gc) {
            std::cout << "enabled, running every "<< gc_rate / 1000.0 << " ms";
            Transaction::set_epoch_cycle(gc_rate);
            advancer = std::thread(&Transaction::epoch_advancer, nullptr);
        } else {
            std::cout << "disabled";
        }
        std::cout << std::endl << std::flush;
        // puts("????");
        sleep(1);
        prof.start(profiler_mode);
        auto num_trans = run_benchmark(db, prof, num_threads, time_limit, mix, verbose);
        prof.finish(num_trans);


        

        size_t remaining_deliveries = 0;
        for (int wh = 1; wh <= db.num_warehouses(); wh++) {
            remaining_deliveries += db.delivery_queue().read(wh);
        }
        std::cout << "Remaining unresolved deliveries: " << remaining_deliveries << std::endl;

        Transaction::rcu_release_all(advancer, num_threads);
        sleep(10);
        return 0;
    }
}; // class tpcc_access

};
