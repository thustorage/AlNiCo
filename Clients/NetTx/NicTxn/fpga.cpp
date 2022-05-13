
#include "fpga.h"
int fpga_fd;
volatile fpga_dispatch_queue_t* fpga_dispatch_queue;
uint32_t* control_addr;
// volatile uint32_t fpga_init_over;
uint64_t fpga_mm;
uint64_t control_mm;
uint64_t ddio_mm; // not sure whether ddio is ok 
uint64_t host_mm;
uint64_t reply_mm = 0;
int operation_fd;
uint64_t fpga_operation_mm;
uint64_t parallel_ip_addr;
const uint32_t fpga_rev_size = SLOT_LEN * SLOT_NUM * MAX_CLIENT * CLIENT_THREAD;