#ifndef COMMON_METRICS_H_
#define COMMON_METRICS_H_

#include "common/image_io.h"

int metrics_compute_psnr(const ImageBuffer* lhs, const ImageBuffer* rhs, double* out_value);
int metrics_compute_ssim(const ImageBuffer* lhs, const ImageBuffer* rhs, double* out_value);

#endif
