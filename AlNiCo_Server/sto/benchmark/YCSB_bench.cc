#include "YCSB_bench.hh"

#include <iostream>
#include <sstream>
#include <thread>

#include "DB_profiler.hh"
#include "PlatformFeatures.hh"
#include "Rpc.h"
#include "YCSB_txns.hh"
#include "fpga.h"

namespace ycsb
{

  using namespace db_params;
  using bench::db_profiler;

  enum
  {
    opt_dbid = 1,
    opt_nthrs,
    opt_mode,
    opt_time,
    opt_perf,
    opt_pfcnt,
    opt_gc,
    opt_node,
    opt_comm
  };

  static const Clp_Option options[] = {
      {"dbid", 'i', opt_dbid, Clp_ValString, Clp_Optional},
      {"nthreads", 't', opt_nthrs, Clp_ValInt, Clp_Optional},
      {"mode", 'm', opt_mode, Clp_ValString, Clp_Optional},
      {"time", 'l', opt_time, Clp_ValDouble, Clp_Optional},
      {"perf", 'p', opt_perf, Clp_NoVal, Clp_Optional},
      {"perf-counter", 'c', opt_pfcnt, Clp_NoVal, Clp_Negate | Clp_Optional},
      {"gc", 'g', opt_gc, Clp_NoVal, Clp_Negate | Clp_Optional},
      {"node", 'n', opt_node, Clp_NoVal, Clp_Negate | Clp_Optional},
      {"commute", 'x', opt_comm, Clp_NoVal, Clp_Negate | Clp_Optional},
  };

  static inline void print_usage(const char *argv_0)
  {
    std::stringstream ss;
    ss << "Usage of " << std::string(argv_0) << ":" << std::endl
       << "  --dbid=<STRING> (or -i<STRING>)" << std::endl
       << "    Specify the type of DB concurrency control used. Can be one of "
          "the followings:"
       << std::endl
       << "      default, opaque, 2pl, adaptive, swiss, tictoc, defaultnode, "
          "mvcc, mvccnode"
       << std::endl
       << "  --nthreads=<NUM> (or -t<NUM>)" << std::endl
       << "    Specify the number of threads (or TPCC workers/terminals, default "
          "1)."
       << std::endl
       << "  --mode=<CHAR> (or -m<CHAR>)" << std::endl
       << "    Specify which YCSB variant to run (A/B/C, default C)." << std::endl
       << "  --time=<NUM> (or -l<NUM>)" << std::endl
       << "    Specify the time (duration) for which the benchmark is run "
          "(default 10 seconds)."
       << std::endl
       << "  --perf (or -p)" << std::endl
       << "    Spawns perf profiler in record mode for the duration of the "
          "benchmark run."
       << std::endl
       << "  --perf-counter (or -c)" << std::endl
       << "    Spawns perf profiler in counter mode for the duration of the "
          "benchmark run."
       << std::endl
       << "  --gc (or -g)" << std::endl
       << "    Enable garbage collection (default false)." << std::endl
       << "  --node (or -n)" << std::endl
       << "    Enable node tracking (default false)." << std::endl
       << "  --commute (or -x)" << std::endl
       << "    Enable commutative updates in MVCC (default false)." << std::endl;
    std::cout << ss.str() << std::flush;
  }

  template <typename DBParams>
  void ycsb_prepopulation_thread(int thread_id, ycsb_db<DBParams> &db,
                                 uint64_t key_begin, uint64_t key_end)
  {
    set_affinity(thread_id);
    ycsb_input_generator ig(thread_id);
    db.table_thread_init();
    for (uint64_t i = key_begin; i < key_end; ++i)
    {
#if TPCC_SPLIT_TABLE
      db.ycsb_half_tables(true).nontrans_put(
          ycsb_key(i), ig.random_ycsb_value<ycsb_half_value>());
      db.ycsb_half_tables(false).nontrans_put(
          ycsb_key(i), ig.random_ycsb_value<ycsb_half_value>());
#else
      db.ycsb_table().nontrans_put(ycsb_key(i),
                                   ig.random_ycsb_value<ycsb_value>());
#endif
    }
  }

  template <typename DBParams>
  void ycsb_db<DBParams>::prepopulate()
  {
    static constexpr uint64_t nthreads = 32;
    uint64_t key_begin, key_end;
    uint64_t segment_size = ycsb_table_size / nthreads;
    key_begin = 0;
    key_end = segment_size;

    std::vector<std::thread> prepopulators;

    for (uint64_t tid = 0; tid < nthreads; ++tid)
    {
      prepopulators.emplace_back(ycsb_prepopulation_thread<DBParams>, (int)tid,
                                 std::ref(*this), key_begin, key_end);
      key_begin += segment_size;
      key_end += segment_size;
    }

    for (auto &t : prepopulators)
      t.join();
  }
  // YCSB HEAR`
  template <typename DBParams>
  class ycsb_access
  {
  public:
    static void ycsb_static_hack(ycsb_db<DBParams> &db, db_profiler &prof,
                                 ycsb_runner<DBParams> &runner, double time_limit,
                                 uint64_t &txn_cnt)
    {
      uint64_t local_cnt = 0;
      db.table_thread_init();

      ::TThread::set_id(runner.id());
      set_affinity(runner.id());

      uint64_t tsc_diff =
          (uint64_t)(time_limit * constants::processor_tsc_frequency *
                     constants::billion);
      auto start_t = prof.start_timestamp();

      /*new worker add 1*/
      strife_msg_t *worker_msg_ptr = NULL;
      // uint16_t slot_size =
      //     (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD + (MAX_THREAD)-1) / (MAX_THREAD);
      // uint16_t slot_base = min(slot_size * runner.id(), MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
      // uint16_t slot_end = min(slot_base + slot_size, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
      // printf("ycsb_static_manager = %d %d %d\n", runner.id(), slot_base, slot_end);

      uint16_t slot_size =
          (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD) / (MAX_THREAD);
      uint16_t left = MAX_CLIENT * SLOT_NUM * CLIENT_THREAD - slot_size * MAX_THREAD;
      uint16_t slot_base = min(slot_size * runner.id() + min(left, runner.id()), MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
      uint16_t slot_end = min(slot_base + slot_size + (runner.id() < left), MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);

      strife_batch.init_thread(runner.id());
      performance.init_thread(runner.id());
      performance_listen.init_thread(runner.id());

      worker_msg_ptr = new strife_msg_t();
      worker_msg_ptr->init(runner.id());
      worker_msg_ptr->init_strife(runner.id());

      ycsb_txn_t it;
      it.ops.reserve(YCSBSIZE);

      while (true)
      {
        central_scheduler.run_strife_once(slot_base, slot_end,
                                          runner.id()); // recv and push

        if (!worker_msg_ptr->next_txn_check_strife())
        {
          continue;
        }
        runner.msg_next_tx(&it, worker_msg_ptr);

        runner.run_txn(&it, worker_msg_ptr);
        // runner.run_txn(&(*itt));
        ++local_cnt;
        worker_msg_ptr->reply_now_partition(false);
      }

      txn_cnt = local_cnt;
    }
    static void ycsb_runner_thread_strife_hack(ycsb_db<DBParams> &db,
                                               db_profiler &prof,
                                               ycsb_runner<DBParams> &runner,
                                               double time_limit,
                                               uint64_t &txn_cnt)
    {
      uint64_t local_cnt = 0;
      db.table_thread_init();

      ::TThread::set_id(runner.id());
      set_affinity(runner.id());

      uint64_t tsc_diff =
          (uint64_t)(time_limit * constants::processor_tsc_frequency *
                     constants::billion);
      auto start_t = prof.start_timestamp();

      /*new worker add 1*/
      strife_msg_t *worker_msg_ptr = NULL;
      uint16_t slot_size =
          (MAX_CLIENT * SLOT_NUM * CLIENT_THREAD + (MAX_THREAD)-1) / (MAX_THREAD);
      uint16_t slot_base = slot_size * runner.id();
      uint16_t slot_end =
          min(slot_base + slot_size, MAX_CLIENT * SLOT_NUM * CLIENT_THREAD);
      // printf("%d %d %d\n", runner.id(), slot_base, slot_end);

      strife_batch.init_thread(runner.id());
      performance.init_thread(runner.id());
      performance_listen.init_thread(runner.id());

      worker_msg_ptr = new strife_msg_t();
      worker_msg_ptr->init(runner.id());
      worker_msg_ptr->init_strife(runner.id());

      ycsb_txn_t it;
      it.ops.reserve(YCSBSIZE);

      while (true)
      {
        central_scheduler.run_strife_once(slot_base, slot_end,
                                          runner.id()); // recv and push

        worker_msg_ptr->count_strife();

        if (now_phase == 1 && strife_phase[runner.id()][0] == 0)
        {
          strife_phase[runner.id()][0] = 1;
          // continue;
        }
        if (strife_phase[runner.id()][0] == 1 && now_phase == 2)
        {
          worker_msg_ptr->store_count();
          strife_phase[runner.id()][0] = 2;
        }
        if (!(now_phase == 0 && strife_phase[runner.id()][0] == 2))
        {
          continue;
        }
        if (worker_msg_ptr->batch_req_num == 0)
        {
          strife_phase[runner.id()][0] = 0;
          continue;
        }

        if (!worker_msg_ptr->next_txn_check_strife())
        {
          continue;
        }
        runner.msg_next_tx(&it, worker_msg_ptr);

        runner.run_txn(&it, worker_msg_ptr);
        // runner.run_txn(&(*itt));
        worker_msg_ptr->batch_req_num--;
        worker_msg_ptr->reply_now_strife();
        ++local_cnt;
      }

      txn_cnt = local_cnt;
    }
    static void ycsb_runner_thread(ycsb_db<DBParams> &db, db_profiler &prof,
                                   ycsb_runner<DBParams> &runner,
                                   double time_limit, uint64_t &txn_cnt)
    {
#if (CPUBASED == 4)
      ycsb_static_hack(db, prof, runner, time_limit, txn_cnt);
      return;
#endif

#ifdef STRIFE
      ycsb_runner_thread_strife_hack(db, prof, runner, time_limit, txn_cnt);
      return;
#endif
      uint64_t local_cnt = 0;
      db.table_thread_init();

      ::TThread::set_id(runner.id());
      set_affinity(runner.id());

      uint64_t tsc_diff =
          (uint64_t)(time_limit * constants::processor_tsc_frequency *
                     constants::billion);
      auto start_t = prof.start_timestamp();

      // ycsb_txn_t txn {};
      // txn.ops.reserve(txn_size);
      // std::set<uint32_t> key_set;
      // for (int j = 0; j < txn_size; ++j) {
      //     uint32_t key;
      //     do {
      //         key = dd->sample();
      //     } while (key_set.find(key) != key_set.end());
      //     key_set.insert(key);
      // }
      // bool any_write = false;
      // for (auto it = key_set.begin(); it != key_set.end(); ++it) {
      //     bool is_write = ud->sample() < write_threshold;
      //     ycsb_op_t op {};
      //     op.is_write = is_write;
      //     op.key = *it;
      //     op.col_n = ud->sample() % (2*HALF_NUM_COLUMNS); /*column number*/
      //     if (is_write) {
      //         any_write = true;
      //         ig.random_ycsb_col_value_inplace(&op.write_value);
      //     }
      //     txn.ops.push_back(std::move(op));
      // }
      // txn.rw_txn = any_write;

      /*new worker add 1*/
      worker_msg_t worker_msg;
      worker_msg.init(runner.id());
      performance.init_thread(runner.id());
      performance_listen.init_thread(runner.id());
      ycsb_txn_t it;
      it.ops.reserve(YCSBSIZE);

#ifdef QUICKCHECK
      runner.init();
      auto itt = runner.workload.begin();
#endif

      while (true)
      {
#ifdef QUICKCHECK
        // runner.next_tx(&it, 16);
        ++itt;
        if (itt == runner.workload.end())
          itt = runner.workload.begin();
#else
        if (!worker_msg.next_txn_check())
        {
          continue;
        }
        runner.msg_next_tx(&it, &worker_msg);
        // worker_msg.reply_now();
        // continue;
#endif

        runner.run_txn(&it, &worker_msg);
        // runner.run_txn(&(*itt));

        ++local_cnt;
#ifdef QUICKCHECK
        worker_msg.count_count_only();
#else
        worker_msg.reply_now();
#endif
      }

      txn_cnt = local_cnt;
    }
    static void thread_perf_func()
    {
      // set_affinity(21);
      int cnt = 0;
#ifdef DYNAMIC
      while (cnt < 15)
      {
#else
      while (cnt < 10)
      {
#endif
        usleep(5);

#ifdef DYNAMIC
        if (performance.try_print_throughput_null()) // time gap = 2
        {
#else
        if (performance.try_print_throughput())
        { /*1DYNAMIC +_null*/
#endif
          if (cnt == 2)
          {
            // puts("begin scheduling");
            model_update.merge_weights(true);
          }
          else if (cnt > 2)
          {
            performance.update_dynamic_phase((cnt - 3));
          }
          cnt++;
        }
/*1DYNAMIC*/
// /*
#if defined(DYNAMIC) && (!defined(CPUBASED))
        if (cnt >= 3 && performance.check_throughput())
        {
          model_update.merge_weights(false);
          // model_update.print();
        }
#endif
        // */
      }
      exit(0);
      return;
    }

    static void workload_generation(std::vector<ycsb_runner<DBParams>> &runners,
                                    mode_id mode)
    {
      std::vector<std::thread> thrs;
      int tsize = (mode == mode_id::ReadOnly) ? 2 : 64;
      for (auto &r : runners)
      {
        thrs.emplace_back(&ycsb_runner<DBParams>::gen_workload, &r, tsize);
      }

      for (auto &t : thrs)
        t.join();
    }

    static uint64_t run_benchmark(ycsb_db<DBParams> &db, db_profiler &prof,
                                  std::vector<ycsb_runner<DBParams>> &runners,
                                  double time_limit)
    {
      int num_runners = runners.size();
      std::vector<std::thread> runner_thrs;
      std::vector<uint64_t> txn_cnts(size_t(num_runners), 0);

#ifdef CPUBASED
      // puts("????????");
      // central_scheduler.init(control_mm, host_mm, host_mm + FEATURE_SIZE,
      // MAX_THREAD);
      central_scheduler.cpu_alnico_init(control_mm, host_mm,
                                        host_mm + FEATURE_SIZE, MAX_THREAD);

#endif

#ifdef STRIFE
      central_scheduler.init_strife((uint64_t)strife_dispatch_queue,
                                    MAX_THREAD + 2);
#endif

      for (int i = 0; i < num_runners; ++i)
      {
        runner_thrs.emplace_back(ycsb_runner_thread, std::ref(db), std::ref(prof),
                                 std::ref(runners[i]), time_limit,
                                 std::ref(txn_cnts[i]));
      }

      sleep(1);
      // DYNAMIC !!!!
      runner_thrs.emplace_back(reply_write);
// runner_thrs.emplace_back(reply_write_dual, 0);
// runner_thrs.emplace_back(reply_write_dual, 1);

// 1DYNAMIC !!!!
#ifdef DYNAMIC
      runner_thrs.emplace_back(&latency_evaluation_t::throughput_listen_ms, &performance, 18);
#endif

      bindCore(11);
      thread_perf_func();

      for (auto &t : runner_thrs)
        t.join();

      uint64_t total_txn_cnt = 0;
      for (auto &cnt : txn_cnts)
        total_txn_cnt += cnt;
      return total_txn_cnt;
    }

    static int execute(int argc, const char *const *argv)
    {
      int ret = 0;

      bool spawn_perf = false;
      bool counter_mode = false;
      int num_threads = 1;
      mode_id mode = mode_id::ReadOnly;
      double time_limit = 10.0;
      bool enable_gc = false;

      Clp_Parser *clp = Clp_NewParser(argc, argv, arraysize(options), options);

      int opt;
      bool clp_stop = false;
      while (!clp_stop && ((opt = Clp_Next(clp)) != Clp_Done))
      {
        switch (opt)
        {
        case opt_dbid:
          break;
        case opt_nthrs:
          num_threads = clp->val.i;
          break;
        case opt_mode:
        {
          switch (*clp->val.s)
          {
          case 'A':
            mode = mode_id::HighContention;
            break;
          case 'B':
            mode = mode_id::MediumContention;
            break;
          case 'C':
            mode = mode_id::ReadOnly;
            break;
          default:
            print_usage(argv[0]);
            ret = 1;
            clp_stop = true;
            break;
          }
          break;
        }
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
        case opt_node:
          break;
        case opt_comm:
          break;
        default:
          print_usage(argv[0]);
          ret = 1;
          clp_stop = true;
          break;
        }
      }

      Clp_DeleteParser(clp);
      if (ret != 0)
        return ret;

      auto profiler_mode = counter_mode ? Profiler::perf_mode::counters
                                        : Profiler::perf_mode::record;

      if (counter_mode && !spawn_perf)
      {
        // turns on profiling automatically if perf counters are requested
        spawn_perf = true;
      }

      if (spawn_perf)
      {
        std::cout << "Info: Spawning perf profiler in "
                  << (counter_mode ? "counter" : "record") << " mode"
                  << std::endl;
      }

      db_profiler prof(spawn_perf);
      ycsb_db<DBParams> db;

      std::cout << "Prepopulating database..." << std::endl;
      db.prepopulate();
      std::cout << "Prepopulation complete." << std::endl;

      std::vector<ycsb_runner<DBParams>> runners;
      puts("modify runner buffer");

      for (int i = 0; i < num_threads; ++i)
      {
        runners.emplace_back(i, db, mode);
      }

      std::thread advancer;
      std::cout << "Generating workload..." << std::endl;
#ifdef QUICKCHECK
      workload_generation(runners, mode);
#endif
      std::cout << "Done." << std::endl;
      std::cout << "Garbage collection: ";
      if (enable_gc)
      {
        std::cout << "enabled, running every 1 ms";
        Transaction::set_epoch_cycle(1000);
        advancer = std::thread(&Transaction::epoch_advancer, nullptr);
      }
      else
      {
        std::cout << "disabled";
      }
      std::cout << std::endl
                << std::flush;

      prof.start(profiler_mode);
      auto num_trans = run_benchmark(db, prof, runners, time_limit);
      prof.finish(num_trans);

      Transaction::rcu_release_all(advancer, num_threads);

      return 0;
    }
  };

}; // namespace ycsb

using namespace ycsb;
using namespace db_params;

double constants::processor_tsc_frequency;

NodeInfo node_i;
std::vector<NodeInfo> node_list;
void node_config()
{
  node_list.clear();
  node_i.server_num = KEY2NODE_MOD;
  node_i.message_size = SLOT_LEN * SLOT_NUM;
  node_i.message_size_rpc_reply = REPLY_LEN * SLOT_NUM + 8 * 1024;
  node_i.double_write_data_size = DATA_LEN * SLOT_NUM;

  for (int i = 0; i < KEY2NODE_MOD; i++)
  {
    node_i.node_id = i;
    node_i.server_type = SERVERTYPE::SERVER_NIC;
    node_list.push_back(node_i);
  }
  for (int i = KEY2NODE_MOD; i < MAX_SERVER; i++)
  {
    node_i.node_id = i;
    node_i.server_type = SERVERTYPE::CLIENT_ONLY;
    node_list.push_back(node_i);
  }
}

int main(int argc, const char *const *argv)
{
  // puts("init RPC HEAR");
  node_config();
  bindCore(11);

#ifndef CPUBASED
  init_fpga();
#endif

#ifdef STRIFE
  init_strife_phase();
  init_strife_queue();
#endif

  // test_fpga();
  printf("fpga_mm = %llx\n", fpga_mm);
#ifdef CPUBASED
  host_mm = (uint64_t)(uint8_t *)malloc(sizeof(uint8_t) * 2 * FEATURE_SIZE);
  fpga_mm = (uint64_t)malloc(fpga_rev_size);
  memset((uint8_t *)fpga_mm, 0, fpga_rev_size);
#endif
  model_update.init_addr(host_mm);
  init_feature_partitionycsb();

  // start FPGA
#ifndef CPUBASED
  begin_fpga();
#endif

  memcached_st *memc = connectMemcached();
  server = new Rpc(node_list[0], node_list, memc, 1);

  db_params_id dbid = db_params_id::Default;
  int ret_code = 0;

  Sto::global_init();
  Clp_Parser *clp = Clp_NewParser(argc, argv, arraysize(options), options);

  int opt;
  bool clp_stop = false;
  bool node_tracking = false;
  bool enable_commute = false;
  while (!clp_stop && ((opt = Clp_Next(clp)) != Clp_Done))
  {
    switch (opt)
    {
    case opt_dbid:
      dbid = parse_dbid(clp->val.s);
      if (dbid == db_params_id::None)
      {
        std::cout << "Unsupported DB CC id: "
                  << ((clp->val.s == nullptr) ? "" : std::string(clp->val.s))
                  << std::endl;
        print_usage(argv[0]);
        ret_code = 1;
        clp_stop = true;
      }
      break;
    case opt_node:
      node_tracking = !clp->negated;
      break;
    case opt_comm:
      enable_commute = !clp->negated;
      break;
    default:
      break;
    }
  }

  Clp_DeleteParser(clp);
  if (ret_code != 0)
    return ret_code;

  auto cpu_freq = determine_cpu_freq();
  if (cpu_freq == 0.0)
    return 1;
  else
    constants::processor_tsc_frequency = cpu_freq;

  switch (dbid)
  {
  case db_params_id::Default:
    // std::cout << "shit default" << std::flush << std::endl;
    // ret_code = ycsb_access<db_adaptive_params>::execute(argc, argv);
    // break;
    if (node_tracking && enable_commute)
    {
      ret_code =
          ycsb_access<db_default_commute_node_params>::execute(argc, argv);
    }
    else if (node_tracking)
    {
      ret_code = ycsb_access<db_default_node_params>::execute(argc, argv);
    }
    else if (enable_commute)
    {
      ret_code = ycsb_access<db_default_commute_params>::execute(argc, argv);
    }
    else
    {
      ret_code = ycsb_access<db_default_params>::execute(argc, argv);
    }
    break;

  case db_params_id::Opaque:
    ret_code = ycsb_access<db_opaque_params>::execute(argc, argv);
    break;
  case db_params_id::TwoPL:
    ret_code = ycsb_access<db_2pl_params>::execute(argc, argv);
    break;
  case db_params_id::Adaptive:
    // puts("??????????????????db_params_id::Adaptive");
    ret_code = ycsb_access<db_adaptive_params>::execute(argc, argv);
    break;
  case db_params_id::Swiss:
    ret_code = ycsb_access<db_swiss_params>::execute(argc, argv);
    break;
  case db_params_id::TicToc:
    if (node_tracking)
    {
      std::cerr << "Warning: node tracking and commute options ignored."
                << std::endl;
    }
    if (enable_commute)
    {
      ret_code = ycsb_access<db_tictoc_commute_params>::execute(argc, argv);
    }
    else
    {
      ret_code = ycsb_access<db_tictoc_params>::execute(argc, argv);
    }
    break;
  case db_params_id::MVCC:
    if (node_tracking && enable_commute)
    {
      ret_code =
          ycsb_access<db_mvcc_commute_node_params>::execute(argc, argv);
    }
    else if (node_tracking)
    {
      ret_code = ycsb_access<db_mvcc_node_params>::execute(argc, argv);
    }
    else if (enable_commute)
    {
      ret_code = ycsb_access<db_mvcc_commute_params>::execute(argc, argv);
    }
    else
    {
      ret_code = ycsb_access<db_mvcc_params>::execute(argc, argv);
    }
    break;
  default:
    std::cerr << "unknown db config parameter id" << std::endl;
    ret_code = 1;
    break;
  };

  return ret_code;
}
