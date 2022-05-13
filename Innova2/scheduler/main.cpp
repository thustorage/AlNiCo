#include "utils.h"
#include <ap_int.h>
int main()
{
	int i;
	int *cdma;
	ap_uint<8> *cdma_buf;
	int ok = 0;
	uint32 MAX_CLIENT = 2;
	uint32 CLIENT_THREAD = 24;
	uint32 SLOT_NUM = 2;
	uint32 CORE_NUM = 20;
	uint32 MESS_NUM = 256;
	uint32 slot_len = 2048;
	uint32 mess_len = 64;
	uint32 max_core = 24;
	uint32 fpga_rev_size = slot_len * SLOT_NUM * MAX_CLIENT * CLIENT_THREAD;
	long long total = fpga_rev_size + (MESS_NUM + 4) * mess_len * max_core + 8192;
	puts("???");
	printf("%u\n", MAX_CLIENT);
	total = 33570816ull; //128M + 64K
	printf("%u + %u = size = %u\n", fpga_rev_size, (MESS_NUM + 4) * mess_len * max_core, total);
	uint32 *mm = (uint32 *)malloc(total * sizeof(uint32));
	memset(mm, 0, sizeof(uint32) * total);



	puts("\n--------------------TEST 1 START");
	for (i = 0; i < 10; i++)
	{
		printf("%u ", mm[i + 16000]);
	}
	puts("");
	mm[16000] = 0;
	schedule_nodma(mm, mm, mm, mm, (ap_uint<8> *)mm, 1, 16000 * 4, 1, 4, 1, 15, 0, 0, 0, 0, 0);
	for (i = 0; i < 10; i++)
	{
		printf("%u ", mm[i + 16000]);
	}
	puts("\n--------------------TEST 1 PASS\n");
	//
	puts("\n--------------------TEST 2 START");
	memset(mm, 0, sizeof(uint32) * total);
	cdma = mm + 0x02000000;
	cdma_buf = (ap_uint<8> *)((unsigned long long)mm) + 0x200000;
	for (int i = 0; i < 20; i++){
		cdma_buf[i + 35 + 512] = i;
		cdma_buf[i+35] = 100;
	}

	uint32 size = 124;
	uint32 magic = 111;
//	mm[0] = 1;
//	mm[1] = 0;
	short *x = (short *)(mm+512 + 0x40000);
//	mm[512] = 0x23000000;
//	mm[513] = 0;
	x[32] = 0x0;
	x[33] = 0xFFFF;
	x[2] = 0x0010;
	x[3] = 0x0000;
//	mm[4] = size;
//	mm[5] = magic;
//	mm[6] = 0;
	uint32 *tail_magic = (mm + size / 4);
	*tail_magic = magic;

	uint32 *control_addr = mm + (fpga_rev_size/2) / 4 + 0x40000;
	printf("my-control %llu\n", fpga_rev_size + (MESS_NUM + 4) * mess_len * max_core);
	*control_addr = 666;

	//	uint32* pp = control_addr + 1;

	uint32 *f_to_c = mm + fpga_rev_size / 4 + 0x01000000 + QUEUE_METADATA / 4 + ((MESS_NUM + 4) * mess_len/4);

	f_to_c[0] = 0xFFFFFFFF;
	f_to_c[1] = 0;
	f_to_c[2] = 0;
	f_to_c[16] = 100;

	printf("before func %lu %lu \n", *tail_magic, f_to_c[0]);
	//	for (i = 0; i < 10; i++){
	//			printf("%u ",pp[i]);
	//		}
	puts("");
	mm[0x02000000 + 1] = 0x2;
	cdma[1] = 0x2;
	schedule_nodma(mm, mm, mm, mm, (ap_uint<8> *)mm, 2, fpga_rev_size, MAX_CLIENT * CLIENT_THREAD, SLOT_NUM, CORE_NUM, MESS_NUM,
							0, MAX_CLIENT * CLIENT_THREAD, 0, CORE_NUM, 0);

	printf("offset = %d\n",fpga_rev_size / 4 + 0x01000000 + QUEUE_METADATA / 4 + ((MESS_NUM + 4) * mess_len/4));
	printf("after func %lu %lu\n", *tail_magic, f_to_c[0]);


	//	for (i = 0; i < 10; i++){
	//		printf("%u ",pp[i]);
	//	}
	if (f_to_c[0] == 0)
	{
		puts("\n--------------------TEST 2_0 PASS");
	}
	return ok;

	// Return 0 if the test passes
}
