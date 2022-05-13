// ==============================================================
// RTL generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2018.2
// Copyright (C) 1986-2018 Xilinx, Inc. All Rights Reserved.
// 
// ===========================================================

`timescale 1 ns / 1 ps 

module load_feature_1 (
        ap_clk,
        ap_rst,
        ap_start,
        ap_done,
        ap_idle,
        ap_ready,
        m_axi_m_d_AWVALID,
        m_axi_m_d_AWREADY,
        m_axi_m_d_AWADDR,
        m_axi_m_d_AWID,
        m_axi_m_d_AWLEN,
        m_axi_m_d_AWSIZE,
        m_axi_m_d_AWBURST,
        m_axi_m_d_AWLOCK,
        m_axi_m_d_AWCACHE,
        m_axi_m_d_AWPROT,
        m_axi_m_d_AWQOS,
        m_axi_m_d_AWREGION,
        m_axi_m_d_AWUSER,
        m_axi_m_d_WVALID,
        m_axi_m_d_WREADY,
        m_axi_m_d_WDATA,
        m_axi_m_d_WSTRB,
        m_axi_m_d_WLAST,
        m_axi_m_d_WID,
        m_axi_m_d_WUSER,
        m_axi_m_d_ARVALID,
        m_axi_m_d_ARREADY,
        m_axi_m_d_ARADDR,
        m_axi_m_d_ARID,
        m_axi_m_d_ARLEN,
        m_axi_m_d_ARSIZE,
        m_axi_m_d_ARBURST,
        m_axi_m_d_ARLOCK,
        m_axi_m_d_ARCACHE,
        m_axi_m_d_ARPROT,
        m_axi_m_d_ARQOS,
        m_axi_m_d_ARREGION,
        m_axi_m_d_ARUSER,
        m_axi_m_d_RVALID,
        m_axi_m_d_RREADY,
        m_axi_m_d_RDATA,
        m_axi_m_d_RLAST,
        m_axi_m_d_RID,
        m_axi_m_d_RUSER,
        m_axi_m_d_RRESP,
        m_axi_m_d_BVALID,
        m_axi_m_d_BREADY,
        m_axi_m_d_BRESP,
        m_axi_m_d_BID,
        m_axi_m_d_BUSER,
        offset,
        ap_return,
        m_d_blk_n_AR,
        m_d_blk_n_R
);

parameter    ap_ST_fsm_pp0_stage0 = 4'd8;
parameter    ap_ST_fsm_pp0_stage1 = 4'd0;
parameter    ap_ST_fsm_pp0_stage2 = 4'd1;
parameter    ap_ST_fsm_pp0_stage3 = 4'd3;
parameter    ap_ST_fsm_pp0_stage4 = 4'd2;
parameter    ap_ST_fsm_pp0_stage5 = 4'd6;
parameter    ap_ST_fsm_pp0_stage6 = 4'd7;
parameter    ap_ST_fsm_pp0_stage7 = 4'd5;
parameter    ap_ST_fsm_pp0_stage8 = 4'd4;
parameter    ap_ST_fsm_pp0_stage9 = 4'd12;
parameter    ap_ST_fsm_pp0_stage10 = 4'd13;
parameter    ap_ST_fsm_pp0_stage11 = 4'd15;
parameter    ap_ST_fsm_pp0_stage12 = 4'd14;
parameter    ap_ST_fsm_pp0_stage13 = 4'd10;
parameter    ap_ST_fsm_pp0_stage14 = 4'd11;
parameter    ap_ST_fsm_pp0_stage15 = 4'd9;

input   ap_clk;
input   ap_rst;
input   ap_start;
output   ap_done;
output   ap_idle;
output   ap_ready;
output   m_axi_m_d_AWVALID;
input   m_axi_m_d_AWREADY;
output  [31:0] m_axi_m_d_AWADDR;
output  [0:0] m_axi_m_d_AWID;
output  [31:0] m_axi_m_d_AWLEN;
output  [2:0] m_axi_m_d_AWSIZE;
output  [1:0] m_axi_m_d_AWBURST;
output  [1:0] m_axi_m_d_AWLOCK;
output  [3:0] m_axi_m_d_AWCACHE;
output  [2:0] m_axi_m_d_AWPROT;
output  [3:0] m_axi_m_d_AWQOS;
output  [3:0] m_axi_m_d_AWREGION;
output  [0:0] m_axi_m_d_AWUSER;
output   m_axi_m_d_WVALID;
input   m_axi_m_d_WREADY;
output  [31:0] m_axi_m_d_WDATA;
output  [3:0] m_axi_m_d_WSTRB;
output   m_axi_m_d_WLAST;
output  [0:0] m_axi_m_d_WID;
output  [0:0] m_axi_m_d_WUSER;
output   m_axi_m_d_ARVALID;
input   m_axi_m_d_ARREADY;
output  [31:0] m_axi_m_d_ARADDR;
output  [0:0] m_axi_m_d_ARID;
output  [31:0] m_axi_m_d_ARLEN;
output  [2:0] m_axi_m_d_ARSIZE;
output  [1:0] m_axi_m_d_ARBURST;
output  [1:0] m_axi_m_d_ARLOCK;
output  [3:0] m_axi_m_d_ARCACHE;
output  [2:0] m_axi_m_d_ARPROT;
output  [3:0] m_axi_m_d_ARQOS;
output  [3:0] m_axi_m_d_ARREGION;
output  [0:0] m_axi_m_d_ARUSER;
input   m_axi_m_d_RVALID;
output   m_axi_m_d_RREADY;
input  [31:0] m_axi_m_d_RDATA;
input   m_axi_m_d_RLAST;
input  [0:0] m_axi_m_d_RID;
input  [0:0] m_axi_m_d_RUSER;
input  [1:0] m_axi_m_d_RRESP;
input   m_axi_m_d_BVALID;
output   m_axi_m_d_BREADY;
input  [1:0] m_axi_m_d_BRESP;
input  [0:0] m_axi_m_d_BID;
input  [0:0] m_axi_m_d_BUSER;
input  [31:0] offset;
output  [511:0] ap_return;
output   m_d_blk_n_AR;
output   m_d_blk_n_R;

reg ap_done;
reg ap_idle;
reg ap_ready;
reg m_axi_m_d_ARVALID;
reg m_axi_m_d_RREADY;
reg m_d_blk_n_AR;
reg m_d_blk_n_R;

reg   [3:0] ap_CS_fsm;
reg    ap_enable_reg_pp0_iter0;
wire    ap_block_pp0_stage0;
reg    ap_enable_reg_pp0_iter1;
reg    ap_idle_pp0;
reg    ap_block_state16_pp0_stage15_iter0;
reg    ap_block_pp0_stage15_11001;
wire    ap_block_pp0_stage2;
wire    ap_block_pp0_stage9;
wire    ap_block_pp0_stage10;
wire    ap_block_pp0_stage11;
wire    ap_block_pp0_stage12;
wire    ap_block_pp0_stage13;
wire    ap_block_pp0_stage14;
wire    ap_block_pp0_stage15;
wire    ap_block_pp0_stage1;
wire    ap_block_pp0_stage3;
wire    ap_block_pp0_stage4;
wire    ap_block_pp0_stage5;
wire    ap_block_pp0_stage6;
wire    ap_block_pp0_stage7;
wire    ap_block_pp0_stage8;
wire    ap_block_state3_pp0_stage2_iter0;
reg    ap_sig_ioackin_m_axi_m_d_ARREADY;
reg    ap_block_state19_pp0_stage2_iter1;
reg    ap_block_pp0_stage2_11001;
reg   [31:0] m_d_addr_read_reg_96;
reg    ap_block_state10_pp0_stage9_iter0;
reg    ap_block_pp0_stage9_11001;
reg   [31:0] m_d_addr_read_1_reg_101;
reg    ap_block_state11_pp0_stage10_iter0;
reg    ap_block_pp0_stage10_11001;
reg   [31:0] m_d_addr_read_2_reg_106;
reg    ap_block_state12_pp0_stage11_iter0;
reg    ap_block_pp0_stage11_11001;
reg   [31:0] m_d_addr_read_3_reg_111;
reg    ap_block_state13_pp0_stage12_iter0;
reg    ap_block_pp0_stage12_11001;
reg   [31:0] m_d_addr_read_4_reg_116;
reg    ap_block_state14_pp0_stage13_iter0;
reg    ap_block_pp0_stage13_11001;
reg   [31:0] m_d_addr_read_5_reg_121;
reg    ap_block_state15_pp0_stage14_iter0;
reg    ap_block_pp0_stage14_11001;
reg   [31:0] m_d_addr_read_6_reg_126;
reg   [31:0] m_d_addr_read_7_reg_131;
reg    ap_block_state1_pp0_stage0_iter0;
reg    ap_block_state17_pp0_stage0_iter1;
reg    ap_block_pp0_stage0_11001;
reg   [31:0] m_d_addr_read_8_reg_136;
wire    ap_block_state2_pp0_stage1_iter0;
reg    ap_block_state18_pp0_stage1_iter1;
reg    ap_block_pp0_stage1_11001;
reg   [31:0] m_d_addr_read_9_reg_141;
reg   [31:0] m_d_addr_read_10_reg_146;
wire    ap_block_state4_pp0_stage3_iter0;
reg    ap_block_state20_pp0_stage3_iter1;
reg    ap_block_pp0_stage3_11001;
reg   [31:0] m_d_addr_read_11_reg_151;
wire    ap_block_state5_pp0_stage4_iter0;
reg    ap_block_state21_pp0_stage4_iter1;
reg    ap_block_pp0_stage4_11001;
reg   [31:0] m_d_addr_read_12_reg_156;
wire    ap_block_state6_pp0_stage5_iter0;
reg    ap_block_state22_pp0_stage5_iter1;
reg    ap_block_pp0_stage5_11001;
reg   [31:0] m_d_addr_read_13_reg_161;
wire    ap_block_state7_pp0_stage6_iter0;
reg    ap_block_state23_pp0_stage6_iter1;
reg    ap_block_pp0_stage6_11001;
reg   [31:0] m_d_addr_read_14_reg_166;
wire    ap_block_state8_pp0_stage7_iter0;
reg    ap_block_state24_pp0_stage7_iter1;
reg    ap_block_pp0_stage7_11001;
reg    ap_enable_reg_pp0_iter0_reg;
wire    ap_block_state9_pp0_stage8_iter0;
reg    ap_block_state25_pp0_stage8_iter1;
reg    ap_block_pp0_stage8_subdone;
reg    ap_block_pp0_stage15_subdone;
reg   [31:0] ap_port_reg_offset;
wire  signed [63:0] tmp_s_fu_58_p1;
reg    ap_reg_ioackin_m_axi_m_d_ARREADY;
reg    ap_block_pp0_stage2_01001;
reg    ap_block_pp0_stage8_11001;
reg   [3:0] ap_NS_fsm;
reg    ap_block_pp0_stage0_subdone;
reg    ap_idle_pp0_1to1;
reg    ap_block_pp0_stage1_subdone;
reg    ap_block_pp0_stage2_subdone;
reg    ap_block_pp0_stage3_subdone;
reg    ap_block_pp0_stage4_subdone;
reg    ap_block_pp0_stage5_subdone;
reg    ap_block_pp0_stage6_subdone;
reg    ap_block_pp0_stage7_subdone;
reg    ap_idle_pp0_0to0;
reg    ap_reset_idle_pp0;
reg    ap_block_pp0_stage9_subdone;
reg    ap_block_pp0_stage10_subdone;
reg    ap_block_pp0_stage11_subdone;
reg    ap_block_pp0_stage12_subdone;
reg    ap_block_pp0_stage13_subdone;
reg    ap_block_pp0_stage14_subdone;
wire    ap_enable_pp0;

// power-on initialization
initial begin
#0 ap_CS_fsm = 4'd8;
#0 ap_enable_reg_pp0_iter1 = 1'b0;
#0 m_d_addr_read_reg_96 = 32'd0;
#0 m_d_addr_read_1_reg_101 = 32'd0;
#0 m_d_addr_read_2_reg_106 = 32'd0;
#0 m_d_addr_read_3_reg_111 = 32'd0;
#0 m_d_addr_read_4_reg_116 = 32'd0;
#0 m_d_addr_read_5_reg_121 = 32'd0;
#0 m_d_addr_read_6_reg_126 = 32'd0;
#0 m_d_addr_read_7_reg_131 = 32'd0;
#0 m_d_addr_read_8_reg_136 = 32'd0;
#0 m_d_addr_read_9_reg_141 = 32'd0;
#0 m_d_addr_read_10_reg_146 = 32'd0;
#0 m_d_addr_read_11_reg_151 = 32'd0;
#0 m_d_addr_read_12_reg_156 = 32'd0;
#0 m_d_addr_read_13_reg_161 = 32'd0;
#0 m_d_addr_read_14_reg_166 = 32'd0;
#0 ap_enable_reg_pp0_iter0_reg = 1'b0;
#0 ap_port_reg_offset = 32'd0;
#0 ap_reg_ioackin_m_axi_m_d_ARREADY = 1'b0;
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        ap_CS_fsm <= ap_ST_fsm_pp0_stage0;
    end else begin
        ap_CS_fsm <= ap_NS_fsm;
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        ap_enable_reg_pp0_iter0_reg <= 1'b0;
    end else begin
        if ((ap_ST_fsm_pp0_stage0 == ap_CS_fsm)) begin
            ap_enable_reg_pp0_iter0_reg <= ap_start;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        ap_enable_reg_pp0_iter1 <= 1'b0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage15_subdone) & (ap_ST_fsm_pp0_stage15 == ap_CS_fsm))) begin
            ap_enable_reg_pp0_iter1 <= ap_enable_reg_pp0_iter0;
        end else if (((1'b0 == ap_block_pp0_stage8_subdone) & (ap_enable_reg_pp0_iter0 == 1'b0) & (ap_ST_fsm_pp0_stage8 == ap_CS_fsm))) begin
            ap_enable_reg_pp0_iter1 <= 1'b0;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        ap_port_reg_offset <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage0_11001) & (ap_enable_reg_pp0_iter0 == 1'b1))) begin
            ap_port_reg_offset <= offset;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        ap_reg_ioackin_m_axi_m_d_ARREADY <= 1'b0;
    end else begin
        if (((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (ap_enable_reg_pp0_iter0 == 1'b1))) begin
            if ((1'b0 == ap_block_pp0_stage2_11001)) begin
                ap_reg_ioackin_m_axi_m_d_ARREADY <= 1'b0;
            end else if (((1'b0 == ap_block_pp0_stage2_01001) & (m_axi_m_d_ARREADY == 1'b1))) begin
                ap_reg_ioackin_m_axi_m_d_ARREADY <= 1'b1;
            end
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_10_reg_146 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage3 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage3_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_10_reg_146 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_11_reg_151 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage4 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage4_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_11_reg_151 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_12_reg_156 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage5 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage5_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_12_reg_156 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_13_reg_161 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage6 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage6_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_13_reg_161 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_14_reg_166 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage7_11001) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage7 == ap_CS_fsm))) begin
            m_d_addr_read_14_reg_166 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_1_reg_101 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage10_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage10 == ap_CS_fsm))) begin
            m_d_addr_read_1_reg_101 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_2_reg_106 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage11_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage11 == ap_CS_fsm))) begin
            m_d_addr_read_2_reg_106 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_3_reg_111 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage12_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage12 == ap_CS_fsm))) begin
            m_d_addr_read_3_reg_111 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_4_reg_116 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage13_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage13 == ap_CS_fsm))) begin
            m_d_addr_read_4_reg_116 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_5_reg_121 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage14_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage14 == ap_CS_fsm))) begin
            m_d_addr_read_5_reg_121 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_6_reg_126 <= 32'd0;
    end else begin
        if (((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage15 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage15_11001))) begin
            m_d_addr_read_6_reg_126 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_7_reg_131 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage0_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_7_reg_131 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_8_reg_136 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage1 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage1_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_8_reg_136 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_9_reg_141 <= 32'd0;
    end else begin
        if (((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage2_11001) & (ap_enable_reg_pp0_iter1 == 1'b1))) begin
            m_d_addr_read_9_reg_141 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (posedge ap_clk) begin
    if (ap_rst == 1'b1) begin
        m_d_addr_read_reg_96 <= 32'd0;
    end else begin
        if (((1'b0 == ap_block_pp0_stage9_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage9 == ap_CS_fsm))) begin
            m_d_addr_read_reg_96 <= m_axi_m_d_RDATA;
        end
    end
end

always @ (*) begin
    if ((((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage0) & (ap_start == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1)) | ((1'b0 == ap_block_pp0_stage8_11001) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage8 == ap_CS_fsm)))) begin
        ap_done = 1'b1;
    end else begin
        ap_done = 1'b0;
    end
end

always @ (*) begin
    if ((ap_ST_fsm_pp0_stage0 == ap_CS_fsm)) begin
        ap_enable_reg_pp0_iter0 = ap_start;
    end else begin
        ap_enable_reg_pp0_iter0 = ap_enable_reg_pp0_iter0_reg;
    end
end

always @ (*) begin
    if (((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (ap_start == 1'b0) & (ap_idle_pp0 == 1'b1))) begin
        ap_idle = 1'b1;
    end else begin
        ap_idle = 1'b0;
    end
end

always @ (*) begin
    if (((ap_enable_reg_pp0_iter1 == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b0))) begin
        ap_idle_pp0 = 1'b1;
    end else begin
        ap_idle_pp0 = 1'b0;
    end
end

always @ (*) begin
    if ((ap_enable_reg_pp0_iter0 == 1'b0)) begin
        ap_idle_pp0_0to0 = 1'b1;
    end else begin
        ap_idle_pp0_0to0 = 1'b0;
    end
end

always @ (*) begin
    if ((ap_enable_reg_pp0_iter1 == 1'b0)) begin
        ap_idle_pp0_1to1 = 1'b1;
    end else begin
        ap_idle_pp0_1to1 = 1'b0;
    end
end

always @ (*) begin
    if (((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage15 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage15_11001))) begin
        ap_ready = 1'b1;
    end else begin
        ap_ready = 1'b0;
    end
end

always @ (*) begin
    if (((ap_start == 1'b0) & (ap_idle_pp0_0to0 == 1'b1))) begin
        ap_reset_idle_pp0 = 1'b1;
    end else begin
        ap_reset_idle_pp0 = 1'b0;
    end
end

always @ (*) begin
    if ((ap_reg_ioackin_m_axi_m_d_ARREADY == 1'b0)) begin
        ap_sig_ioackin_m_axi_m_d_ARREADY = m_axi_m_d_ARREADY;
    end else begin
        ap_sig_ioackin_m_axi_m_d_ARREADY = 1'b1;
    end
end

always @ (*) begin
    if (((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage2_01001) & (ap_reg_ioackin_m_axi_m_d_ARREADY == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1))) begin
        m_axi_m_d_ARVALID = 1'b1;
    end else begin
        m_axi_m_d_ARVALID = 1'b0;
    end
end

always @ (*) begin
    if ((((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage15 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage15_11001)) | ((1'b0 == ap_block_pp0_stage14_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage14 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage13_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage13 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage12_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage12 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage11_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage11 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage10_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage10 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage9_11001) & (ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage9 == ap_CS_fsm)) | ((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage0_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((1'b0 == ap_block_pp0_stage8_11001) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage8 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage7_11001) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage7 == ap_CS_fsm)) | ((ap_ST_fsm_pp0_stage6 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage6_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage5 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage5_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage4 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage4_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage3 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage3_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage1 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage1_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage2_11001) & (ap_enable_reg_pp0_iter1 == 1'b1)))) begin
        m_axi_m_d_RREADY = 1'b1;
    end else begin
        m_axi_m_d_RREADY = 1'b0;
    end
end

always @ (*) begin
    if (((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (ap_enable_reg_pp0_iter0 == 1'b1) & (1'b0 == ap_block_pp0_stage2))) begin
        m_d_blk_n_AR = m_axi_m_d_ARREADY;
    end else begin
        m_d_blk_n_AR = 1'b1;
    end
end

always @ (*) begin
    if ((((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage15 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage15)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage14 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage14)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage13 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage13)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage12 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage12)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage11 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage11)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage10 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage10)) | ((ap_enable_reg_pp0_iter0 == 1'b1) & (ap_ST_fsm_pp0_stage9 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage9)) | ((ap_ST_fsm_pp0_stage0 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage0) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((1'b0 == ap_block_pp0_stage8) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage8 == ap_CS_fsm)) | ((1'b0 == ap_block_pp0_stage7) & (ap_enable_reg_pp0_iter1 == 1'b1) & (ap_ST_fsm_pp0_stage7 == ap_CS_fsm)) | ((ap_ST_fsm_pp0_stage6 == ap_CS_fsm) & (1'b0 == ap_block_pp0_stage6) & (ap_enable_reg_pp0_iter1 == 1'b1)) | ((ap_ST_fsm_pp0_stage5 == ap_CS_fsm) & (ap_enable_reg_pp0_iter1 == 1'b1) & (1'b0 == ap_block_pp0_stage5)) | ((ap_ST_fsm_pp0_stage4 == ap_CS_fsm) & (ap_enable_reg_pp0_iter1 == 1'b1) & (1'b0 == ap_block_pp0_stage4)) | ((ap_ST_fsm_pp0_stage3 == ap_CS_fsm) & (ap_enable_reg_pp0_iter1 == 1'b1) & (1'b0 == ap_block_pp0_stage3)) | ((ap_ST_fsm_pp0_stage1 == ap_CS_fsm) & (ap_enable_reg_pp0_iter1 == 1'b1) & (1'b0 == ap_block_pp0_stage1)) | ((ap_ST_fsm_pp0_stage2 == ap_CS_fsm) & (ap_enable_reg_pp0_iter1 == 1'b1) & (1'b0 == ap_block_pp0_stage2)))) begin
        m_d_blk_n_R = m_axi_m_d_RVALID;
    end else begin
        m_d_blk_n_R = 1'b1;
    end
end

always @ (*) begin
    case (ap_CS_fsm)
        ap_ST_fsm_pp0_stage0 : begin
            if ((~((ap_start == 1'b0) & (ap_idle_pp0_1to1 == 1'b1)) & (1'b0 == ap_block_pp0_stage0_subdone))) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage1;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage0;
            end
        end
        ap_ST_fsm_pp0_stage1 : begin
            if ((1'b0 == ap_block_pp0_stage1_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage2;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage1;
            end
        end
        ap_ST_fsm_pp0_stage2 : begin
            if ((1'b0 == ap_block_pp0_stage2_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage3;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage2;
            end
        end
        ap_ST_fsm_pp0_stage3 : begin
            if ((1'b0 == ap_block_pp0_stage3_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage4;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage3;
            end
        end
        ap_ST_fsm_pp0_stage4 : begin
            if ((1'b0 == ap_block_pp0_stage4_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage5;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage4;
            end
        end
        ap_ST_fsm_pp0_stage5 : begin
            if ((1'b0 == ap_block_pp0_stage5_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage6;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage5;
            end
        end
        ap_ST_fsm_pp0_stage6 : begin
            if ((1'b0 == ap_block_pp0_stage6_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage7;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage6;
            end
        end
        ap_ST_fsm_pp0_stage7 : begin
            if ((1'b0 == ap_block_pp0_stage7_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage8;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage7;
            end
        end
        ap_ST_fsm_pp0_stage8 : begin
            if (((ap_reset_idle_pp0 == 1'b0) & (1'b0 == ap_block_pp0_stage8_subdone))) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage9;
            end else if (((1'b0 == ap_block_pp0_stage8_subdone) & (ap_reset_idle_pp0 == 1'b1))) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage0;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage8;
            end
        end
        ap_ST_fsm_pp0_stage9 : begin
            if ((1'b0 == ap_block_pp0_stage9_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage10;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage9;
            end
        end
        ap_ST_fsm_pp0_stage10 : begin
            if ((1'b0 == ap_block_pp0_stage10_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage11;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage10;
            end
        end
        ap_ST_fsm_pp0_stage11 : begin
            if ((1'b0 == ap_block_pp0_stage11_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage12;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage11;
            end
        end
        ap_ST_fsm_pp0_stage12 : begin
            if ((1'b0 == ap_block_pp0_stage12_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage13;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage12;
            end
        end
        ap_ST_fsm_pp0_stage13 : begin
            if ((1'b0 == ap_block_pp0_stage13_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage14;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage13;
            end
        end
        ap_ST_fsm_pp0_stage14 : begin
            if ((1'b0 == ap_block_pp0_stage14_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage15;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage14;
            end
        end
        ap_ST_fsm_pp0_stage15 : begin
            if ((1'b0 == ap_block_pp0_stage15_subdone)) begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage0;
            end else begin
                ap_NS_fsm = ap_ST_fsm_pp0_stage15;
            end
        end
        default : begin
            ap_NS_fsm = 'bx;
        end
    endcase
end

assign ap_block_pp0_stage0 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage0_11001 = (((ap_start == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1)) | ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1)));
end

always @ (*) begin
    ap_block_pp0_stage0_subdone = (((ap_start == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1)) | ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1)));
end

assign ap_block_pp0_stage1 = ~(1'b1 == 1'b1);

assign ap_block_pp0_stage10 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage10_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage10_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

assign ap_block_pp0_stage11 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage11_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage11_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

assign ap_block_pp0_stage12 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage12_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage12_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

assign ap_block_pp0_stage13 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage13_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage13_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

assign ap_block_pp0_stage14 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage14_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage14_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

assign ap_block_pp0_stage15 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage15_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage15_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage1_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage1_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage2 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage2_01001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage2_11001 = (((ap_sig_ioackin_m_axi_m_d_ARREADY == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1)) | ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1)));
end

always @ (*) begin
    ap_block_pp0_stage2_subdone = (((ap_sig_ioackin_m_axi_m_d_ARREADY == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1)) | ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1)));
end

assign ap_block_pp0_stage3 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage3_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage3_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage4 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage4_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage4_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage5 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage5_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage5_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage6 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage6_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage6_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage7 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage7_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage7_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage8 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage8_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage8_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter1 == 1'b1));
end

assign ap_block_pp0_stage9 = ~(1'b1 == 1'b1);

always @ (*) begin
    ap_block_pp0_stage9_11001 = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_pp0_stage9_subdone = ((m_axi_m_d_RVALID == 1'b0) & (ap_enable_reg_pp0_iter0 == 1'b1));
end

always @ (*) begin
    ap_block_state10_pp0_stage9_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state11_pp0_stage10_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state12_pp0_stage11_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state13_pp0_stage12_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state14_pp0_stage13_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state15_pp0_stage14_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state16_pp0_stage15_iter0 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state17_pp0_stage0_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state18_pp0_stage1_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state19_pp0_stage2_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state1_pp0_stage0_iter0 = (ap_start == 1'b0);
end

always @ (*) begin
    ap_block_state20_pp0_stage3_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state21_pp0_stage4_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state22_pp0_stage5_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state23_pp0_stage6_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state24_pp0_stage7_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

always @ (*) begin
    ap_block_state25_pp0_stage8_iter1 = (m_axi_m_d_RVALID == 1'b0);
end

assign ap_block_state2_pp0_stage1_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state3_pp0_stage2_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state4_pp0_stage3_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state5_pp0_stage4_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state6_pp0_stage5_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state7_pp0_stage6_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state8_pp0_stage7_iter0 = ~(1'b1 == 1'b1);

assign ap_block_state9_pp0_stage8_iter0 = ~(1'b1 == 1'b1);

assign ap_enable_pp0 = (ap_idle_pp0 ^ 1'b1);

assign ap_return = {{{{{{{{{{{{{{{{m_axi_m_d_RDATA}, {m_d_addr_read_14_reg_166}}, {m_d_addr_read_13_reg_161}}, {m_d_addr_read_12_reg_156}}, {m_d_addr_read_11_reg_151}}, {m_d_addr_read_10_reg_146}}, {m_d_addr_read_9_reg_141}}, {m_d_addr_read_8_reg_136}}, {m_d_addr_read_7_reg_131}}, {m_d_addr_read_6_reg_126}}, {m_d_addr_read_5_reg_121}}, {m_d_addr_read_4_reg_116}}, {m_d_addr_read_3_reg_111}}, {m_d_addr_read_2_reg_106}}, {m_d_addr_read_1_reg_101}}, {m_d_addr_read_reg_96}};

assign m_axi_m_d_ARADDR = tmp_s_fu_58_p1;

assign m_axi_m_d_ARBURST = 2'd0;

assign m_axi_m_d_ARCACHE = 4'd0;

assign m_axi_m_d_ARID = 1'd0;

assign m_axi_m_d_ARLEN = 32'd16;

assign m_axi_m_d_ARLOCK = 2'd0;

assign m_axi_m_d_ARPROT = 3'd0;

assign m_axi_m_d_ARQOS = 4'd0;

assign m_axi_m_d_ARREGION = 4'd0;

assign m_axi_m_d_ARSIZE = 3'd0;

assign m_axi_m_d_ARUSER = 1'd0;

assign m_axi_m_d_AWADDR = 32'd0;

assign m_axi_m_d_AWBURST = 2'd0;

assign m_axi_m_d_AWCACHE = 4'd0;

assign m_axi_m_d_AWID = 1'd0;

assign m_axi_m_d_AWLEN = 32'd0;

assign m_axi_m_d_AWLOCK = 2'd0;

assign m_axi_m_d_AWPROT = 3'd0;

assign m_axi_m_d_AWQOS = 4'd0;

assign m_axi_m_d_AWREGION = 4'd0;

assign m_axi_m_d_AWSIZE = 3'd0;

assign m_axi_m_d_AWUSER = 1'd0;

assign m_axi_m_d_AWVALID = 1'b0;

assign m_axi_m_d_BREADY = 1'b0;

assign m_axi_m_d_WDATA = 32'd0;

assign m_axi_m_d_WID = 1'd0;

assign m_axi_m_d_WLAST = 1'b0;

assign m_axi_m_d_WSTRB = 4'd0;

assign m_axi_m_d_WUSER = 1'd0;

assign m_axi_m_d_WVALID = 1'b0;

assign tmp_s_fu_58_p1 = $signed(ap_port_reg_offset);

endmodule //load_feature_1