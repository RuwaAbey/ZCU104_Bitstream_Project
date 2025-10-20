//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2024.2 (lin64) Build 5239630 Fri Nov 08 22:34:34 MST 2024
//Date        : Thu Oct 16 10:36:09 2025
//Host        : ruwa-HP running 64-bit Ubuntu 24.04.3 LTS
//Command     : generate_target GPIO_design_1_wrapper.bd
//Design      : GPIO_design_1_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module GPIO_design_1_wrapper
   (led_4bits_tri_o);
  output [3:0]led_4bits_tri_o;

  wire [3:0]led_4bits_tri_o;

  GPIO_design_1 GPIO_design_1_i
       (.led_4bits_tri_o(led_4bits_tri_o));
endmodule
