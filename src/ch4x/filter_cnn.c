#include "filter_cnn.h"
#include "common/onnx_inference.h"

#include <stdio.h>
#include <stdlib.h>

int filter_apply_cnn_impl(const ImageBuffer* input, ImageBuffer* output) {
  OnnxSession* session = NULL;
  const char* model_path = NULL;
  int result = -1;

  if (input == NULL || output == NULL || input->data == NULL || output->data == NULL ||
      input->width != output->width || input->height != output->height ||
      input->channels != output->channels) {
    fprintf(stderr, "CNN filter: invalid input/output buffers\n");
    return -1;
  }

  model_path = getenv("CNN_MODEL_PATH");
  if (model_path == NULL || model_path[0] == '\0') {
    fprintf(stderr, "CNN filter: CNN_MODEL_PATH environment variable not set\n");
    return -1;
  }

  session = onnx_session_create(model_path);
  if (session == NULL) {
    fprintf(stderr, "CNN filter: failed to load model from '%s' - %s\n", model_path,
            onnx_get_last_error());
    return -1;
  }

  result = onnx_run_inference(session, input, output);
  if (result != 0) {
    fprintf(stderr, "CNN filter: inference failed - %s\n", onnx_get_last_error());
  }

  onnx_session_release(session);
  return result;
}
