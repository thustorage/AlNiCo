///////////////////////////////////////////////////////////////////////////////
// This Program is the Confidential and Proprietary product of
// Mellanox Technologies LTD. Any unauthorized use, reproduction
// or transfer of this program is strictly prohibited.
// Copyright (c) 1999 by Mellanox Technologies LTD All Rights Reserved.
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////
// INNOVA-2 FLEX FPGA COMPONENT //
//////////////////////////////////

`timescale 1 ps / 1 ps

module pcie_xdma_wrapper 
  (io_clk_100mhz_p, io_clk_100mhz_n, leds_green, cx_rst_, //vp, vn, 
   c0_ddr4_clk_p, c0_ddr4_clk_n, c0_ddr4_reset_n,
   c0_ddr4_act_n, c0_ddr4_ck_t, c0_ddr4_ck_c, c0_ddr4_cke, c0_ddr4_cs_n, c0_ddr4_odt,
   c0_ddr4_bg, c0_ddr4_ba, c0_ddr4_dm_dbi_n, c0_ddr4_dqs_t, c0_ddr4_dqs_c, c0_ddr4_adr,
   c0_ddr4_dq, c0_ddr4_alert_n, 
   srds_cx_pcie_clk_n, srds_cx_pcie_clk_p,
   srds_cx_pcie_perst, srds_cx_pcie_rx_ap, srds_cx_pcie_rx_an, srds_cx_pcie_rx_bp,
   srds_cx_pcie_rx_bn, srds_cx_pcie_rx_cp, srds_cx_pcie_rx_cn, srds_cx_pcie_rx_dp,
   srds_cx_pcie_rx_dn, srds_cx_pcie_rx_ep, srds_cx_pcie_rx_en, srds_cx_pcie_rx_fp,
   srds_cx_pcie_rx_fn, srds_cx_pcie_rx_gp, srds_cx_pcie_rx_gn, srds_cx_pcie_rx_hp,
   srds_cx_pcie_rx_hn, srds_cx_pcie_tx_ap, srds_cx_pcie_tx_an, srds_cx_pcie_tx_bp,
   srds_cx_pcie_tx_bn, srds_cx_pcie_tx_cp, srds_cx_pcie_tx_cn, srds_cx_pcie_tx_dp,
   srds_cx_pcie_tx_dn, srds_cx_pcie_tx_ep, srds_cx_pcie_tx_en, srds_cx_pcie_tx_fp,
   srds_cx_pcie_tx_fn, srds_cx_pcie_tx_gp, srds_cx_pcie_tx_gn, srds_cx_pcie_tx_hp,
   srds_cx_pcie_tx_hn);

  ////////////
 // SYSTEM //
////////////
input         io_clk_100mhz_p;
input         io_clk_100mhz_n;
input         cx_rst_; // Constant High

// LED's (back side)
output [1:0]  leds_green;

  //////////
 // PCIE //
//////////

// clock
input         srds_cx_pcie_clk_n;
input         srds_cx_pcie_clk_p;

// perst
input         srds_cx_pcie_perst;

// input channels
input         srds_cx_pcie_rx_ap;
input         srds_cx_pcie_rx_an;
input         srds_cx_pcie_rx_bp;
input         srds_cx_pcie_rx_bn;
input         srds_cx_pcie_rx_cp;
input         srds_cx_pcie_rx_cn;
input         srds_cx_pcie_rx_dp;
input         srds_cx_pcie_rx_dn;
input         srds_cx_pcie_rx_ep;
input         srds_cx_pcie_rx_en;
input         srds_cx_pcie_rx_fp;
input         srds_cx_pcie_rx_fn;
input         srds_cx_pcie_rx_gp;
input         srds_cx_pcie_rx_gn;
input         srds_cx_pcie_rx_hp;
input         srds_cx_pcie_rx_hn;

// output channels
output        srds_cx_pcie_tx_ap;
output        srds_cx_pcie_tx_an;
output        srds_cx_pcie_tx_bp;
output        srds_cx_pcie_tx_bn;
output        srds_cx_pcie_tx_cp;
output        srds_cx_pcie_tx_cn;
output        srds_cx_pcie_tx_dp;
output        srds_cx_pcie_tx_dn;
output        srds_cx_pcie_tx_ep;
output        srds_cx_pcie_tx_en;
output        srds_cx_pcie_tx_fp;
output        srds_cx_pcie_tx_fn;
output        srds_cx_pcie_tx_gp;
output        srds_cx_pcie_tx_gn;
output        srds_cx_pcie_tx_hp;
output        srds_cx_pcie_tx_hn;


  //////////
 // DDR4 //
//////////

// *** not available for Innova-2 Flex VPI ***

input         c0_ddr4_clk_p;
input         c0_ddr4_clk_n;
output        c0_ddr4_reset_n;
output        c0_ddr4_act_n;
output        c0_ddr4_ck_t;
output        c0_ddr4_ck_c;
output        c0_ddr4_cke;
output        c0_ddr4_cs_n;
output        c0_ddr4_odt;
output [1:0]  c0_ddr4_bg;
output [1:0]  c0_ddr4_ba;
inout  [8:0]  c0_ddr4_dm_dbi_n;
inout  [8:0]  c0_ddr4_dqs_t;
inout  [8:0]  c0_ddr4_dqs_c;
output [16:0] c0_ddr4_adr;
inout  [71:0] c0_ddr4_dq;
input         c0_ddr4_alert_n;


// Connectx-FPGA PCI-E example design instantiation:

wire [7:0]    pci_exp_txp;
wire [7:0]    pci_exp_txn;
wire [7:0]    pci_exp_rxp;
wire [7:0]    pci_exp_rxn;

assign        srds_cx_pcie_tx_hp = pci_exp_txp[7];
assign        srds_cx_pcie_tx_gp = pci_exp_txp[6];
assign        srds_cx_pcie_tx_fp = pci_exp_txp[5];
assign        srds_cx_pcie_tx_ep = pci_exp_txp[4];
assign        srds_cx_pcie_tx_dp = pci_exp_txp[3];
assign        srds_cx_pcie_tx_cp = pci_exp_txp[2];
assign        srds_cx_pcie_tx_bp = pci_exp_txp[1];
assign        srds_cx_pcie_tx_ap = pci_exp_txp[0];
assign        srds_cx_pcie_tx_hn = pci_exp_txn[7];
assign        srds_cx_pcie_tx_gn = pci_exp_txn[6];
assign        srds_cx_pcie_tx_fn = pci_exp_txn[5];
assign        srds_cx_pcie_tx_en = pci_exp_txn[4];
assign        srds_cx_pcie_tx_dn = pci_exp_txn[3];
assign        srds_cx_pcie_tx_cn = pci_exp_txn[2];
assign        srds_cx_pcie_tx_bn = pci_exp_txn[1];
assign        srds_cx_pcie_tx_an = pci_exp_txn[0];

assign        pci_exp_rxp = {srds_cx_pcie_rx_hp,srds_cx_pcie_rx_gp,srds_cx_pcie_rx_fp,srds_cx_pcie_rx_ep,srds_cx_pcie_rx_dp,srds_cx_pcie_rx_cp,srds_cx_pcie_rx_bp,srds_cx_pcie_rx_ap};
assign        pci_exp_rxn = {srds_cx_pcie_rx_hn,srds_cx_pcie_rx_gn,srds_cx_pcie_rx_fn,srds_cx_pcie_rx_en,srds_cx_pcie_rx_dn,srds_cx_pcie_rx_cn,srds_cx_pcie_rx_bn,srds_cx_pcie_rx_an};

  pcie_xdma pcie_xdma_i
       (.ddr4_rtl_act_n    (c0_ddr4_act_n),
        .ddr4_rtl_adr      (c0_ddr4_adr),
        .ddr4_rtl_ba       (c0_ddr4_ba),
        .ddr4_rtl_bg       (c0_ddr4_bg),
        .ddr4_rtl_ck_c     (c0_ddr4_ck_c),
        .ddr4_rtl_ck_t     (c0_ddr4_ck_t),
        .ddr4_rtl_cke      (c0_ddr4_cke),
        .ddr4_rtl_cs_n     (c0_ddr4_cs_n),
        .ddr4_rtl_dm_n     (c0_ddr4_dm_dbi_n),
        .ddr4_rtl_dq       (c0_ddr4_dq),
        .ddr4_rtl_dqs_c    (c0_ddr4_dqs_c),
        .ddr4_rtl_dqs_t    (c0_ddr4_dqs_t),
        .ddr4_rtl_odt      (c0_ddr4_odt),
        .ddr4_rtl_reset_n  (c0_ddr4_reset_n),
        .pcie_7x_mgt_rxn   (pci_exp_rxn),
        .pcie_7x_mgt_rxp   (pci_exp_rxp),
        .pcie_7x_mgt_txn   (pci_exp_txn),
        .pcie_7x_mgt_txp   (pci_exp_txp),
        .pcie_perstn       (srds_cx_pcie_perst),
        .pcie_refclk_clk_n (srds_cx_pcie_clk_n),
        .pcie_refclk_clk_p (srds_cx_pcie_clk_p),
        .sys_refclk_clk_n  (c0_ddr4_clk_n),
        .sys_refclk_clk_p  (c0_ddr4_clk_p),
        .sys_reset         (~cx_rst_));



///////////////////////////////////////////////////////////////////////////////
//             Clock & Reset
///////////////////////////////////////////////////////////////////////////////


// generate SYS 100mhz_clk reset  using the PLL Lock

wire          bufgds_io_clk_100mhz;

   IBUFDS #(
      .DQS_BIAS("FALSE")  // (FALSE, TRUE)
   )
   IBUFDS_io_clk (
      .O(bufgds_io_clk_100mhz),  // 1-bit output: Buffer output
      .I(io_clk_100mhz_p),       // 1-bit input: Diff_p buffer input (connect directly to top-level port)
      .IB(io_clk_100mhz_n)       // 1-bit input: Diff_n buffer input (connect directly to top-level port)
   );

wire io_clk_100mhz_alive_value;
wire sys_100mhz_clk_reset;
wire sys_100mhz_clk;

wire          pre_io_clk_100mhz;

clk_wiz_100 clk_wiz_100_sys_clk(
   .clk_in    (bufgds_io_clk_100mhz),
   .reset     (~cx_rst_),
   .locked    (io_clk_100mhz_alive_value),
   .clk_out1  (pre_io_clk_100mhz)            // main system clock source point
);

   BUFG BUFG_sys_clk (
      .O(sys_100mhz_clk),        // 1-bit output: Clock output
      .I(pre_io_clk_100mhz)      // 1-bit input: Clock input
   );

assign sys_100mhz_clk_reset = ~io_clk_100mhz_alive_value;

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                        LEDS
///////////////////////////////////////////////////////////////////////////////


// generate tick every one second ddr_clk
wire [1:0]    leds_green_register;
wire          led_sel;
wire [27:0]   sec_count;
g_counter__descn28_increment0_hw_reset_value1_idle_check_modeoff g_counter_one_sec_ddr_clk(
   .clk         (sys_100mhz_clk),
   .reset       (sys_100mhz_clk_reset),
   .reset_value (28'd100000000),
   .set         (sec_count == 28'd0),
   .set_value   (28'd100000000),
   .en          (1'b1),
   .ps          (sec_count)
);

wire          led_blink;

g_ff__descn1_reset_value1_d0 g_ff_ddr_clk_led_blink(
   .clk         (sys_100mhz_clk),
   .reset       (sys_100mhz_clk_reset),
   .vld         (sec_count == 28'd0),
   .ns          (~led_blink),
   .ps          (led_blink)
);

// insert led blink for port clocks (LED active low)
assign        leds_green = {led_blink,led_blink};


endmodule
