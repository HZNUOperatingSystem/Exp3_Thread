#ifndef CH4_FILTER_CNN_H_
#define CH4_FILTER_CNN_H_

#include "common/image_io.h"

/**
 * @brief Apply CNN-based image restoration using ONNX model.
 *
 * The model-loading logic is defined in the ch4x source files.
 *
 * @param input Input image buffer
 * @param output Output image buffer (must be pre-allocated)
 * @return 0 on success, non-zero on failure
 */
int filter_apply_cnn_impl(const ImageBuffer* input, ImageBuffer* output);

#endif
