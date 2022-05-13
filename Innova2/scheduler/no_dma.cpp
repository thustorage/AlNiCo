//#include <ap_cint.h>
//#include <ap_int.h>
#include "utils.h"
/*
con: operation id
BASE_ADDR: QUEUE ADDR
client_num * slot_num: requests from client
core_num * mess_num: requests to core
*/

// args
uint32 BASE_ADDR;
uint32 CLIENT_NUM;
uint32 SLOT_NUM;
uint32 CORE_NUM;
uint32 MESS_NUM;
uint32 b_client;
uint32 e_client;
uint32 b_core;
uint32 e_core;
uint32 ip_id;

int ii, jj, kk;
cdma_msg_out_t last_cdma;
//clusters_t global_old_cluster;
clusters_t cluster;

// const value
uint32 MESSAGE_ADDR = HOSTADDR;
uint32 QUEUE_ADDR_BASE;
uint32 my_client_num;
uint32 slot_partition0;
uint32 slot_partition1;
uint32 max_slot;

// slot addr offset
uint32 slot_offset0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2] = {0};
uint32 slot_offset1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2] = {0};
feature_t features0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2] = {0};
feature_t features1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2] = {0};
int RRB0[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
int RRB1[MAX_CLIENT_NUM * MAX_SLOT_NUM / 2];
uint32 old_core_id[MAX_CLIENT_NUM * MAX_SLOT_NUM];
feature_t bit_clusters[MAX_CORE];
//feature_t old_bit_clusters[MAX_CORE];
//ap_uint<8> gray_code[256];
//membership_t memberships[MAX_CLIENT_NUM][MAX_SLOT_NUM];

ap_uint<8> weights[FEATURE_SIZE/8][8];
ap_uint<8> hard_partition[FEATURE_SIZE];
ap_uint<12> merge_weights[FEATURE_SIZE / 8][256];

// const value for doorbell
uint32 queue_addr_map[MAX_CORE];
uint32 queue_metadata[MAX_CORE][4];

void dataflow_no_dma(uint32 *m0, uint32 *m1, uint32 *hostm)
{

	static hls::stream<cdma_msg_out_t> check_msg_out0;
	static hls::stream<cdma_msg_out_t> check_msg_out1;
	static hls::stream<compute_id_out_t> compute_core_id_out;

	static hls::stream<update_cluster_in_t> update_cluster_in;
//	static hls::stream<bit_and_out_t> bit_and_out;
//	static hls::stream<next_slot_stream_t> inter_stream;
#pragma HLS stream depth = 96 variable = check_msg_out0
#pragma HLS stream depth = 96 variable = check_msg_out1
#pragma HLS stream depth = 192 variable = compute_core_id_out
#pragma HLS stream depth = 192 variable = update_cluster_in
	//#pragma HLS stream depth = 3840 variable = bit_and_out
	//#pragma HLS stream depth=256 variable = inter_stream

#pragma HLS DATAFLOW
	// load_cluster(now_clusters); // fetch old cluster, generate bitmap
	check_message_dataflow_nodma0(check_msg_out0, m0);
	check_message_dataflow_nodma1(check_msg_out1, m1);

	//	bit_and(check_msg_out, bit_and_out);
	compute_core_id(check_msg_out0, check_msg_out1, compute_core_id_out, update_cluster_in); // load weights
	doorbell_dataflow(compute_core_id_out, hostm);
	update_cluster(update_cluster_in); // output new cluster

	//	check_message_fake(compute_core_id_out, m);
	//	doorbell_dataflow(compute_core_id_out, hostm);
	return;
}

void schedule_nodma(uint32 *m0, uint32 *m1, uint32 *hostm, uint32 *cdma, ap_uint<8> *cdma_buf, int con, uint32 BASE_ADDR_, uint32 CLIENT_NUM_, uint32 SLOT_NUM_, uint32 CORE_NUM_, uint32 MESS_NUM_, uint32 b_client_, uint32 e_client_, uint32 b_core_, uint32 e_core_, uint32 ip_id_)
{
#pragma HLS INTERFACE m_axi depth = 33570816 port = m0 bundle = BUS_M0 num_read_outstanding = 32 max_read_burst_length = 16 num_write_outstanding = 32 max_write_burst_length = 16 //latency = 10
#pragma HLS INTERFACE m_axi depth = 33570816 port = m1 bundle = BUS_M1 num_read_outstanding = 32 max_read_burst_length = 16 num_write_outstanding = 32 max_write_burst_length = 16 //latency = 10
#pragma HLS INTERFACE m_axi depth = 33570816 port = hostm bundle = BUS_HOST num_write_outstanding = 32 max_write_burst_length = 8												   // latency = 100
#pragma HLS INTERFACE m_axi depth = 33570816 port = cdma bundle = BUS_CDMA																											   //latency = 10
#pragma HLS INTERFACE m_axi depth = 33570816 port = cdma_buf bundle = BUS_BUF

#pragma HLS INTERFACE s_axilite port = return
#pragma HLS INTERFACE s_axilite port = con
#pragma HLS INTERFACE s_axilite port = BASE_ADDR_
#pragma HLS INTERFACE s_axilite port = CLIENT_NUM_
#pragma HLS INTERFACE s_axilite port = SLOT_NUM_
#pragma HLS INTERFACE s_axilite port = CORE_NUM_
#pragma HLS INTERFACE s_axilite port = MESS_NUM_
#pragma HLS INTERFACE s_axilite port = b_client_
#pragma HLS INTERFACE s_axilite port = e_client_
#pragma HLS INTERFACE s_axilite port = b_core_
#pragma HLS INTERFACE s_axilite port = e_core_
#pragma HLS INTERFACE s_axilite port = ip_id_

//#pragma HLS RESOURCE variable = merge_weights core=RAM latency = 4
//#pragma HLS RESOURCE variable = weights core=RAM latency = 3
//#pragma HLS RESOURCE variable = bit_clusters core=RAM  latency = 3
//#pragma HLS RESOURCE variable = cluster.clusters latency=3
//#pragma HLS RESOURCE variable = hard_partition latency = 3
//
	int ccc = 0;

	uint32 *total_offset;
	uint32 *m1_addr = m1 + 0x40000;
	uint32 *cdma_status;
	BASE_ADDR = BASE_ADDR_;
	CLIENT_NUM = CLIENT_NUM_;
	SLOT_NUM = SLOT_NUM_;
	CORE_NUM = CORE_NUM_;
	MESS_NUM = MESS_NUM_;
	b_client = b_client_;
	e_client = e_client_;
	b_core = b_core_;
	e_core = e_core_;
	ip_id = ip_id_;

	total_offset = m1 + (SLOT_LEN * SLOT_NUM * CLIENT_NUM / 2) / 4 + 0x40000; // 1m offset + recv size
	my_client_num = e_client_ - b_client_;
	QUEUE_ADDR_BASE = b_client * SLOT_NUM * SLOT_LEN; // request buff

	uint32 addr = (SLOT_LEN * SLOT_NUM * CLIENT_NUM) + MESSAGE_ADDR; // queue buffer
																	 //	printf("doorbell_addr=%d\n",addr);
	for (int i = 0; i < CORE_NUM; i++)
	{
#pragma HLS PIPELINE
		queue_metadata[i][0] = 0;
		int parallel_offset = (MESS_NUM * MESS_LEN * ip_id) / 2;
		queue_addr_map[i] = (addr + QUEUE_METADATA + parallel_offset) / 4;
		addr += TOTAL_IP_MESS_N * MESS_LEN + QUEUE_METADATA;
		//		printf("i=%d %d\n", i, (int)cluster.clusters[i][i]);
	}

	for (int i = 0; i < my_client_num; i++)
	{

		for (int j = 0; j < SLOT_NUM; j++)
		{
#pragma HLS PIPELINE
			if (i * 2 < my_client_num)
			{
				slot_offset0[i * SLOT_NUM + j] = (SLOT_LEN * (i * SLOT_NUM + j) + QUEUE_ADDR_BASE) / 4 + 16;
				RRB0[i * SLOT_NUM + j] = 0;
				RRB1[i * SLOT_NUM + j] = 0;
			}
			else
			{
				slot_offset1[(i - my_client_num / 2) * SLOT_NUM + j] = (SLOT_LEN * ((i - my_client_num / 2) * SLOT_NUM + j) + QUEUE_ADDR_BASE) / 4 + 16;
			}
			old_core_id[i * SLOT_NUM + j] = CORE_NUM; /*CORE_NUM*/
												//			slot_in_flight_flag[i][j] = 0;
		}
	}
	max_slot = SLOT_NUM * my_client_num;
	slot_partition0 = max_slot / 2;
	slot_partition1 = max_slot / 2;

	//	for (int p = 0; p < FEATURE_SIZE / 8; p++)
	//	{
	////#pragma HLS PIPELINE
	//		merge_weights[p][0] = 0;
	//	}

	cdma[CDMA_OFFSET] = 0x00000004;
	cdma[CDMA_OFFSET] = 0x00007000;
	if (con == 0x00000002)
	{

		ii = 0;
		jj = 0;
		kk = 0;
		int u_w = 1024;
	LOOP_WHILETRUE:
		while (true)
		{
			switch (u_w % 2)
			{
			case 0:
			{
				if (u_w == 1024)
				{
					u_w = 0;
					if (*total_offset != 666)
					{
						return;
					}
					if ((cdma[CDMA_OFFSET + 1] & 0x2) == 0x2)
					{
						init_weight(cdma, cdma_buf);
					}
				}
				else
				{
					init_bitmap();
					u_w++;
				}
				break;
			}
			case 1:
				for (int it = 0; it < 1024; it++)
				{
					dataflow_no_dma(m0, m1_addr, hostm);
				}
				u_w++;
//								return; // C simulation
				break;
			}
		}
		return;
	}
	else if (con == 0x00000001)
	{
		uint32 buff[N + 2]; // for test = 1
		memcpy(buff, (m0 + (BASE_ADDR / 4)), (N + 2) * sizeof(uint32));
		buff[0] += 2;
		for (int i = 0; i < N - 1; i++)
		{
			buff[i + 1] = buff[i] + 2;
		}
		memcpy((m0 + (BASE_ADDR / 4)), buff, (N + 2) * sizeof(uint32));
		cdma_buf[0x200000] = 0xFF;
		// memcpy((cdma_buf + 0x80000, buff, (N + 2) * sizeof(uint32));
		cdma[CDMA_OFFSET + 0x6] = 0x200000;
		cdma[CDMA_OFFSET + 0x8] = MESSAGE_ADDR;
		cdma[CDMA_OFFSET + 10] = FEATURE_SIZE;
		return;
	}
	return;
}
