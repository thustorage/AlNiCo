//MTL-HEDER///////////////////////////////////////////////////////////////////
// This Program is the Confidential and Proprietary product of
// Mellanox Technologies LTD. Any unauthorized use, reproduction
// or transfer of this program is strictly prohibited.
// Copyright 1999 by Mellanox Technologies LTD All Rights Reserved.
///////////////////////////////////////////////////////////////////////////////

module g_ff__descn1_reset_value1_d0(clk, reset, vld, ns, ps);
input         clk;
input         reset;
input         vld;
input [0:0]   ns;
output [0:0]  ps;

wire [0:0]    ns_pack;
assign        ns_pack = ns;
wire [0:0]    final_reset_value = 1'd0;
reg [0:0]     ps_pack;
always @(posedge clk )
ps_pack <= reset ? final_reset_value :
           vld ? ns_pack : ps_pack;
assign        ps = ps_pack;

endmodule
