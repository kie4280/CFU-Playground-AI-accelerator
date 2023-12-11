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

#include "cfu.h"
#include "menu.h"

namespace {

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

void matrix_multiply(void) {
  printf("\nmatrix multiply\n");
  int r = cfu_op0(0, 0, 0);  // reset the cfu
  std::array<std::array<int32_t, 4>, 4> A = {
      {{1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {0, 0, 0, 4}}};
  std::array<std::array<int32_t, 4>, 4> B = {
      {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
  for (int k = 0; k < 4; ++k) {
    uint32_t a4 = 0, b4 = 0;
    for (int j = 0; j < 4; ++j) {
      a4 += A[j][k] << (j << 3);
      b4 += B[k][j] << (j << 3);
    }
    cfu_op1(0, k, a4);
    cfu_op1(1, k, b4);
  }
  r = cfu_op2(0, 4, (4 << 16) + 4);
  printf("cycles: %d\n", r);
  r = cfu_op7(0, 0, 0);
  printf("debug: %d\n", r);
  r = cfu_op7(1, 0, 0);
  printf("debug: %d\n", r);
  r = cfu_op3(0, 0, 0);
  printf("debug: %d\n", r);
}

struct Menu MENU = {
    "Project Menu",
    "project",
    {
        MENU_ITEM('r', "read data", read_mat),
        MENU_ITEM('z', "zero counter", zero),
        MENU_ITEM('m', "matrix multiply", matrix_multiply),
        MENU_ITEM('a', "write A", writeA),
        MENU_ITEM('b', "write B", writeB),
        MENU_ITEM('c', "compute", compute),
        MENU_END,
    },
};
};  // anonymous namespace

extern "C" void do_proj_menu() { menu_run(&MENU); }
