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
  * A typical implementation needs few steps,
  * you should setup environment, create session,
  * setting device and somehow IO.
  *
  * You are required to make inference on your CPU.
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
  * Prepare the input image data for the model and perform inference,
  * then convert the model output back to the output image buffer.
  *
  * You will need to:
  * - Transform the raw image bytes into the tensor format the model expects
  * - Execute the model using the prepared session
  * - Extract and post-process the result tensor into 8-bit image data
  *
  * On any error:
  * - Use set_last_error() with a descriptive message
  * - Clean up any temporary resources you allocated
  * - Return -1
  *
  * Keep resource management careful to avoid leaks.
  */

  set_last_error("TODO: implement onnx_run_inference in src/ch4x/onnx_inference.c");
  return -1;
}

const char* onnx_get_last_error(void) {
  return g_last_error;
}
