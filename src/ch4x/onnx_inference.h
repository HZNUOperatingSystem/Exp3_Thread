#ifndef CH4X_ONNX_INFERENCE_H_
#define CH4X_ONNX_INFERENCE_H_

#include "common/image_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OnnxSession OnnxSession;

OnnxSession* onnx_session_create(const char* model_path);
void onnx_session_release(OnnxSession* session);
int onnx_run_inference(OnnxSession* session, const ImageBuffer* input, ImageBuffer* output);
const char* onnx_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif
