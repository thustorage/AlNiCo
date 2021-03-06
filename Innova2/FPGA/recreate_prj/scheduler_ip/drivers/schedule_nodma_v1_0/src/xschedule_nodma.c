// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2018.2
// Copyright (C) 1986-2018 Xilinx, Inc. All Rights Reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xschedule_nodma.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSchedule_nodma_CfgInitialize(XSchedule_nodma *InstancePtr, XSchedule_nodma_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Axilites_BaseAddress = ConfigPtr->Axilites_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSchedule_nodma_Start(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL) & 0x80;
    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSchedule_nodma_IsDone(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSchedule_nodma_IsIdle(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSchedule_nodma_IsReady(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSchedule_nodma_EnableAutoRestart(XSchedule_nodma *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL, 0x80);
}

void XSchedule_nodma_DisableAutoRestart(XSchedule_nodma *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_AP_CTRL, 0);
}

void XSchedule_nodma_Set_con(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CON_DATA, Data);
}

u32 XSchedule_nodma_Get_con(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CON_DATA);
    return Data;
}

void XSchedule_nodma_Set_BASE_ADDR_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_BASE_ADDR_S_DATA, Data);
}

u32 XSchedule_nodma_Get_BASE_ADDR_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_BASE_ADDR_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_CLIENT_NUM_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CLIENT_NUM_S_DATA, Data);
}

u32 XSchedule_nodma_Get_CLIENT_NUM_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CLIENT_NUM_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_SLOT_NUM_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_SLOT_NUM_S_DATA, Data);
}

u32 XSchedule_nodma_Get_SLOT_NUM_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_SLOT_NUM_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_CORE_NUM_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CORE_NUM_S_DATA, Data);
}

u32 XSchedule_nodma_Get_CORE_NUM_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_CORE_NUM_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_MESS_NUM_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_MESS_NUM_S_DATA, Data);
}

u32 XSchedule_nodma_Get_MESS_NUM_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_MESS_NUM_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_b_client_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_B_CLIENT_S_DATA, Data);
}

u32 XSchedule_nodma_Get_b_client_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_B_CLIENT_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_e_client_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_E_CLIENT_S_DATA, Data);
}

u32 XSchedule_nodma_Get_e_client_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_E_CLIENT_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_b_core_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_B_CORE_S_DATA, Data);
}

u32 XSchedule_nodma_Get_b_core_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_B_CORE_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_e_core_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_E_CORE_S_DATA, Data);
}

u32 XSchedule_nodma_Get_e_core_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_E_CORE_S_DATA);
    return Data;
}

void XSchedule_nodma_Set_ip_id_s(XSchedule_nodma *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IP_ID_S_DATA, Data);
}

u32 XSchedule_nodma_Get_ip_id_s(XSchedule_nodma *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IP_ID_S_DATA);
    return Data;
}

void XSchedule_nodma_InterruptGlobalEnable(XSchedule_nodma *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_GIE, 1);
}

void XSchedule_nodma_InterruptGlobalDisable(XSchedule_nodma *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_GIE, 0);
}

void XSchedule_nodma_InterruptEnable(XSchedule_nodma *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IER);
    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IER, Register | Mask);
}

void XSchedule_nodma_InterruptDisable(XSchedule_nodma *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IER);
    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IER, Register & (~Mask));
}

void XSchedule_nodma_InterruptClear(XSchedule_nodma *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSchedule_nodma_WriteReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_ISR, Mask);
}

u32 XSchedule_nodma_InterruptGetEnabled(XSchedule_nodma *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_IER);
}

u32 XSchedule_nodma_InterruptGetStatus(XSchedule_nodma *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSchedule_nodma_ReadReg(InstancePtr->Axilites_BaseAddress, XSCHEDULE_NODMA_AXILITES_ADDR_ISR);
}

