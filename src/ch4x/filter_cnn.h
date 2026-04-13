#ifndef CH4_FILTER_CNN_H_
#define CH4_FILTER_CNN_H_

#include "common/image_io.h"

/**
 * @brief Apply CNN-based image restoration using ONNX model.
 *
 * Set CNN_MODEL_PATH environment variable to the .onnx model file path.
 *
 * @param input Input image buffer
 * @param output Output image buffer (must be pre-allocated)
 * @return 0 on success, non-zero on failure
 */
int filter_apply_cnn_impl(const ImageBuffer* input, ImageBuffer* output);

#endif
