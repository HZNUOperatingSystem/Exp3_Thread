#ifndef COMMON_ONNX_INFERENCE_H_
#define COMMON_ONNX_INFERENCE_H_

#include "common/image_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OnnxSession OnnxSession;

/**
 * @brief Create an ONNX inference session from a model file.
 *
 * @param model_path Path to the .onnx model file
 * @return OnnxSession* Session handle, or NULL on failure
 */
OnnxSession* onnx_session_create(const char* model_path);

/**
 * @brief Release an ONNX session and free associated resources.
 *
 * @param session Session to release
 */
void onnx_session_release(OnnxSession* session);

/**
 * @brief Run inference on an image using the loaded ONNX model.
 *
 * The model is expected to accept input in NCHW format (batch, channels, height, width)
 * with normalized float values [0, 1] or [-1, 1] depending on training.
 * Output will be in the same shape as input.
 *
 * @param session ONNX session
 * @param input Input image buffer (RGB, 8-bit per channel)
 * @param output Output image buffer (must be pre-allocated with same dimensions as input)
 * @return 0 on success, non-zero on failure
 */
int onnx_run_inference(OnnxSession* session, const ImageBuffer* input, ImageBuffer* output);

/**
 * @brief Get the last error message from the ONNX runtime.
 *
 * @return const char* Error message string (valid until next API call)
 */
const char* onnx_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif
