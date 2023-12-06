`timescale 1ns/10ps
// `include "/home/kie/MyProjects/aaml/CFU-Playground/proj/lab5/RTL/TPU.v"
`include "/home/kie/MyProjects/aaml/CFU-Playground/proj/lab5/RTL/global_buffer.v"

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

  reg             A_wr_en;
  wire [15:0]     A_index_w;
  reg  [31:0]     A_data_in;
  reg  [31:0]     A_data_out;

  reg             B_wr_en;
  wire [15:0]     B_index_w;
  reg  [31:0]     B_data_in;
  reg  [31:0]     B_data_out;

  wire            C_wr_en;
  wire [15:0]     C_index_w;
  reg  [127:0]    C_data_in;
  reg  [127:0]    C_data_out;

  reg             in_valid;
  reg  [7:0]      K;
  reg  [7:0]      M;
  reg  [7:0]      N;
  reg             busy = 0;

  reg  [15:0]     A_index_CFU;
  reg  [15:0]     B_index_CFU;
  reg  [15:0]     C_index_CFU;

  wire [15:0]     A_index_TPU;
  wire [15:0]     B_index_TPU;
  wire [15:0]     C_index_TPU;


  // TPU my_tpu(
  //   .clk(clk),
  //   .rst_n(~reset),
  //   .in_valid(in_valid),
  //   .K(K),
  //   .M(M),
  //   .N(N),
  //   .busy(busy),
  //   .A_wr_en(A_wr_en),
  //   .A_index(A_index_TPU),
  //   .A_data_in(A_data_in),
  //   .A_data_out(A_data_out),
  //   .B_wr_en(B_wr_en),
  //   .B_index(B_index_TPU),
  //   .B_data_in(B_data_in),
  //   .B_data_out(B_data_out),
  //   .C_wr_en(C_wr_en),
  //   .C_index(C_index_TPU),
  //   .C_data_in(C_data_in),
  //   .C_data_out(C_data_out)

  // );

  global_buffer #(
      .ADDR_BITS(16),
      .DATA_BITS(32)
  )
  gbuff_A(
      .clk(clk),
      .rst(reset),
      .wr_en(A_wr_en),
      .index(A_index_w),
      .data_in(A_data_in),
      .data_out(A_data_out)
  );

  global_buffer #(
      .ADDR_BITS(16),
      .DATA_BITS(32)
  ) gbuff_B(
      .clk(clk),
      .rst(reset),
      .wr_en(B_wr_en),
      .index(B_index_w),
      .data_in(B_data_in),
      .data_out(B_data_out)
  );


  global_buffer #(
      .ADDR_BITS(16),
      .DATA_BITS(128)
  ) gbuff_C(
      .clk(clk),
      .rst(reset),
      .wr_en(C_wr_en),
      .index(C_index_w),
      .data_in(C_data_in),
      .data_out(C_data_out)
  );


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



  localparam STATE_IDLE = 0;
  localparam STATE_EXEC = 1;
  localparam STATE_RSP_READY = 7;

  localparam OP_RESET = 0;
  localparam OP_WRITE_MEM = 1;
  localparam OP_COMPUTE = 2;
  localparam OP_READ_MEM = 3;

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
        if (cmd_ready && cmd_valid) begin
          if (opcode == OP_COMPUTE || opcode == OP_READ_MEM) next_state = STATE_EXEC;
          else next_state = STATE_RSP_READY;
        end
        else next_state = STATE_IDLE;
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

  reg [31:0] counter;
  assign A_index_w = cur_state == STATE_EXEC ? A_index_TPU: A_index_CFU;
  assign B_index_w = cur_state == STATE_EXEC ? B_index_TPU: B_index_CFU;
  assign C_index_w = cur_state == STATE_EXEC ? C_index_TPU: C_index_CFU;

  always @(posedge reset, posedge clk) begin
    case (cur_state)
      STATE_IDLE: begin
        rsp_payload_outputs_0 <= 32'b0;
        A_wr_en <= 0;
        B_wr_en <= 0;
        if (funct_id[5] == 0) begin
          A_index_CFU <= cmd_inputs_0[15:0];
        end
        else begin
          B_index_CFU <= cmd_inputs_0[15:0];
        end
        C_index_CFU <= cmd_inputs_0[15:0];

      end
      STATE_EXEC: begin
        if (opcode == OP_COMPUTE) begin

        end
        else if (opcode == OP_READ_MEM) begin
          rsp_payload_outputs_0 <= C_data_out;
        end

      end
      STATE_RSP_READY: begin
        if (opcode == OP_WRITE_MEM) begin
          if (funct_id[5] == 0) A_wr_en <= 1;
          else B_wr_en <= 1;
          C_data_in <= cmd_inputs_1;
        end
        else begin
          rsp_payload_outputs_0 <= 0;
        end
      end

      default:;
    endcase
  end
endmodule
