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
  int x;
  int y;
  double mean_x = 0.0;
  double mean_y = 0.0;
  double var_x = 0.0;
  double var_y = 0.0;
  double cov_xy = 0.0;
  double c1;
  double c2;
  double pixel_count;

  /* Starter note:
   * Variable declarations, shape checking, pixel_count, C1, and C2 are already
   * provided.
   * Delete the temporary (void) lines below once you start implementing SSIM.
   */
  if (!images_match(lhs, rhs) || out_value == NULL) {
    return -1;
  }

  pixel_count = (double)(lhs->width * lhs->height);
  c1 = (0.01 * 255.0) * (0.01 * 255.0);
  c2 = (0.03 * 255.0) * (0.03 * 255.0);

  (void)grayscale_value;
  (void)x;
  (void)y;
  (void)mean_x;
  (void)mean_y;
  (void)var_x;
  (void)var_y;
  (void)cov_xy;
  (void)c1;
  (void)c2;
  (void)pixel_count;

  /* TODO:
   * Implement the simplified global SSIM used in this lab.
   *
   * Suggested steps:
   * 1. Delete the temporary (void) lines above.
   * 2. Convert each pixel to grayscale with grayscale_value(...).
   * 3. Compute the grayscale means mean_x and mean_y.
   * 4. Compute var_x, var_y, and cov_xy over the whole image.
   * 5. Store the result in *out_value.
   */
  *out_value = ((2.0 * mean_x * mean_y + c1) * (2.0 * cov_xy + c2)) /
               ((mean_x * mean_x + mean_y * mean_y + c1) * (var_x + var_y + c2));
  return 0;
}
