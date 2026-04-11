#include "common/metrics.h"

#include <math.h>
#include <stddef.h>

static int images_match(const ImageBuffer* lhs, const ImageBuffer* rhs) {
  return lhs != NULL && rhs != NULL && lhs->data != NULL && rhs->data != NULL &&
         lhs->width == rhs->width && lhs->height == rhs->height && lhs->channels == rhs->channels;
}

static double grayscale_value(const unsigned char* pixel, int channels) {
  if (channels == 1) {
    return (double)pixel[0];
  }

  return 0.299 * (double)pixel[0] + 0.587 * (double)pixel[1] + 0.114 * (double)pixel[2];
}

int metrics_compute_psnr(const ImageBuffer* lhs, const ImageBuffer* rhs, double* out_value) {
  size_t byte_count;
  size_t index;
  double mse = 0.0;

  if (!images_match(lhs, rhs) || out_value == NULL) {
    return -1;
  }

  byte_count = (size_t)lhs->width * (size_t)lhs->height * (size_t)lhs->channels;
  for (index = 0; index < byte_count; ++index) {
    double diff = (double)lhs->data[index] - (double)rhs->data[index];
    mse += diff * diff;
  }

  mse /= (double)byte_count;
  if (mse == 0.0) {
    *out_value = 99.0;
    return 0;
  }

  *out_value = 10.0 * log10((255.0 * 255.0) / mse);
  return 0;
}

int metrics_compute_ssim(const ImageBuffer* lhs, const ImageBuffer* rhs, double* out_value) {
  (void)grayscale_value;
  (void)lhs;
  (void)rhs;
  (void)out_value;

  /* TODO:
   * Implement the simplified global SSIM used in this lab.
   *
   * Suggested steps:
   * 1. Check that the two images have the same shape.
   * 2. Convert each pixel to grayscale with grayscale_value(...).
   * 3. Compute the grayscale means mean_x and mean_y.
   * 4. Compute var_x, var_y, and cov_xy over the whole image.
   * 5. Use:
   *      C1 = (0.01 * 255)^2
   *      C2 = (0.03 * 255)^2
   * 6. Fill:
   *      SSIM = ((2 * mean_x * mean_y + C1) * (2 * cov_xy + C2)) /
   *             ((mean_x * mean_x + mean_y * mean_y + C1) * (var_x + var_y + C2))
   */
  return -1;
}
