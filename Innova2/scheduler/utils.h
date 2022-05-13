#ifndef __TEST_H__
#define __TEST_H__
#include <ap_int.h>
#include <hls_stream.h>
#include <memory.h>
#define N 4
#define SLOT_LEN 2048
#define MAX_CORE 24
#define MESS_LEN 64
#define MESS_LEN_DIV_4 16
#define QUEUE_METADATA 256
typedef int uint32;
#define MUTEX_BASE 0x10000000
#define MUTEX_SIZE 64
#define TOTAL_IP_MESS_N 256
#define HOSTADDR 0x04000000
#define CDMAADDR 0x08000000
#define CDMA_OFFSET 0x02000000
#define CDMASIZE 0x1000 //4k
#define MAX_CLIENT_NUM 48
#define MAX_SLOT_NUM 4
typedef ap_uint<512> feature_t;
#define FEATURE_SIZE 512
#define RWFEATURE 0
// W:11 R:10  (WR)
//
#define VECTOR_SIZE 16

struct check_msg_out_t
{
	uint32 i;
	uint32 core_id;
	uint32 addr;
	uint32 size;
	check_msg_out_t() {}
	check_msg_out_t(
		uint32 i,
		uint32 core_id,
		uint32 addr,
		uint32 size)
		: i(i),
		  core_id(core_id),
		  addr(addr),
		  size(size)
	{
	}
};
struct cdma_msg_out_t
{
	uint32 i;
	uint32 core_id;

	feature_t old_features;
	feature_t new_features;
	cdma_msg_out_t() {}
	cdma_msg_out_t(
		uint32 i,
		uint32 core_id,
		feature_t old_features)
		: i(i),
		  core_id(core_id),
		  old_features(old_features)
	{
	}
};

struct update_cluster_in_t
{
	feature_t old_features;
	feature_t new_features;
	uint32 i;
	uint32 core_id;
	update_cluster_in_t() {}
	update_cluster_in_t(
		feature_t old_features,
		feature_t new_features,
		uint32 i,
		uint32 core_id)
		: old_features(old_features),
		  new_features(new_features),
		  i(i),
		  core_id(core_id)
	{
	}
};
//from bit_and to compute id
struct bit_and_out_t
{

	int i, id, core_id;
	feature_t distance_bit;
	feature_t old_features;
	feature_t new_features;
	bit_and_out_t() {}
	bit_and_out_t(
		int i,
		int id,
		int core_id,
		feature_t distance_bit)
		: i(i),
		  id(id),
		  core_id(core_id),
		  distance_bit(distance_bit)
	{
	}
	bit_and_out_t(
		int i,
		int id,
		int core_id,
		feature_t distance_bit,
		feature_t old_features,
		feature_t new_features)
		: i(i),
		  id(id),
		  core_id(core_id),
		  distance_bit(distance_bit),
		  old_features(old_features),
		  new_features(new_features)
	{
	}
};

struct clusters_t
{
	ap_uint<8> clusters[24][FEATURE_SIZE];
	//#pragma HLS ARRAY_PARTITION variable=clusters complete
	clusters_t()
	{
		for (int i = 0; i < 24; i++)
		{
			for (int j = 0; j < FEATURE_SIZE; j++)
				clusters[i][j] = 0;
			clusters[i][i] = 1;
		}
	}
};
struct compute_id_out_t
{
	uint32 i;
	uint32 core_id;
	compute_id_out_t() {}
	compute_id_out_t(
		uint32 i,
		uint32 core_id)
		: i(i),
		  core_id(core_id)
	{
	}
};
struct membership_t
{
	int index;
	int old_index;
	ap_uint<16> ans;
	//	ap_uint<16> ansmap[MAX_CORE];
	//#pragma HLS ARRAY_PARTITION variable=ansmap complete
	membership_t() {}
	membership_t(int index, ap_uint<16> ans)
		: index(index), ans(ans)
	{
	}
	void cheat(int x, ap_uint<16> cheat_ans)
	{
		old_index = index;
		ans = cheat_ans;
		index = x;
	}
	//	inline void update_index(int x)
	//	{
	//		if (ansmap[x] > ans)
	//		{
	//			ans = ansmap[x];
	//			index = x;
	//		}
	//	}
};

void doorbell_dataflow(hls::stream<cdma_msg_out_t> &doolbell_in, uint32 *hostm_d);
void schedule_nodma(uint32 *m0, uint32 *m1, uint32 *hostm, uint32 *cdma, ap_uint<8> *cdma_buf, int con, uint32 BASE_ADDR_, uint32 CLIENT_NUM_, uint32 SLOT_NUM_, uint32 CORE_NUM_, uint32 MESS_NUM_, uint32 b_client_, uint32 e_client_, uint32 b_core_, uint32 e_core_, uint32 ip_id_);
void check_message_fake(hls::stream<compute_id_out_t> &check_msg_out, uint32 *m_d0);

void init_bitmap();
void init_weight(uint32 *cdma, ap_uint<8> *cdma_buf);
void doorbell_dataflow(hls::stream<compute_id_out_t> &doolbell_in, uint32 *hostm_d);
void update_cluster(hls::stream<update_cluster_in_t> &update_cluster_in);
void compute_core_id(hls::stream<cdma_msg_out_t> &compute_core_id_in0, hls::stream<cdma_msg_out_t> &compute_core_id_in1, hls::stream<compute_id_out_t> &compute_core_id_out, hls::stream<update_cluster_in_t> &update_cluster_in);
void check_message_dataflow_nodma0(hls::stream<cdma_msg_out_t> &check_msg_out, uint32 *m_d);
void check_message_dataflow_nodma1(hls::stream<cdma_msg_out_t> &check_msg_out, uint32 *m_d);
#endif
