#include "common/onnx_inference.h"

#include "onnxruntime_c_api.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const OrtApi* g_ort = NULL;
#ifdef __APPLE__
static __thread char g_last_error[512];
#else
static _Thread_local char g_last_error[512];
#endif

static void set_last_error(const char* msg) {
  strncpy(g_last_error, msg, sizeof(g_last_error) - 1);
  g_last_error[sizeof(g_last_error) - 1] = '\0';
}

static int check_ort_status(OrtStatus* status) {
  if (status != NULL) {
    const char* msg = g_ort->GetErrorMessage(status);
    set_last_error(msg);
    fprintf(stderr, "ONNX Runtime Error: %s\n", msg);
    g_ort->ReleaseStatus(status);
    return -1;
  }
  return 0;
}

struct OnnxSession {
  OrtEnv* env;
  OrtSession* session;
  OrtMemoryInfo* memory_info;
  int input_width;
  int input_height;
  int input_channels;
};

OnnxSession* onnx_session_create(const char* model_path) {
  OnnxSession* session = NULL;
  OrtStatus* status = NULL;
  OrtSessionOptions* session_options = NULL;

  g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
  if (g_ort == NULL) {
    set_last_error("Failed to get ONNX Runtime API");
    return NULL;
  }

  session = (OnnxSession*)calloc(1, sizeof(OnnxSession));
  if (session == NULL) {
    set_last_error("Failed to allocate session");
    return NULL;
  }

  status = g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &session->memory_info);
  if (check_ort_status(status) != 0) {
    free(session);
    return NULL;
  }

  status = g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "onnx_inference", &session->env);
  if (check_ort_status(status) != 0) {
    g_ort->ReleaseMemoryInfo(session->memory_info);
    free(session);
    return NULL;
  }

  status = g_ort->CreateSessionOptions(&session_options);
  if (check_ort_status(status) != 0) {
    g_ort->ReleaseEnv(session->env);
    g_ort->ReleaseMemoryInfo(session->memory_info);
    free(session);
    return NULL;
  }

  status = g_ort->CreateSession(session->env, model_path, session_options, &session->session);
  g_ort->ReleaseSessionOptions(session_options);

  if (check_ort_status(status) != 0) {
    g_ort->ReleaseEnv(session->env);
    g_ort->ReleaseMemoryInfo(session->memory_info);
    free(session);
    return NULL;
  }

  size_t num_input_nodes = 0;
  status = g_ort->SessionGetInputCount(session->session, &num_input_nodes);
  if (check_ort_status(status) != 0 || num_input_nodes == 0) {
    set_last_error("Model has no input nodes");
    goto cleanup;
  }

  OrtTypeInfo* typeinfo = NULL;
  const OrtTensorTypeAndShapeInfo* tensor_info = NULL;
  size_t num_dims = 0;

  status = g_ort->SessionGetInputTypeInfo(session->session, 0, &typeinfo);
  if (check_ort_status(status) != 0) {
    goto cleanup;
  }

  status = g_ort->CastTypeInfoToTensorInfo(typeinfo, &tensor_info);
  if (check_ort_status(status) != 0) {
    g_ort->ReleaseTypeInfo(typeinfo);
    goto cleanup;
  }

  status = g_ort->GetDimensionsCount(tensor_info, &num_dims);
  if (check_ort_status(status) != 0) {
    g_ort->ReleaseTypeInfo(typeinfo);
    goto cleanup;
  }

  if (num_dims == 4) {
    int64_t dims[4];
    status = g_ort->GetDimensions(tensor_info, dims, 4);
    if (check_ort_status(status) == 0) {
      session->input_channels = (int)dims[1];
      session->input_height = (int)dims[2];
      session->input_width = (int)dims[3];
    }
  }

  g_ort->ReleaseTypeInfo(typeinfo);

  return session;

cleanup:
  g_ort->ReleaseSession(session->session);
  g_ort->ReleaseEnv(session->env);
  g_ort->ReleaseMemoryInfo(session->memory_info);
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
  OrtStatus* status = NULL;
  OrtValue* input_tensor = NULL;
  OrtValue* output_tensor = NULL;
  const char* input_names[] = {"input"};
  const char* output_names[] = {"output"};

  if (session == NULL || input == NULL || output == NULL) {
    set_last_error("Invalid arguments");
    return -1;
  }

  if (input->width != output->width || input->height != output->height ||
      input->channels != output->channels) {
    set_last_error("Input and output dimensions must match");
    return -1;
  }

  int width = input->width;
  int height = input->height;
  int channels = input->channels;

  size_t input_tensor_size = (size_t)channels * (size_t)height * (size_t)width;
  float* input_float = (float*)malloc(input_tensor_size * sizeof(float));
  if (input_float == NULL) {
    set_last_error("Failed to allocate input buffer");
    return -1;
  }

  for (int c = 0; c < channels; ++c) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        size_t src_idx = ((size_t)y * (size_t)width + (size_t)x) * (size_t)channels + (size_t)c;
        size_t dst_idx = ((size_t)c * (size_t)height + (size_t)y) * (size_t)width + (size_t)x;
        input_float[dst_idx] = input->data[src_idx] / 255.0f;
      }
    }
  }

  int64_t input_shape[] = {1, channels, height, width};
  status = g_ort->CreateTensorWithDataAsOrtValue(
      session->memory_info, input_float, input_tensor_size * sizeof(float), input_shape, 4,
      ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensor);

  if (check_ort_status(status) != 0) {
    free(input_float);
    return -1;
  }

  int is_tensor;
  status = g_ort->IsTensor(input_tensor, &is_tensor);
  if (check_ort_status(status) != 0 || !is_tensor) {
    g_ort->ReleaseValue(input_tensor);
    free(input_float);
    return -1;
  }

  status = g_ort->Run(session->session, NULL, input_names, (const OrtValue* const*)&input_tensor, 1,
                      output_names, 1, &output_tensor);

  g_ort->ReleaseValue(input_tensor);

  if (check_ort_status(status) != 0) {
    free(input_float);
    return -1;
  }

  float* output_float = NULL;
  status = g_ort->GetTensorMutableData(output_tensor, (void**)&output_float);
  if (check_ort_status(status) != 0 || output_float == NULL) {
    g_ort->ReleaseValue(output_tensor);
    free(input_float);
    return -1;
  }

  for (int c = 0; c < channels; ++c) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        size_t src_idx = ((size_t)c * (size_t)height + (size_t)y) * (size_t)width + (size_t)x;
        size_t dst_idx = ((size_t)y * (size_t)width + (size_t)x) * (size_t)channels + (size_t)c;
        float val = output_float[src_idx] * 255.0f;
        if (val < 0)
          val = 0;
        if (val > 255)
          val = 255;
        output->data[dst_idx] = (unsigned char)roundf(val);
      }
    }
  }

  g_ort->ReleaseValue(output_tensor);
  free(input_float);

  return 0;
}

const char* onnx_get_last_error(void) {
  return g_last_error;
}
