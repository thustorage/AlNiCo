#include "Rdma.h"
bool createContext(RdmaContext* context, uint8_t port, int gidIndex,
    uint8_t devIndex)
{

    ibv_device* dev = NULL;
    ibv_context* ctx = NULL;
    ibv_pd* pd = NULL;
    ibv_port_attr portAttr;

    // get device names in the system
    int devicesNum;
    struct ibv_device** deviceList = ibv_get_device_list(&devicesNum);
    if (!deviceList) {
        Debug::notifyError("failed to get IB devices list");
        goto CreateResourcesExit;
    }

    // if there isn't any IB device in host
    if (!devicesNum) {
        Debug::notifyInfo("found %d device(s)", devicesNum);
        goto CreateResourcesExit;
    }
    // Debug::notifyInfo("Open IB Device");
    // find devindex
    for (int i = 0; i < devicesNum; ++i){
        if (devIndex == (uint8_t)-1) {
            break;
        }
        if (ibv_get_device_name(deviceList[i])[3] == '5' && ibv_get_device_name(deviceList[i])[5] == '0' + devIndex) {
            devIndex = i;
            break;
        }
    }
    // find zhe first
    for (int i = 0; i < devicesNum; ++i) {
        if (devIndex != (uint8_t)-1) {
            break;
        }
        printf("Device %d: %s\n", i, ibv_get_device_name(deviceList[i]));
        if (ibv_get_device_name(deviceList[i])[3] == '5') {
            devIndex = i;
            break;
        }
    }
    if (devIndex >= devicesNum) {
        Debug::notifyError("ib device wasn't found %d %d ",devIndex,devicesNum);
        goto CreateResourcesExit;
    }

    dev = deviceList[devIndex];
    // Debug::notifyInfo("!!!!!!!!!!!!!!!%d",devIndex);
    

    // get device handle
    ctx = ibv_open_device(dev);
    if (!ctx) {
        Debug::notifyError("failed to open device %s");
        goto CreateResourcesExit;
    }
    /* We are now done with device list, free it */
    ibv_free_device_list(deviceList);
    deviceList = NULL;

    // query port properties
    if (ibv_query_port(ctx, port, &portAttr)) {
        Debug::notifyError("ibv_query_port failed");
        goto CreateResourcesExit;
    }

    // allocate Protection Domain
    // Debug::notifyInfo("Allocate Protection Domain");
    pd = ibv_alloc_pd(ctx);
    if (!pd) {
        Debug::notifyError("ibv_alloc_pd failed");
        goto CreateResourcesExit;
    }

    if (ibv_query_gid(ctx, port, gidIndex, &context->gid)) {
        Debug::notifyError("could not get gid for port: %d, gidIndex: %d", port,
            gidIndex);
        goto CreateResourcesExit;
    }
    for (int i = 0; i<16; i++){
        printf("%x ",context->gid.raw[i]);
    }
    printf("%lx %lx\n",context->gid.global.subnet_prefix,context->gid.global.interface_id);
    puts("");


    // Success :)
    context->devIndex = devIndex;
    context->gidIndex = gidIndex;
    context->port = port;
    context->ctx = ctx;
    context->pd = pd;
    context->lid = portAttr.lid;
    
    printf("I open %d %s lid=%d:)\n", devIndex, ibv_get_device_name(dev), context->lid);

    return true;

/* Error encountered, cleanup */
CreateResourcesExit:
    Debug::notifyError("Error Encountered, Cleanup ...");

    if (pd) {
        ibv_dealloc_pd(pd);
        pd = NULL;
    }
    if (ctx) {
        ibv_close_device(ctx);
        ctx = NULL;
    }
    if (deviceList) {
        ibv_free_device_list(deviceList);
        deviceList = NULL;
    }

    return false;
}

bool destoryContext(RdmaContext* context)
{
    bool rc = true;
    if (context->pd) {
        if (ibv_dealloc_pd(context->pd)) {
            Debug::notifyError("Failed to deallocate PD");
            rc = false;
        }
    }
    if (context->ctx) {
        if (ibv_close_device(context->ctx)) {
            Debug::notifyError("failed to close device context");
            rc = false;
        }
    }

    return rc;
}

ibv_mr* createMemoryRegion(uint64_t mm, uint64_t mmSize, RdmaContext* ctx)
{

    // register the memory buffer
    // Debug::notifyInfo("Register Memory Region");
    ibv_mr* mr = NULL;
    mr = ibv_reg_mr(ctx->pd, (void*)mm, mmSize,
        IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_ATOMIC);

    if (!mr) {
        Debug::notifyError("Memory registration failed %llx %d", mm, mmSize);
    }

    return mr;
}

bool createQueuePair(ibv_qp** qp, ibv_qp_type mode, ibv_cq* send_cq,
    ibv_cq* recv_cq, RdmaContext* context,
    uint32_t qpsMaxDepth, uint32_t maxInlineData)
{

    struct ibv_qp_init_attr_ex attr;
    memset(&attr, 0, sizeof(attr));

    attr.qp_type = mode;
    attr.sq_sig_all = 0;
    attr.send_cq = send_cq;
    attr.recv_cq = recv_cq;
    attr.pd = context->pd;
    attr.comp_mask = IBV_QP_INIT_ATTR_PD;

    attr.cap.max_send_wr = (2 * BatchPollSend) > qpsMaxDepth ?  (2 * BatchPollSend) : qpsMaxDepth ;
    attr.cap.max_recv_wr = (2 * BatchPostRec) > qpsMaxDepth ?  (2 * BatchPostRec) : qpsMaxDepth ;
    attr.cap.max_send_sge = 1;
    attr.cap.max_recv_sge = 1;
    attr.cap.max_inline_data = maxInlineData;

    *qp = ibv_create_qp_ex(context->ctx, &attr);
    if (!(*qp)) {
        Debug::notifyError("Failed to create QP");
        return false;
    }

    // Debug::notifyInfo("Create Queue Pair with Num = %d", (*qp)->qp_num);

    return true;
}

bool createQueuePair(ibv_qp** qp, ibv_qp_type mode, ibv_cq* cq,
    RdmaContext* context, uint32_t qpsMaxDepth,
    uint32_t maxInlineData)
{
    return createQueuePair(qp, mode, cq, cq, context, qpsMaxDepth,
        maxInlineData);
}

bool createDCTarget(ibv_exp_dct** dct, ibv_cq* cq, RdmaContext* context,
    uint32_t qpsMaxDepth, uint32_t maxInlineData)
{

    // construct SRQ fot DC Target :)
    struct ibv_srq_init_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.attr.max_wr = qpsMaxDepth;
    attr.attr.max_sge = 1;
    ibv_srq* srq = ibv_create_srq(context->pd, &attr);

    ibv_exp_dct_init_attr dAttr;
    memset(&dAttr, 0, sizeof(dAttr));
    dAttr.gid_index = context->gidIndex;
    dAttr.pd = context->pd;
    dAttr.cq = cq;
    dAttr.srq = srq;
    dAttr.dc_key = DCT_ACCESS_KEY;
    dAttr.port = context->port;
    dAttr.access_flags = IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_ATOMIC;
    // dAttr.access_flags = IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_READ |
    //                      IBV_ACCESS_REMOTE_ATOMIC;
    dAttr.min_rnr_timer = 2;
    dAttr.tclass = 0;
    dAttr.flow_label = 0;
    dAttr.mtu = IBV_MTU_4096;
    dAttr.pkey_index = 0;
    dAttr.hop_limit = 1;
    dAttr.create_flags = 0;
    dAttr.inline_size = maxInlineData;

    *dct = ibv_exp_create_dct(context->ctx, &dAttr);
    if (dct == NULL) {
        Debug::notifyError("failed to create dc target");
        return false;
    }

    return true;
}

void fillAhAttr(ibv_ah_attr* attr, uint32_t remoteLid, uint8_t* remoteGid, 
    RdmaContext* context, uint64_t interface_id)
{

    (void)remoteGid;

    memset(attr, 0, sizeof(ibv_ah_attr));
    attr->dlid = remoteLid;
    attr->sl = 0;
    attr->src_path_bits = 0;
    attr->port_num = context->port;

    // attr->is_global = 0;

    if (remoteGid[15] != 0){
    // fill ah_attr with GRH
        // printf("GID RoCE");
        // attr->grh.dgid.global.interface_id = interface_id;
        printf("GID RoCE %lx %lx\n", attr->grh.dgid.global.interface_id, attr->grh.dgid.global.subnet_prefix);
        attr->is_global = 1;
        memcpy(&attr->grh.dgid.raw, remoteGid, 16);
        attr->grh.flow_label = 0;
        attr->grh.hop_limit = 1;
        attr->grh.sgid_index = context->gidIndex;
        attr->grh.traffic_class = 0;
    }
}


void checkDctSupported(struct ibv_context *ctx) {
    printf("Checking if DCT is supported.. ");
    struct ibv_exp_device_attr dattr;

    dattr.comp_mask = IBV_EXP_DEVICE_ATTR_EXP_CAP_FLAGS |
                      IBV_EXP_DEVICE_DC_RD_REQ | IBV_EXP_DEVICE_DC_RD_RES;
    int err = ibv_exp_query_device(ctx, &dattr);
    if (err) {
        printf("couldn't query device extended attributes\n");
        assert(false);
    } else {
        if (!(dattr.comp_mask & IBV_EXP_DEVICE_ATTR_EXP_CAP_FLAGS)) {
            printf("no extended capability flgas\n");
            assert(false);
        }
        if (!(dattr.exp_device_cap_flags & IBV_EXP_DEVICE_DC_TRANSPORT)) {
            printf("DC transport not enabled\n");
            assert(false);
        }

        if (!(dattr.comp_mask & IBV_EXP_DEVICE_DC_RD_REQ)) {
            printf("no report on max requestor rdma/atomic resources\n");
            assert(false);
        }

        if (!(dattr.comp_mask & IBV_EXP_DEVICE_DC_RD_RES)) {
            printf("no report on max responder rdma/atomic resources\n");
            assert(false);
        }
    }
    printf("Success\n");
}