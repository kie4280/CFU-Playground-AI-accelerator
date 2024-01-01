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
void matrixMult(uint32_t* out, uint32_t* A, uint32_t* B, int m, int k, int n) {
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
    r = cfu_op3(2, a, 0);
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

// inline size_t compute_offset(const int a, const int b, const int m, const int
// k,
//                     const int n, const int a_or_b = 0) {
//   printf("%ld", sizeof(int));
//   if (a_or_b == 0) {

//   } else if (a_or_b == 1) {

//   }
//   return -1;
// }

template <int M, int K, int N>
void cfu_mul(std::array<uint32_t, M * N>& C,
             const std::array<uint32_t, M * K>& A,
             const std::array<uint32_t, K * N>& B, int m, int k, int n) {

}

bool matrix_multiply(void) {
  int r = cfu_op0(0, 0, 0);  // reset the cfu
  const int m = 1, k = 350, n = 1;
  std::array<uint32_t, m* k> A = {0};
  std::array<uint32_t, k* n> B = {0};
  std::array<uint32_t, m* n> answer = {0};
  std::array<uint32_t, m* n> cfu_result = {0};

  for (int a = 0; a < m * k; ++a) {
    A[a] = std::rand() % 255;
    // A[a] = a;
  }
  for (int a = 0; a < k * n; ++a) {
    B[a] = std::rand() % 255;
    // B[a] = a;
  }
  const int m_blocks = (m + 3) / 4;
  const int n_blocks = (n + 3) / 4;

  for (int a = 0; a < m_blocks; ++a) {
    // load A
    int a_counter = 0;
    int row_offset = 4 * a;
    for (int i = 0; i < k; ++i) {
      for (int j = 0; j < 4; ++j) {
        const int row = row_offset + j;
        int val;
        if (row >= m) {
          val = 0;
        } else {
          val = A[row * k + i];
        }
        cfu_op1(0, a_counter++, val);
      }
    }

    for (int b = 0; b < n_blocks; ++b) {
      // load B
      int b_counter = 0;
      int col_offset = 4 * b;
      for (int i = 0; i < k; ++i) {
        for (int j = 0; j < 4; ++j) {
          const int col = (col_offset + j);
          int val;
          if (col >= n) {
            val = 0;
          } else {
            val = B[i * n + col];
          }
          cfu_op1(1, b_counter++, val);
        }
      }

      // start compute
      r = cfu_op2(0, k, 0);
      // retrieve C
      int c_counter = 0;
      for (int c_r = 0; c_r < 4; ++c_r) {
        for (int c_c = 0; c_c < 4; ++c_c) {
          uint32_t c = cfu_op3(0, c_counter++, 0);
          if (row_offset + c_r < m && col_offset + c_c < n) {
            cfu_result[(row_offset + c_r) * n + (col_offset + c_c)] = c;
          }
          printf("%lu ", c);
        }
        printf("\n");
      }
      printf("\n");
    }
  }

  printf("cycles: %d\n", r);
  // load A
  // int a_counter = 0;
  // for (int v = 0; v < ; ++v) {
  //   for (int i = 0; i < k; ++i) {
  //     for (int j = 0; j < 4; ++j) {
  //       const int row = (4 * v + j);
  //       int val;
  //       if (row >= m) {
  //         val = 0;
  //       } else {
  //         val = A[row * k + i];
  //       }
  //       cfu_op1(0, a_counter++, val);
  //     }
  //   }
  // }
  // // load B
  // int b_counter = 0;
  // for (int v = 0; v < (n + 3) / 4; ++v) {
  //   for (int i = 0; i < k; ++i) {
  //     for (int j = 0; j < 4; ++j) {
  //       const int col = (4 * v + j);
  //       int val;
  //       if (col >= n) {
  //         val = 0;
  //       } else {
  //         val = B[i * n + col];
  //       }
  //       cfu_op1(1, b_counter++, val);
  //     }
  //   }
  // }
  // // initiate compute
  // r = cfu_op2(0, 4, (k << 16) + 4);
  // printf("cycles: %d\n", r);
  // // read back value of C
  // for (int a = 0; a < m; ++a) {
  //   for (int b = 0; b < n; ++b) {
  //     r = cfu_op3(0, a * 4 + b, 0);
  //     cfu_result[a * 4 + b] = r;
  //     printf("%d ", r);
  //   }
  //   printf("\n");
  // }
  // printf("\n");

  // matmul using software
  matrixMult(answer.begin(), A.data(), B.data(), m, k, n);
  // matmul checking
  for (int a = 0; a < m * n; ++a) {
    if (answer[a] != cfu_result[a]) {
      printf("index %d should be %ld instead of %ld\n", a, answer[a],
             cfu_result[a]);
      return false;
    } else if (a == m * n - 1) {
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
