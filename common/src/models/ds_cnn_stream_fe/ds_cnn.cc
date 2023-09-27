#include "models/ds_cnn_stream_fe/ds_cnn.h"

#include <stdio.h>

#include <algorithm>
#include <cstdint>

#include "menu.h"
#include "models/ds_cnn_stream_fe/ds_cnn_stream_fe.h"
#include "models/label/label0_board.h"
#include "models/label/label11_board.h"
#include "models/label/label1_board.h"
#include "models/label/label6_board.h"
#include "models/label/label8_board.h"
#include "models/my_cycles.h"
#include "tflite.h"

// Initialize everything once
// deallocate tensors when done
static void ds_cnn_stream_fe_init(void) {
  tflite_load_model(ds_cnn_stream_fe, ds_cnn_stream_fe_len);
}

struct Results {
  uint32_t data[12];
};

static void print_results(Results rs) {
  for (int a = 0; a < 12; ++a) {
    printf("%d : 0x%08x,\n", a, static_cast<unsigned int>(rs.data[a]));
  }
}

// Implement your design here

static void run_inference() {
  printf("loaded data\n");
  tflite_classify();
  printf("got results!\n");

  float* output = tflite_get_output_float();
  Results rs;
  uint32_t* i = reinterpret_cast<uint32_t*>(output);
  std::copy(i, i + 12, rs.data);
  printf("MAC: %lld\n", get_my_cycles());
  print_results(rs);
}

static void run_inference_0() {
  tflite_set_input_float(label0_data);
  run_inference();
}
static void run_inference_1() {
  tflite_set_input_float(label1_data);
  run_inference();
}
static void run_inference_6() {
  tflite_set_input_float(label6_data);
  run_inference();
}
static void run_inference_8() {
  tflite_set_input_float(label8_data);
  run_inference();
}
static void run_inference_11() {
  tflite_set_input_float(label11_data);
  run_inference();
}

static struct Menu MENU = {
    "Tests for ds_cnn_stream_fe",
    "ds_cnn_stream_fe",
    {
        MENU_ITEM('1', "label 0", run_inference_0),
        MENU_ITEM('2', "label 1", run_inference_1),
        MENU_ITEM('3', "label 6", run_inference_6),
        MENU_ITEM('4', "label 8", run_inference_8),
        MENU_ITEM('5', "label 11", run_inference_11),
        MENU_END,
    },
};

// For integration into menu system
void ds_cnn_stream_fe_menu() {
  ds_cnn_stream_fe_init();
  menu_run(&MENU);
}
