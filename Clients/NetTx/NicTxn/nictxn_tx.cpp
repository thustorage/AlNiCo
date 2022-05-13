
#include "nictxn_rpc.h"
void nictxn_tx::sys_execution(uint64_t call_type, void* ctx)
{
    // usleep(20);
    // sleep(30);
    switch (call_type) {
    case SINGLE_TX_TPCC_NEW_ORDER: {
        tpcc_procedure();
        return;
        break;
    }
    case SINGLE_TX_TEST: {
        single_tx_test();
        return;
        break;
    }
    default:
        Debug::notifyError("why i am here?");
        break;
    }
    
    execution(call_type, ctx);
    return;
}