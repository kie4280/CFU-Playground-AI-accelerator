`timescale 1ns/10ps
// `include "/home/kie/MyProjects/aaml/CFU-Playground/proj/lab5/RTL/TPU.v"
// `include "/home/kie/MyProjects/aaml/CFU-Playground/proj/lab5/RTL/global_buffer.v"

// Copyright 2021 The CFU-Playground Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


module Cfu (
  input               cmd_valid,
  output              cmd_ready,
  input      [9:0]    cmd_payload_function_id,
  input      [31:0]   cmd_payload_inputs_0,
  input      [31:0]   cmd_payload_inputs_1,
  output reg          rsp_valid,
  input               rsp_ready,
  output reg [31:0]   rsp_payload_outputs_0,
  input               reset,
  input               clk
);

  wire          A_wr_en;
  reg  [15:0]   A_index;
  reg  [31:0]   A_data_in;
  reg  [31:0]   A_data_out;

  wire          B_wr_en;
  reg  [15:0]   B_index;
  reg  [31:0]   B_data_in;
  reg  [31:0]   B_data_out;

  wire          C_wr_en;
  reg  [15:0]   C_index;
  reg  [127:0]  C_data_in;
  reg  [127:0]  C_data_out;

  reg           in_valid;
  reg [7:0]     K;
  reg [7:0]     M;
  reg [7:0]     N;
  reg           busy = 0;

  // TPU my_tpu(
  //   .clk(clk),
  //   .rst_n(~reset),
  //   .in_valid(in_valid),
  //   .K(K),
  //   .M(M),
  //   .N(N),
  //   .busy(busy),
  //   .A_wr_en(A_wr_en),
  //   .A_index(A_index),
  //   .A_data_in(A_data_in),
  //   .A_data_out(A_data_out),
  //   .B_wr_en(B_wr_en),
  //   .B_index(B_index),
  //   .B_data_in(B_data_in),
  //   .B_data_out(B_data_out),
  //   .C_wr_en(C_wr_en),
  //   .C_index(C_index),
  //   .C_data_in(C_data_in),
  //   .C_data_out(C_data_out)

  // );

  // global_buffer #(
  //     .ADDR_BITS(16),
  //     .DATA_BITS(32)
  // )
  // gbuff_A(
  //     .clk(clk),
  //     .rst_n(reset),
  //     .wr_en(A_wr_en),
  //     .index(A_index),
  //     .data_in(A_data_in),
  //     .data_out(A_data_out)
  // );

  // global_buffer #(
  //     .ADDR_BITS(16),
  //     .DATA_BITS(32)
  // ) gbuff_B(
  //     .clk(clk),
  //     .rst_n(reset),
  //     .wr_en(B_wr_en),
  //     .index(B_index),
  //     .data_in(B_data_in),
  //     .data_out(B_data_out)
  // );


  // global_buffer #(
  //     .ADDR_BITS(16),
  //     .DATA_BITS(128)
  // ) gbuff_C(
  //     .clk(clk),
  //     .rst_n(reset),
  //     .wr_en(C_wr_en),
  //     .index(C_index),
  //     .data_in(C_data_in),
  //     .data_out(C_data_out)
  // );


  reg [31:0] cmd_inputs_0;
  reg [31:0] cmd_inputs_1;
  reg [6:0]  funct_id;
  reg [2:0]  opcode;

  always @(posedge reset, posedge cmd_valid) begin
    if (reset) begin
      cmd_inputs_0 = 32'b0;
      cmd_inputs_1 = 32'b0;
      funct_id = 0;
      opcode = 0;
    end
    else if (cmd_valid) begin
      cmd_inputs_0 = cmd_payload_inputs_0;
      cmd_inputs_1 = cmd_payload_inputs_1;
      cmd_id = cmd_payload_function_id;
      funct_id = cmd_payload_function_id[9:3];
      opcode = cmd_payload_function_id[2:0];
    end
    else begin
      cmd_inputs_0 = cmd_inputs_0;
      cmd_inputs_1 = cmd_inputs_1;
      funct_id = funct_id;
      opcode = opcode;
    end
  end


  // Only not ready for a command when we have a response.

  localparam STATE_IDLE = 0;
  localparam STATE_LOAD = 1;
  localparam STATE_EXEC = 2;
  localparam STATE_RSP_READY = 4;

  reg [2:0] cur_state = STATE_IDLE;
  reg [2:0] next_state = STATE_IDLE;

  always @(posedge clk, posedge reset) begin
    if (reset) begin
      cur_state <= STATE_IDLE;
    end
    else begin
      cur_state <= next_state;
    end
  end

  assign cmd_ready = (cur_state == STATE_IDLE);

  always @(*) begin
    case (cur_state)
      STATE_IDLE: begin
        if (cmd_ready && cmd_valid) next_state = STATE_LOAD;
        else next_state = STATE_IDLE;
        rsp_valid = 0;

      end
      STATE_LOAD: begin
        next_state = STATE_EXEC;
        rsp_valid = 0;
      end
      STATE_EXEC: begin
        next_state = STATE_RSP_READY;
        rsp_valid = 0;

      end
      STATE_RSP_READY: begin
        rsp_valid = 1;
        if (rsp_valid && rsp_ready) next_state = STATE_IDLE;
        else next_state = STATE_RSP_READY;

      end

      default:;

    endcase

  end



  localparam InputOffset = $signed(9'd128);

  // SIMD multiply step:
  wire signed [15:0] prod_0, prod_1, prod_2, prod_3;
  assign prod_0 =  ($signed(cmd_payload_inputs_0[7 : 0]) + InputOffset)
                  * $signed(cmd_payload_inputs_1[7 : 0]);
  assign prod_1 =  ($signed(cmd_payload_inputs_0[15: 8]) + InputOffset)
                  * $signed(cmd_payload_inputs_1[15: 8]);
  assign prod_2 =  ($signed(cmd_payload_inputs_0[23:16]) + InputOffset)
                  * $signed(cmd_payload_inputs_1[23:16]);
  assign prod_3 =  ($signed(cmd_payload_inputs_0[31:24]) + InputOffset)
                  * $signed(cmd_payload_inputs_1[31:24]);

  wire signed [31:0] sum_prods;
  assign sum_prods = prod_0 + prod_1 + prod_2 + prod_3;


  reg [31:0] counter;

  always @(posedge reset, posedge clk) begin
    if (reset) begin
      rsp_payload_outputs_0 <= 32'b0;
      counter <= 0;
    end
    else begin
      case (cur_state)
      STATE_IDLE: begin
        rsp_payload_outputs_0 <= 32'b0;

      end
      STATE_LOAD: begin

      end
      STATE_EXEC: begin
        if (cmd_id[9:3] == 1) counter <= counter + 1;
        else counter <= 0;
        rsp_payload_outputs_0 <= counter;
      end
      STATE_RSP_READY: begin

      end

      default:;
      endcase

    end
    
    // if (rsp_valid && ~busy) begin
    //   // Waiting to hand off response to CPU.
    //   rsp_valid <= ~rsp_ready;
    // end else if (cmd_valid) begin
    //   rsp_valid <= 1'b1;
    //   // Accumulate step:
    //   rsp_payload_outputs_0 <= 177777;
    // end
    // else begin
    //   rsp_valid <= rsp_valid;
    //   rsp_payload_outputs_0 <= rsp_payload_outputs_0;

    // end
  end
endmodule
