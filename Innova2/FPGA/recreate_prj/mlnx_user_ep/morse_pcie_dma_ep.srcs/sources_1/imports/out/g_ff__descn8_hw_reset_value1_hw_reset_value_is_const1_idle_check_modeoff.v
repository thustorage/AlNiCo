//MTL-HEDER///////////////////////////////////////////////////////////////////
// This Program is the Confidential and Proprietary product of
// Mellanox Technologies LTD. Any unauthorized use, reproduction
// or transfer of this program is strictly prohibited.
// Copyright 1999 by Mellanox Technologies LTD All Rights Reserved.
///////////////////////////////////////////////////////////////////////////////

module g_ff__descn8_hw_reset_value1_hw_reset_value_is_const1_idle_check_modeoff(
   clk, reset, vld, reset_value, ns, ps);
input         clk;
input         reset;
input         vld;
input [7:0]   reset_value;
input [7:0]   ns;
output [7:0]  ps;

wire [7:0]    ns_pack;
assign        ns_pack = ns;
wire [7:0]    final_reset_value = reset_value;

reg [7:0]     ps_pack;
always @(posedge clk )
ps_pack <= reset ? final_reset_value :
           vld ? ns_pack : ps_pack;
assign        ps = ps_pack;

endmodule
