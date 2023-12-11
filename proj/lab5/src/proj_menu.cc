/*
 * Copyright 2021 The CFU-Playground Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "proj_menu.h"

#include <stdio.h>

#include <array>
#include <cstdint>
#include <cstdlib>

#include "cfu.h"
#include "menu.h"

namespace {

// matrix multiply with matrix B
void matrixMult(int32_t* out, int32_t* A, int32_t* B, int m, int k, int n) {
  for (int a = 0; a < m; ++a) {
    for (int b = 0; b < n; ++b) {
      const int o_idx = a * n + b;
      out[o_idx] = 0;
      for (int c = 0; c < k; ++c) {
        const int a_idx = a * k + c;
        const int b_idx = c * n + b;
        out[o_idx] += A[a_idx] * B[b_idx];
      }
    }
  }
}

// Template Fn

// Test template instruction
void zero(void) {
  printf("\nzero the counter\n");
  cfu_op0(0, 0, 0);
}

void writeA(void) {
  printf("\nwrite matrix A (addr, value)\n");
  int a, b;
  scanf("%d %d", &a, &b);
  int r = cfu_op1(0, a, b);  // write A[0]
  printf("opcode: %d\n", r);
}

void writeB(void) {
  printf("\nwrite matrix B (addr, value)\n");
  int a, b;
  scanf("%d %d", &a, &b);
  int r = cfu_op1(1, a, b);  // write A[0]
  printf("opcode: %d\n", r);
}

void read_mat(void) {
  int r;
  printf("\nread the matrix A\n");
  for (int a = 0; a < 20; ++a) {
    r = cfu_op7(0, a, 0);
    printf("%d ", r);
  }
  printf("\nread the matrix B\n");
  for (int a = 0; a < 20; ++a) {
    r = cfu_op7(1, a, 0);
    printf("%d ", r);
  }
  printf("\nread the matrix C\n");
  for (int a = 0; a < 20; ++a) {
    r = cfu_op3(2, a, -1);
    printf("%d ", r);
  }
}

void compute(void) {
  cfu_op0(0, 0, 0);
  int r = cfu_op2(0, 0, (1 << 16) + 1);
  printf("opcode: %d\n", r);
  int a;
  scanf("%d", &a);
  r = cfu_op3(0, a, -1);
  printf("result: %d\n", r);
}

bool matrix_multiply(void) {
  int r = cfu_op0(0, 0, 0);  // reset the cfu
  std::array<int32_t, 16> A;
  std::array<int32_t, 16> B;
  std::array<int32_t, 16> answer;
  std::array<int32_t, 16> cfu_result;
  for (int a = 0; a < 16; ++a) {
    A[a] = std::rand() % 255;
    B[a] = std::rand() % 255;
  }
  for (int k = 0; k < 4; ++k) {
    uint32_t a4 = 0, b4 = 0;
    for (int j = 0; j < 4; ++j) {
      a4 += A[4 * j + k] << (j << 3);
      b4 += B[4 * k + j] << (j << 3);
    }
    cfu_op1(0, k, a4);
    cfu_op1(1, k, b4);
  }
  r = cfu_op2(0, 4, (4 << 16) + 4);
  // printf("cycles: %d\n", r);
  for (int a = 0; a < 4; ++a) {
    for (int b = 0; b < 4; ++b) {
      r = cfu_op3(0, a * 4 + b, 0);
      cfu_result[a * 4 + b] = r;
      printf("%d ", r);
    }
    printf("\n");
  }
  printf("\n");
  matrixMult(answer.begin(), A.data(), B.data(), 4, 4, 4);
  for (int a = 0; a < 16; ++a) {
    if (answer[a] != cfu_result[a]) {
      return false;
    } else if (a == 15) {
      return true;
    }
  }
  return false;
}

void matrix_check(void) {
  printf("\nmatrix multiply\n");
  bool pass = matrix_multiply();
  if (!pass) {
    printf("answer check failed\n");
  } else {
    printf("matrix multiply test passed\n");
  }
}

void golden_check(void) {
  bool pass = true;
  int a;
  for (a = 0; a < 1000; ++a) {
    pass = matrix_multiply();
    if (!pass) break;
  }
  if (!pass) {
    printf("answer check failed on case %d\n", a + 1);
  } else {
    printf("matrix multiply test passed\n");
  }
}

struct Menu MENU = {
    "Project Menu",
    "project",
    {
        MENU_ITEM('r', "read data", read_mat),
        MENU_ITEM('z', "zero counter", zero),
        MENU_ITEM('m', "matrix multiply", matrix_check),
        MENU_ITEM('a', "write A", writeA),
        MENU_ITEM('b', "write B", writeB),
        MENU_ITEM('c', "compute", compute),
        MENU_ITEM('g', "golden test", golden_check),
        MENU_END,
    },
};
};  // anonymous namespace

extern "C" void do_proj_menu() { menu_run(&MENU); }
