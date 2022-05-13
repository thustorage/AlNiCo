
#include "worker_buf.h"

#include <random>
bool zipf_report;
central_scheduler_t central_scheduler;
tpcc_delivery_t tpcc_delivery;
void worker_msg_t::init(int id)
{
  worker_thread_id = id;
  // printf("I am worker(%d)\n",id);
  msg_id = 0;
  my_fpga_queue = fpga_dispatch_queue + id;
  model_update.init_thread(id);
  abort_queue.init();
  // printf("my doorbell buf first is %d at
  // %llx\n",my_fpga_queue->queue_message[0][0], (uint64_t)my_fpga_queue);
  return;
}

void worker_msg_t::reply_now(bool no_perf_flag)
{
// now_msg.get_last_num();
#ifdef YCSBplusT
  volatile uint8_t *size_addr =
      (volatile uint8_t *)&(((rpc_id_t *)txn_reply_addr)->rpc_type);
  (*size_addr) = now_msg.get_read_num();
#ifdef NULLRPC
  (*size_addr) = 0;
#endif
  volatile uint8_t *phase_addr =
      (volatile uint8_t *)&(((rpc_id_t *)txn_reply_addr)->my_nodeid);
  (*phase_addr) = performance.get_dynamic_phase();
#endif
  volatile uint32_t *r_addr =
      (volatile uint32_t *)&(((rpc_id_t *)txn_reply_addr)->rpc_id);
  (*r_addr) = now_msg.req_head->rpcId.rpc_id + NOTHING_DEBUG;

  if (!no_perf_flag)
    performance.count(worker_thread_id);

  my_fpga_queue->set_message_down(msg_id);
  msg_id = (msg_id + 1) % MESS_NUM;

  return;
}

void strife_msg_t::reply_now_partition(bool no_perf_flag)
{
#ifdef YCSBplusT
  volatile uint8_t *size_addr =
      (volatile uint8_t *)&(((rpc_id_t *)txn_reply_addr)->rpc_type);
  (*size_addr) = now_msg.get_read_num();
  volatile uint8_t *phase_addr =
      (volatile uint8_t *)&(((rpc_id_t *)txn_reply_addr)->my_nodeid);
  (*phase_addr) = performance.get_dynamic_phase();
#endif

  my_strife_queue->set_message_down(strife_msg_id);
  volatile uint32_t *r_addr =
      (volatile uint32_t *)&(((rpc_id_t *)txn_reply_addr)->rpc_id);
  (*r_addr) = now_msg.req_head->rpcId.rpc_id + NOTHING_DEBUG;

  if (!no_perf_flag)
    performance.count(worker_thread_id);

  // volatile uint64_t *r_addr = (volatile uint64_t *)(txn_reply_addr_strife);
  // // printf("thread_id %d, reply(%d,%d,slot_id=%d) = %llx %llx\n", thread_id,
  // client_node_id, client_thread_id, client_rpc_id, (*r_addr),
  // uint64_t((*r_addr) + 1ull)); uint64_t value = (*r_addr);
  // (*r_addr) = uint64_t(value + 11ull);
  strife_msg_id = (strife_msg_id + 1) % strife_buf_num;
}

void worker_msg_t::count_count_only()
{
  performance.count(worker_thread_id);
  return;
}

bool worker_msg_t::next_txn_check()
{
  if (!my_fpga_queue->check_now_message(msg_id))
    return false;

  client_id = my_fpga_queue->get_message_client_id(msg_id);
  slot_id = my_fpga_queue->get_message_slot_id(msg_id);

  auto &now_connect =
      server->nic_connect[client_id / CLIENT_THREAD + KEY2NODE_MOD]
                         [client_id % CLIENT_THREAD];
  txn_addr = now_connect->get_rev_addr(slot_id);
  // printf("txn_addr = %llx\n",txn_addr);
  txn_reply_addr = now_connect->get_send_addr(slot_id);

  now_msg.deserialization(txn_addr);
#ifdef NULLRPC
  performance.count(worker_thread_id);
  reply_now(true);
  return false;
#endif
  now_msg.store_feature(worker_thread_id);
  return true;
}

bool strife_msg_t::next_txn_check_strife()
{
  if (!my_strife_queue->check_now_message(strife_msg_id))
    return false;
  uint64_t strife_msg_addr = my_strife_queue->get_message_addr(strife_msg_id);

  ReqHead *head_for_strife = (ReqHead *)strife_msg_addr;
  client_node_id = head_for_strife->rpcId.my_nodeid - KEY2NODE_MOD;
  client_thread_id = head_for_strife->rpcId.thread_id;
  client_rpc_id = head_for_strife->rpcId.rpc_id;

  auto &now_connect =
      server->nic_connect[client_node_id + KEY2NODE_MOD][client_thread_id];
  // txn_addr = now_connect->get_rev_addr(slot_id);

  txn_reply_addr_strife = now_connect->get_strife_send_addr(client_rpc_id);
  txn_reply_addr =
      now_connect->get_send_addr(head_for_strife->TXID); // slot num
  now_msg.deserialization(strife_msg_addr);
  // now_msg.store_feature(thread_id);
  // printf("%d's %d\n",thread_id, msg_id);
  // puts("!");
  return true;
}
void strife_msg_t::reply_now_strife()
{
  my_strife_queue->set_message_down(strife_msg_id);
  performance.count(worker_thread_id);

  __sync_fetch_and_add((volatile uint64_t *)(txn_reply_addr_strife), 11);
  barrier();

  // volatile uint64_t *r_addr = (volatile uint64_t *)(txn_reply_addr_strife);
  // // printf("thread_id %d, reply(%d,%d,slot_id=%d) = %llx %llx\n", thread_id,
  // client_node_id, client_thread_id, client_rpc_id, (*r_addr),
  // uint64_t((*r_addr) + 1ull)); uint64_t value = (*r_addr);
  // (*r_addr) = uint64_t(value + 11ull);

  strife_msg_id = (strife_msg_id + 1) % strife_buf_num;
  return;
}

void strife_msg_t::next_now_strife()
{
  my_strife_queue->set_message_down(strife_msg_id);
  strife_msg_id = (strife_msg_id + 1) % strife_buf_num;
  return;
}

void strife_msg_t::init_strife(int id)
{
  batch_req_num = 0;
  last_epoch_num = 0;
  my_strife_queue = &(strife_dispatch_queue[id]);
  strife_msg_id = 0;
  cq_signal_id = 0;
  // model_update.init_thread(id);
  // abort_queue.init();
}

void strife_msg_t::count_strife()
{
  while (true)
  {
    if (!my_strife_queue->check_now_message(cq_signal_id))
      return;
    // batch_req_num++;
    strife_batch.count(worker_thread_id);
    cq_signal_id = (cq_signal_id + 1) % strife_buf_num;
  }
  return;
}

void strife_msg_t::store_count()
{
  
  uint64_t tmp = last_epoch_num;
  last_epoch_num = strife_batch.read_count(worker_thread_id);
  batch_req_num = last_epoch_num - tmp;
  // strife cost 3 times
  uint64_t x = 0;
  for (int t = 0; t < 3; t++)
  {
    for (int i = 0; i < batch_req_num; i++)
    { // in strife paper
      uint64_t addr = my_strife_queue->get_message_addr((i + strife_msg_id) % strife_buf_num);
      ReqHead *head_for_strife = (ReqHead *)addr;
      int size = head_for_strife->size;
      for (int j = 0; j < STRIFE_COST; j++)
      {
        x += *(uint64_t *)(addr + size / STRIFE_COST * j);
      }
    }
  }

  return;
}

void worker_msg_t::abort_key(tx_key_t key)
{
  // printf("%d %d\n", key.first, key.second);
  if (key.first == 0xFFFF)
    return;
  pair<int, int> abort_shift = abort_queue.push(key);
  model_update.update_weight(worker_thread_id, abort_shift);
  return;
}

void worker_msg_t::abort_key_ycsb(int i)
{
  pair<int, int> abort_shift = abort_queue.push_ycsb(
      now_msg.get_ycsb_col_origin(i), now_msg.get_ycsb_key_origin(i));
  model_update.update_weight(worker_thread_id, abort_shift);
}

void central_scheduler_t::cpu_alnico_init(uint64_t queue_addr,
                                          uint64_t weights_addr,
                                          uint64_t filter_addr,
                                          uint64_t thread_num_)
{
  weights = (uint8_t *)weights_addr;
  filter = (uint8_t *)filter_addr;
  thread_num = thread_num_;
  memset((void *volatile)head, 0, sizeof(head));
  atomic_add = true;
  for (int i = 0; i < FEATURE_SIZE; i++)
  {
    filter[i] = -1;
  }
  memset(run_count, 0, sizeof(run_count));
  memset((void *volatile)c_byte_feature, 0, sizeof(c_byte_feature));
  memset(old_core_id, -1, sizeof(old_core_id));
  memset((void *volatile)merge_weights, 0, sizeof(merge_weights));
}

void central_scheduler_t::init(uint64_t queue_addr, uint64_t weights_addr,
                               uint64_t filter_addr, uint64_t thread_num_)
{
  for (uint i = 0; i < thread_num_; i++)
  {
    doorbell_queue[i] =
        (fpga_dispatch_queue_t *)(queue_addr +
                                  i * sizeof(fpga_dispatch_queue_t));
  }
  weights = (uint8_t *)weights_addr;
  filter = (uint8_t *)filter_addr;
  thread_num = thread_num_;
  memset((void *volatile)head, 0, sizeof(head));
  atomic_add = true;
  for (int i = 0; i < FEATURE_SIZE; i++)
  {
    filter[i] = -1;
  }
  memset(run_count, 0, sizeof(run_count));
  memset((void *volatile)c_byte_feature, 0, sizeof(c_byte_feature));
  memset(old_core_id, -1, sizeof(old_core_id));
  memset((void *volatile)merge_weights, 0, sizeof(merge_weights));
}

void central_scheduler_t::init_strife(uint64_t queue_addr,
                                      uint64_t thread_num_)
{
  for (uint i = 0; i < thread_num_; i++)
  {
    strife_queue[i] = ((strife_queue_t *)(queue_addr)) + i;
  }
  strife_buffer =
      (uint8_t *)malloc(CLIENT_THREAD * MAX_CLIENT * IDMAPADDRSIZE * DATA_LEN);
  thread_num = thread_num_;
  memset((void *volatile)head, 0, sizeof(head));
  atomic_add = true;
}

void central_scheduler_t::push_new_request(uint16_t core_id, uint32_t slot_id)
{
  int offset;
  if (atomic_add)
  {
    offset = __sync_fetch_and_add(&(head[core_id][0]), 1) % MESS_NUM;
  }
  else
  {
    offset = (head[core_id][0]++) % MESS_NUM;
  }
  // printf("%d %d %d %d\n",core_id, slot_id,
  // doorbell_queue[core_id]->queue_message[offset][0], slot_id);
  doorbell_queue[core_id]->queue_message[offset][0] = slot_id;
  return;
}

void central_scheduler_t::push_new_request_stife(uint16_t core_id,
                                                 uint64_t slot_addr)
{
  int offset;
  offset = __sync_fetch_and_add(&(head[core_id][0]), 1) % strife_buf_num;
  strife_queue[core_id]->queue_message[offset] = slot_addr;
  return;
}

void central_scheduler_t::replace_new_request(uint16_t core_id,
                                              uint32_t slot_id)
{
  old_core_id[slot_id] = core_id;
  for (int i = 0; i < FEATURE_SIZE / 8; i++)
  {
    uint8_t now_f = old_feature[slot_id][i];
    for (int j = 0; j < 8; j++)
    {
      if ((now_f & (1u << j)) != 0)
      {
        int x;
        if (atomic_add)
          x = __sync_fetch_and_add(&c_feature[core_id][i * 8 + j], 1);
        else
        {
          x = c_feature[core_id][i * 8 + j]++;
        }
        if (x >= 0 && (filter[i * 8 + j] == MAX_CORE + 1 ||
                       filter[i * 8 + j] == core_id))
        {
          if (atomic_add)
            __sync_fetch_and_or(&c_byte_feature[core_id][i], 1u << j);
          else
          {
            // puts("???");
            c_byte_feature[core_id][i] =
                (c_byte_feature[core_id][i] | (1u << j));
          }
        }
      }
    }
  }
  return;
}
void central_scheduler_t::delate_old_request(uint32_t slot_id)
{
  int old_c_id = old_core_id[slot_id];
  if (old_c_id != -1)
  {
    for (int i = 0; i < FEATURE_SIZE / 8; i++)
    {
      uint8_t now_f = old_feature[slot_id][i];
      for (int j = 0; j < 8; j++)
      {
        if ((now_f & (1u << j)) != 0)
        {
          int x = 0;
          if (atomic_add)
          {
            x = __sync_fetch_and_add(&(c_feature[old_c_id][i * 8 + j]), -1);
          }
          else
          {
            x = (c_feature[old_c_id][i * 8 + j]--);
          }
          (void)x;
          if (filter[i * 8 + j] != MAX_CORE + 1 &&
              filter[i * 8 + j] != old_c_id)
          {
            if (atomic_add)
              __sync_fetch_and_and(&c_byte_feature[old_c_id][i],
                                   ((1u << 8) - 1 - (1 << j)));
            else
              c_byte_feature[old_c_id][i] =
                  (c_byte_feature[old_c_id][i] & ((1u << 8) - 1 - (1 << j)));
          }
        }
      }
    }
  }
  memcpy(old_feature[slot_id], (uint8_t *volatile)get_feature_addr(slot_id),
         FEATURE_SIZE / 8);
  return;
}
uint16_t central_scheduler_t::get_core_id(uint32_t slot_id,
                                          uint32_t default_id)
{
  pair<int, int> ans = make_pair(0, default_id);

  for (int t = 0; t < MAX_THREAD; t++)
  {
    int tmp_ans = 0;
    for (int i = 0; i < FEATURE_SIZE / 8; i++)
    {
      uint8_t now_f = old_feature[slot_id][i];
      uint8_t now_cf = c_byte_feature[t][i];
      tmp_ans += merge_weights[i][now_f & now_cf];
    }
    if (tmp_ans > ans.first)
    {
      ans.second = t;
      ans.first = tmp_ans;
      // printf("%d %d\n", tmp_ans, t);
    }
  }
  return ans.second;
}
uint64_t central_scheduler_t::get_addr(uint32_t slot_id)
{
  uint32_t client_id = slot_id / SLOT_NUM;
  int c_slot_id = slot_id % SLOT_NUM;
  auto &now_connect =
      server->nic_connect[client_id / CLIENT_THREAD + KEY2NODE_MOD]
                         [client_id % CLIENT_THREAD];
  return now_connect->get_rev_addr(c_slot_id);
}
uint64_t central_scheduler_t::get_feature_addr(uint32_t slot_id)
{
  uint64_t addr = get_addr(slot_id);

  uint32_t *req_head = (uint32_t *)addr;
  uint32_t size = req_head[4];
  uint64_t feature_addr = addr + size - FEATURE_SIZE / 8;
  return feature_addr;
}

void central_scheduler_t::run_once(int begin_id, int end_id, int thread_id)
{
  if ((run_count[thread_id] % 1000 == 0) && (thread_id == 0))
  {
    // merge_weights[FEATURE_SIZE/8][256];
    update_weights();
  }
  run_count[thread_id]++;
  for (int i = begin_id; i < end_id; i++)
  {
    {
      // Reqhead | data | tail_magic
      uint32_t *req_head = (uint32_t *)get_addr(i);
      // puts("run ???");
      uint32_t size = req_head[4]; // byte
      if (size == 0)
        continue;

      uint32_t magic = req_head[5];
      uint32_t *tail_magic = (uint32_t *)(req_head + size / 4);
      // printf("size=%d\n",size);
      if (magic != *tail_magic)
      {
        continue;
      }
      *tail_magic = 0;

      delate_old_request(i);

      uint32_t c_id = req_head[6];
      c_id = get_core_id(i, c_id);

      replace_new_request(c_id, i);
      push_new_request(c_id, i);
    }
  }
}

void central_scheduler_t::test_reply(int thread_id, uint64_t reply_addr)
{
  performance.count(thread_id);
  volatile uint64_t *r_addr = (volatile uint64_t *)(reply_addr);
  uint64_t value = (*r_addr);
  // printf("thread_id %d, reply(%d,%d,slot_id=%d) = %llx %llx\n", c_thread_id,
  // client_node_id, client_thread_id, client_rpc_id, (*r_addr),
  // uint64_t((*r_addr) + 1ull));
  (*r_addr) = value + 11;
  // printf("reply %llx %llu-->%llu\n",reply_addr, value,(*r_addr));
  return;
}

void central_scheduler_t::run_strife_once(int begin_id, int end_id,
                                          int thread_id)
{
#if (CPUBASED == 1)
  if ((run_count[thread_id] % 1000 == 0) && (thread_id == 0))
  {
    // merge_weights[FEATURE_SIZE/8][256];
    update_weights();
  }
  run_count[thread_id]++;
#endif

#if (CPUBASED == 2)
  if ((run_count[thread_id] % 1000 == 0) && (thread_id == 0))
  {
    // merge_weights[FEATURE_SIZE/8][256];
    update_weights();
  }
  run_count[thread_id]++;
#endif

  for (int i = begin_id; i < end_id; i++)
  {
    if (begin_id >= MAX_CLIENT * SLOT_NUM * CLIENT_THREAD)
      continue;
    {
      // Reqhead | data | tail_magic
      uint64_t recv_addr = (uint64_t)get_addr(i);
      volatile uint32_t *req_head = (volatile uint32_t *)recv_addr;
      // puts("run ???");
      uint32_t size = req_head[4]; // byte
      if (size == 0)
        continue;

      uint32_t magic = req_head[5];
      volatile uint32_t *tail_magic = (volatile uint32_t *)(req_head + size / 4);
      // 
      // if (thread_id == 0)
      //   printf("size=%d %d %d\n",size, magic, *tail_magic);
      if (magic != *tail_magic)
      {
        continue;
      }
      *tail_magic = 0;

      ReqHead *head_for_strife = (ReqHead *)recv_addr;
      uint32_t client_id = head_for_strife->rpcId.my_nodeid - KEY2NODE_MOD;
      uint32_t c_thread_id = head_for_strife->rpcId.thread_id;
      uint32_t rpc_id = head_for_strife->rpcId.rpc_id;
      uint64_t addr = 0;
#if (CPUBASED == 3)
      addr =
          ((client_id * CLIENT_THREAD + c_thread_id) * IDMAPADDRSIZE + rpc_id) *
              DATA_LEN +
          (uint64_t)strife_buffer;
      memcpy((void *)addr, (void *)recv_addr, size);
#else
      addr = recv_addr;
#endif

      uint32_t c_id = req_head[6];


// for fair reserved CPU core.
#ifndef STATIC_BEST
#ifndef YCSBplusT
#if (CPUBASED == 3)
      if (c_id < 2)
      {
        c_id = rpc_id % 21 + 1;
      }
#endif
#if (CPUBASED == 4)
      if (c_id < 2)
      {
        c_id = ((rpc_id % 10 < 5) * 10 + c_id) % (MAX_THREAD + 2);
      }
#endif
#endif
#endif


// NetSTO
#if (BASELINE == 1)
#ifndef YCSBplusT
      c_id = rand() % (MAX_THREAD + 2); // for fair reserved CPU core.
#else
      c_id = rand() % MAX_THREAD;
#endif
#endif


// co-located
#if (CPUBASED == 1)
      c_id = rand() % (MAX_THREAD);
      delate_old_request(i);
      c_id = get_core_id(i, c_id);
      // printf("in")
      replace_new_request(c_id, i);
#endif

// central
#if (CPUBASED == 2)
      c_id = rand() % (MAX_THREAD);
      delate_old_request(i);
      c_id = get_core_id(i, c_id);
      // printf("in")
      replace_new_request(c_id, i);
#endif

// #endif

// c_id = rand() % 22;
#ifndef NULLRPC
      push_new_request_stife(c_id, addr);
#endif

      uint64_t type = head_for_strife->read_name;

#ifdef NULLRPC
      performance.count(thread_id);
      auto &now_connect =
          server->nic_connect[client_id + KEY2NODE_MOD][c_thread_id];
      // printf("client_id %d=%d thread_id %d=%d\n",client_id, i / SLOT_NUM /
      // CLIENT_THREAD, c_thread_id, i / SLOT_NUM % CLIENT_THREAD);

      uint64_t reply_addr = now_connect->get_send_addr(i % SLOT_NUM);
      volatile uint8_t *size_addr =
          (volatile uint8_t *)&(((rpc_id_t *)reply_addr)->rpc_type);
      (*size_addr) = 0;
      // test_reply(thread_id, now_connect->get_strife_send_addr(rpc_id));
      volatile uint32_t *r_addr =
          (volatile uint32_t *)&(((rpc_id_t *)reply_addr)->rpc_id);
      (*r_addr) = head_for_strife->rpcId.rpc_id + NOTHING_DEBUG;
#endif

/*strife recv ack*/
#if (CPUBASED == 3)
      auto &now_connect =
          server->nic_connect[client_id + KEY2NODE_MOD][c_thread_id];

      uint64_t reply_addr = now_connect->get_send_addr(i % SLOT_NUM);
#ifdef YCSBplusT
      volatile uint8_t *size_addr =
          (volatile uint8_t *)&(((rpc_id_t *)reply_addr)->rpc_type);

      (*size_addr) = head_for_strife->read_num;

      volatile uint8_t *phase_addr =
          (volatile uint8_t *)&(((rpc_id_t *)reply_addr)->my_nodeid);
      (*phase_addr) = performance.get_dynamic_phase();
#endif
      // test_reply(thread_id, now_connect->get_strife_send_addr(rpc_id));
      volatile uint32_t *r_addr =
          (volatile uint32_t *)&(((rpc_id_t *)reply_addr)->rpc_id);
      (*r_addr) = head_for_strife->rpcId.rpc_id + NOTHING_DEBUG;
#endif
    }
  }
}

void central_scheduler_t::update_weights()
{
  uint8_t last = 0, now = 0;
  for (int i = 0; i < FEATURE_SIZE / 8; i++)
  {
    merge_weights[i][0] = 0;
    now = 0;
    for (int j = 1; j <= 255; j++)
    {
      last = now;
      now = j ^ (j >> 1);
      uint8_t delta_bit = (now ^ last);
      int delta_id = 0;
      switch (delta_bit)
      {
      case 1:
      {
        delta_id = 0;
        break;
      }
      case 2:
      {
        delta_id = 1;
        break;
      }
      case 4:
      {
        delta_id = 2;
        break;
      }
      case 8:
      {
        delta_id = 3;
        break;
      }
      case 16:
      {
        delta_id = 4;
        break;
      }
      case 32:
      {
        delta_id = 5;
        break;
      }
      case 64:
      {
        delta_id = 6;
        break;
      }
      case 128:
      {
        delta_id = 7;
        break;
      }
      }

      if (now > last)
      {
        merge_weights[i][now] =
            merge_weights[i][last] + weights[i * 8 + delta_id];
      }
      else
      {
        merge_weights[i][now] =
            merge_weights[i][last] - weights[i * 8 + delta_id];
      }
    }
    // printf("%d %d gg = %lf\n", i, 255, merge_weights[i][255]);
  }
}
void central_scheduler_t::run(int begin_id, int end_id, int thread_id)
{
  // atomic_add = false;
  puts("gogogo one cpu");
  while (true)
  {
    run_strife_once(begin_id, end_id, thread_id);
  }
  return;
}
