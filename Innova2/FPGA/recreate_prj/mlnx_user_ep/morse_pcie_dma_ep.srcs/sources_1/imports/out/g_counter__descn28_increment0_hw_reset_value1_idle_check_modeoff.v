//MTL-HEADER///////////////////////////////////////////////////////////////////
// This Program is the Confidential and Proprietary product of
// Mellanox Technologies LTD. Any unauthorized use, reproduction
// or transfer of this program is strictly prohibited.
// Copyright 1999 by Mellanox Technologies LTD All Rights Reserved.
///////////////////////////////////////////////////////////////////////////////

module g_counter__descn28_increment0_hw_reset_value1_idle_check_modeoff(clk, reset,
   reset_value, set, set_value, en, ps);
input         clk;
input         reset;
input [27:0]  reset_value;
input         set;
input [27:0]  set_value;// valid on clock of set, load set_value into the counter
input         en;                // inc/dec strobe
output [27:0] ps;

wire [3:0]    gate_en;
assign        gate_en[0] = en;
wire [4:0]    ns0 = {1'd0, ps[3:0]} - 5'd1;
wire [3:0]    reset_value0 = reset_value[3:0];
wire [3:0]    ns_final0 = set ? set_value[3:0] :
                          ns0[3:0];

g_ff__descn4_hw_reset_value1_hw_reset_value_is_const1_idle_check_modeoff g_ff0(
   .clk(clk),
   .reset(reset),
   .reset_value(reset_value0),
   .vld(1'b0 | set | gate_en[0]),
   .ns(ns_final0),
   .ps(ps[3:0])
);
assign        gate_en[1] = gate_en[0] & ns0[4];
wire [8:0]    ns1 = {1'd0, ps[11:4]} - 9'd1;
wire [7:0]    reset_value1 = reset_value[11:4];

wire [7:0]    ns_final1 =
                          set ? set_value[11:4] :
                          ns1[7:0];

g_ff__descn8_hw_reset_value1_hw_reset_value_is_const1_idle_check_modeoff g_ff1(
   .clk(clk),
   .reset(reset),
   .reset_value(reset_value1),
   .vld(1'b0 | set | gate_en[1]),
   .ns(ns_final1),
   .ps(ps[11:4])
);
assign        gate_en[2] = gate_en[1] & ns1[8];
wire [8:0]    ns2 = {1'd0, ps[19:12]} - 9'd1;
wire [7:0]    reset_value2 = reset_value[19:12];
wire [7:0]    ns_final2 = set ? set_value[19:12] :
                          ns2[7:0];

g_ff__descn8_hw_reset_value1_hw_reset_value_is_const1_idle_check_modeoff g_ff2(
   .clk(clk),
   .reset(reset),
   .reset_value(reset_value2),
   .vld(1'b0 | set | gate_en[2]),
   .ns(ns_final2),
   .ps(ps[19:12])
);
assign        gate_en[3] = gate_en[2] & ns2[8];
wire [8:0]    ns3 = {1'd0, ps[27:20]} - 9'd1;
wire [7:0]    reset_value3 = reset_value[27:20];
wire [7:0]    ns_final3 = set ? set_value[27:20] :
                          ns3[7:0];

g_ff__descn8_hw_reset_value1_hw_reset_value_is_const1_idle_check_modeoff g_ff3(
   .clk(clk),
   .reset(reset),
   .reset_value(reset_value3),
   .vld(1'b0 | set | gate_en[3]),
   .ns(ns_final3),
   .ps(ps[27:20])
);

wire          unconnected_ns3_8_ = |{1'b1, ns3[8]};

endmodule
