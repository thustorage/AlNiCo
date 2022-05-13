#include "utils.h"

extern uint32 BASE_ADDR;
extern uint32 CLIENT_NUM;
extern uint32 SLOT_NUM;
extern uint32 CORE_NUM;
extern uint32 MESS_NUM;
extern uint32 b_client;
extern uint32 e_client;
extern uint32 b_core;
extern uint32 e_core;
extern uint32 ip_id;
extern ap_uint<12> merge_weights[64][256];
extern int ii, jj, kk;
extern cdma_msg_out_t last_cdma;

extern clusters_t cluster;

// const value
extern uint32 MESSAGE_ADDR;
extern uint32 QUEUE_ADDR_BASE;
extern uint32 my_client_num;
extern uint32 max_slot;
extern uint32 slot_partition0;
extern uint32 slot_partition1;
extern ap_uint<8> hard_partition[FEATURE_SIZE];
// slot addr offset
extern uint32 slot_offset0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
extern uint32 slot_offset1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
extern feature_t features0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
extern feature_t features1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
extern int RRB0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
extern int RRB1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];

extern feature_t bit_clusters[MAX_CORE];
extern uint32 old_core_id[MAX_CLIENT_NUM * MAX_SLOT_NUM];
//membership_t memberships[MAX_CLIENT_NUM][MAX_SLOT_NUM];

extern ap_uint<8> weights[FEATURE_SIZE/8][8];

// const value for doorbell
extern uint32 queue_addr_map[MAX_CORE];
extern uint32 queue_metadata[MAX_CORE][4];

void init_weight(uint32 *cdma, ap_uint<8> *cdma_buf)
{
#pragma HLS ARRAY_PARTITION variable = merge_weights complete dim = 1
#pragma HLS ARRAY_PARTITION variable = weights complete dim=1
	//
	// 	for (int i = 0; i < FEATURE_SIZE; i++)
	// 	{
	// //#pragma HLS PIPELINE
	// #pragma HLS UNROLL
	// 		if (i < 20)
	// 			weights[i] = 10;
	// 		else
	// 			weights[i] = 0;
	// 	}
	memcpy(weights, cdma_buf + 0x200000, FEATURE_SIZE);
//	printf("ggw36 = %d %d %d\n",(int)weights[36], (int)cdma_buf[0x200000 + 36], (int)cdma_buf[0x200000 + 36 + 512]);
	ap_uint<8> last = 0;
	ap_uint<8> now = 0;
	ap_uint<8> last_value[64];
#pragma HLS ARRAY_PARTITION variable = last_value complete

LOOP_UW_IT:
	for (int i = 0; i < 64; i++)
	{
#pragma HLS UNROLL
		last_value[i] = 0;
		merge_weights[i][0] = 0;
	}

	for (int gray = 1; gray < 128; gray++)
	{
#pragma HLS PIPELINE
		last = now;
		now = gray ^ (gray >> 1);
		ap_uint<8> delta_bit = (now ^ last);
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
		const int delta_id_ = delta_id;


		for (int i = 0; i < 64; i++)
		{
#pragma HLS UNROLL
			if (now > last)
			{
				last_value[i] = last_value[i] + weights[i][delta_id_];
			}
			else
			{
				last_value[i] = last_value[i] - weights[i][delta_id_];
			}
			merge_weights[i][now] = last_value[i];
		}
	}
	memcpy(hard_partition, cdma_buf + 0x200000 + FEATURE_SIZE, FEATURE_SIZE);
	cdma[CDMA_OFFSET + 0x6] = MESSAGE_ADDR;
	// destination
	cdma[CDMA_OFFSET + 0x8] = 0x200000;
	// size
	cdma[CDMA_OFFSET + 10] = FEATURE_SIZE * 2;
	// source
}

void init_bitmap()
{
#pragma HLS ARRAY_PARTITION variable = cluster.clusters cyclic factor = 32 dim = 2
#pragma HLS ARRAY_PARTITION variable = hard_partition cyclic factor = 32
#pragma HLS ARRAY_PARTITION variable = bit_clusters complete
//#pragma HLS ARRAY_PARTITION variable = hard_partition
LOOP_UBIT_IT:
	for (int f = 0; f < FEATURE_SIZE; f++)
	{
#pragma HLS UNROLL factor = 32
#pragma HLS PIPELINE II==11
		const int my_hp = hard_partition[f];
		const bool my_hp_true = (my_hp == 25);
#pragma HLS RESOURCE variable=my_hp latency=2
		for (int i = 0; i < 22 /*CORE_NUM*/; i++)
		{
//			if (i >= CORE_NUM) break;
#pragma HLS UNROLL
//#pragma HLS Latency min=15
			bit_clusters[i][f] = (my_hp == i) || ((cluster.clusters[i][f] != 0) && (my_hp_true));
		}
//		bit_clusters[][f] = 1;
	}
}

void doorbell_dataflow(hls::stream<compute_id_out_t> &doolbell_in, uint32 *hostm_d)
{

#pragma HLS PIPELINE II = 4
	if (!doolbell_in.empty())
	{
		compute_id_out_t doorbell_msg = doolbell_in.read();

		int local_head = (queue_metadata[doorbell_msg.core_id][0]++);
		if (local_head == MESS_NUM - 1)
		{
			queue_metadata[doorbell_msg.core_id][0] = 0;
		}
		int queue_offset = queue_addr_map[doorbell_msg.core_id] + (local_head)*MESS_LEN_DIV_4;
		hostm_d[queue_offset] = (doorbell_msg.i);
	}
}

void update_cluster(hls::stream<update_cluster_in_t> &update_cluster_in)
{
#pragma HLS ARRAY_PARTITION variable = cluster.clusters cyclic factor = 32 dim = 2
	if (!update_cluster_in.empty())
	{
		update_cluster_in_t msg = update_cluster_in.read();
		int old_index = old_core_id[msg.i];
		for (int f = 0; f < 512; f++)
		{
#pragma HLS UNROLL factor = 32
#pragma HLS PIPELINE II = 3
			if (msg.old_features[f] == 1)
			{
				cluster.clusters[old_index][f]--;
			}
			if (msg.new_features[f] == 1)
			{
				cluster.clusters[msg.core_id][f]++;
			}
		}
//		for (int f = FEATURE_SIZE - RWFEATURE; f < FEATURE_SIZE; f+=2)
//		{
//#pragma HLS UNROLL factor = 16
//#pragma HLS PIPELINE II = 8
//			if (msg.old_features[f] == 1)
//			{
//				if (msg.old_features[f+1] == 1) // W request
//					cluster.clusters[old_index][f]--;
//				else // R request
//					cluster.clusters[old_index][f+1]--;
//			}
//			if (msg.new_features[f] == 1)
//			{
//				if (msg.new_features[f+1] == 1) // W request
//					cluster.clusters[msg.core_id][f]--;
//				else // R request
//					cluster.clusters[msg.core_id][f+1]--;
//
//			}
//		}
		old_core_id[msg.i] = msg.core_id;
	}
}

inline void core_id_func(cdma_msg_out_t msg, hls::stream<compute_id_out_t> &compute_core_id_out, hls::stream<update_cluster_in_t> &update_cluster_in)
{

#pragma HLS ARRAY_PARTITION variable = merge_weights complete dim = 1
//#pragma HLS ARRAY_PARTITION variable = bit_clusters complete
	membership_t membership;
	membership.cheat(msg.core_id, 0x9);
	membership.ans = 0;

	//	printf("ans%d = %d\n", msg.i, msg.core_id);

	for (int c = 0; c < 22 /*MAX_CORE_NUM*/; c++)
	{
		if (c >= CORE_NUM) break;
#pragma HLS PIPELINE
		feature_t distance_bitmap = msg.new_features & bit_clusters[c];
		ap_uint<16> ansmap = 0;
		for (int f = 0; f < 64 /*FEATURE SIZE / 8*/; f++)
		{
//			if (distance_bitmap.range(8 * f + 7, 8 * f)!=0){
//				printf("mapbit ok = %d %x\n", f, (int)(distance_bitmap.range(8 * f + 7, 8 * f)));
//				printf("mw = %d \n",(int)merge_weights[f][distance_bitmap.range(8 * f + 7, 8 * f)]);
//			}
			ansmap += merge_weights[f][distance_bitmap.range(8 * f + 7, 8 * f)];
		}
//		printf("%d:%d\n",c,(int)ansmap);
		if (ansmap > membership.ans)
		{

			membership.index = c;
			membership.ans = ansmap;
		}
	}
	compute_core_id_out.write(compute_id_out_t(
		msg.i,
		membership.index));

	update_cluster_in.write(update_cluster_in_t(
		msg.old_features,
		msg.new_features,
		msg.i,
		membership.index));
	//	old_core_id[msg.i] = membership.index;
}

void compute_core_id(hls::stream<cdma_msg_out_t> &compute_core_id_in0, hls::stream<cdma_msg_out_t> &compute_core_id_in1, hls::stream<compute_id_out_t> &compute_core_id_out, hls::stream<update_cluster_in_t> &update_cluster_in)
{
	cdma_msg_out_t msg;
	switch (kk)
	{
	case 0:
	{
		if (compute_core_id_in0.empty())
		{
			kk = !kk;
			return;
		}
		msg = compute_core_id_in0.read();
		break;
	}
	case 1:
	{
		if (compute_core_id_in1.empty())
		{
			kk = !kk;
			return;
		}
		msg = compute_core_id_in1.read();
		break;
	}
	}
	core_id_func(msg, compute_core_id_out, update_cluster_in);
	return;
}

inline void load_feature(int offset, cdma_msg_out_t &msg, uint32 *m_d)
{
#pragma HLS latency min = 24
#pragma HLS PIPELINE II = 16
	uint32 buf[VECTOR_SIZE];
	memcpy(buf, m_d + offset, VECTOR_SIZE * 4);
	for (int c = 0; c < VECTOR_SIZE; c++)
	{
		msg.new_features.range(c * 32 + 31, c * 32) = buf[c]; //m_d[now_slot_offset + 1 + c];
															  //		printf("read %llx %llx %llx\n",buf[c], msg.new_features.range(c * 32 + 31, c * 32), m_d[offset+c]);
	}
}

void check_message_dataflow_nodma0(hls::stream<cdma_msg_out_t> &check_msg_out, uint32 *m_d)
{

	int i = ii;
	int message_head;
	int tmp_RRB;
	{
		int now_slot_offset0 = slot_offset0[i];
		message_head = m_d[now_slot_offset0];
		tmp_RRB = (message_head >> 16) & 0xFFFF;

		if ((tmp_RRB | 0xFFFF0000) == ((~RRB0[i]) | 0xFFFF0000))
		{

			RRB0[i] = tmp_RRB;
			int core_id = message_head & 0xFFFF;
			cdma_msg_out_t tmp_msg(i, core_id, features0[i]);

			load_feature(now_slot_offset0 - 16, tmp_msg, m_d);

			check_msg_out.write(tmp_msg);
			features0[i] = tmp_msg.new_features;
		}
		// update scheduler
	}

	if (ii == slot_partition0 - 1)
	{
		ii = 0;
	}
	else
	{
		ii += 1;
	}
}
void check_message_dataflow_nodma1(hls::stream<cdma_msg_out_t> &check_msg_out, uint32 *m_d)
{

	int j = jj;
	int message_head;
	int tmp_RRB;
	{
		int now_slot_offset1 = slot_offset1[j];
		message_head = m_d[now_slot_offset1];
		tmp_RRB = (message_head >> 16) & 0xFFFF;

		if ((tmp_RRB | 0xFFFF0000) == ((~RRB1[j]) | 0xFFFF0000))
		{
			RRB1[j] = tmp_RRB;
			int core_id = message_head & 0xFFFF;
			cdma_msg_out_t tmp_msg(j + slot_partition1, core_id, features1[j]);
			load_feature(now_slot_offset1 - 16, tmp_msg, m_d);
			check_msg_out.write(tmp_msg);
			features1[j] = tmp_msg.new_features;
		}
		// update scheduler
	}

	if (jj == slot_partition1 - 1)
	{
		jj = 0;
	}
	else
	{
		jj += 1;
	}
}
