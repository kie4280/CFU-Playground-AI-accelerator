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
void do_grid_cfu_op0(void) {
  puts("\nExercise CFU Op0\n");
  printf("a   b-->");
  for (int b = 0; b < 6; b++) {
    printf("%8d", b);
  }
  puts("\n-------------------------------------------------------");
  for (int a = 0; a < 6; a++) {
    printf("%-8d", a);
    for (int b = 0; b < 6; b++) {
      int cfu = cfu_op0(0, a, b);
      printf("%8d", cfu);
    }
    puts("");
  }
}

void zero(void) {
  printf("\nzero the counter\n");
  cfu_op0(0, 0, 0);
}

// Test template instruction
void do_exercise_cfu_op0(void) {
  puts("\nExercise CFU Op0\n");
  int count = 0;
  for (int a = -0x71234567; a < 0x68000000; a += 0x10012345) {
    for (int b = -0x7edcba98; b < 0x68000000; b += 0x10770077) {
      int cfu = cfu_op0(0, a, b);
      printf("a: %08x b:%08x cfu=%08x\n", a, b, cfu);
      if (cfu != a) {
        printf("\n***FAIL\n");
        return;
      }
      count++;
    }
  }
  printf("Performed %d comparisons", count);
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
  for (int a=0; a<20; ++a) {
    r = cfu_op7(0, a, 0);
    printf("%d ", r);
  }
  printf("\nread the matrix B\n");
  for (int a=0; a<20; ++a) {
    r = cfu_op7(1, a, 0);
    printf("%d ", r);
  }
  printf("\nread the matrix C\n");
  for (int a=0; a<20; ++a) {
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
      {{2, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 2, 0}, {0, 0, 0, 2}}};
  std::array<std::array<int32_t, 4>, 4> B = {
      {{1, 2, 3, 4}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};
  for (int k = 0; k < 4; ++k) {
    uint32_t a4 = 0, b4 = 0;
    for (int j = 0; j < 4; ++j) {
      a4 += A[j][k] << j;
      b4 += B[k][j] << j;
    }
    cfu_op1(0, k, a4);
    cfu_op1(32, k, b4);
  }
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
        MENU_ITEM('0', "exercise cfu op0", do_exercise_cfu_op0),
        MENU_ITEM('g', "grid cfu op0", do_grid_cfu_op0),
        MENU_ITEM('r', "read data", read_mat),
        MENU_ITEM('z', "zero counter", zero),
        MENU_ITEM('m', "matrix multiply", matrix_multiply),
        MENU_ITEM('a', "write A", writeA),
        MENU_ITEM('b', "write B", writeB),
        MENU_ITEM('c', "compute", compute),
        MENU_END,
    },
};
}
;  // anonymous namespace

extern "C" void do_proj_menu() { menu_run(&MENU); }
