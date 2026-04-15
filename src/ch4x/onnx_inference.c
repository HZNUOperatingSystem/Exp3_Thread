#include "ch4x/onnx_inference.h"

#include "onnxruntime_c_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const OrtApi* g_ort = NULL;
#ifdef __APPLE__
static __thread char g_last_error[512];
#else
static _Thread_local char g_last_error[512];
#endif

struct OnnxSession {
  OrtEnv* env;
  OrtSession* session;
  OrtMemoryInfo* memory_info;
};

static void set_last_error(const char* msg) {
  strncpy(g_last_error, msg, sizeof(g_last_error) - 1);
  g_last_error[sizeof(g_last_error) - 1] = '\0';
}

OnnxSession* onnx_session_create(const char* model_path) {
  OnnxSession* session = NULL;

  if (model_path == NULL || model_path[0] == '\0') {
    set_last_error("model path is empty");
    return NULL;
  }

  g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
  if (g_ort == NULL) {
    set_last_error("failed to get ONNX Runtime API");
    return NULL;
  }

  session = (OnnxSession*)calloc(1, sizeof(OnnxSession));
  if (session == NULL) {
    set_last_error("failed to allocate ONNX session");
    return NULL;
  }

  /*
   * TODO:
   * Build the ONNX Runtime session here.
   *
   * A typical implementation needs to:
   * 1. create the OrtEnv
   * 2. create and configure OrtSessionOptions
   * 3. open the model from model_path
   * 4. create CPU memory info for tensor buffers
   * 5. store the created handles inside `session`
   *
   * Return the allocated session on success.
   * If any step fails:
   * - record a helpful error with set_last_error(...)
   * - release every partially-created ONNX object
   * - free(session)
   * - return NULL
   */

  set_last_error("TODO: implement onnx_session_create in src/ch4x/onnx_inference.c");
  free(session);
  return NULL;
}

void onnx_session_release(OnnxSession* session) {
  if (session == NULL || g_ort == NULL) {
    return;
  }

  if (session->session != NULL) {
    g_ort->ReleaseSession(session->session);
  }
  if (session->env != NULL) {
    g_ort->ReleaseEnv(session->env);
  }
  if (session->memory_info != NULL) {
    g_ort->ReleaseMemoryInfo(session->memory_info);
  }

  free(session);
}

int onnx_run_inference(OnnxSession* session, const ImageBuffer* input, ImageBuffer* output) {
  if (session == NULL || input == NULL || output == NULL) {
    set_last_error("invalid arguments");
    return -1;
  }

  if (input->data == NULL || output->data == NULL) {
    set_last_error("image buffers are not allocated");
    return -1;
  }

  if (input->width != output->width || input->height != output->height ||
      input->channels != output->channels) {
    set_last_error("input and output image shapes must match");
    return -1;
  }

  /*
   * TODO:
   * Convert the input image into the tensor format expected by the ONNX model
   * and run inference.
   *
   * Recommended steps:
   * 1. allocate a float buffer for the normalized input tensor
   * 2. reorder image data from HWC into the layout expected by the model
   * 3. create the OrtValue input tensor
   * 4. call OrtSession::Run with the correct input/output node names
   * 5. read the output tensor back
   * 6. convert the float output into 8-bit image pixels
   * 7. release every temporary ONNX object and heap allocation
   *
   * On failure:
   * - call set_last_error(...) with a useful message
   * - clean up temporary resources
   * - return -1
   */

  set_last_error("TODO: implement onnx_run_inference in src/ch4x/onnx_inference.c");
  return -1;
}

const char* onnx_get_last_error(void) {
  return g_last_error;
}
