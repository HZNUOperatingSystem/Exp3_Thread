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

  if (!images_match(lhs, rhs) || out_value == NULL) {
    return -1;
  }

  pixel_count = (double)(lhs->width * lhs->height);
  c1 = (0.01 * 255.0) * (0.01 * 255.0);
  c2 = (0.03 * 255.0) * (0.03 * 255.0);

  /* TODO: delete the temporary (void) lines below, then accumulate mean_x and mean_y. */
  for (y = 0; y < lhs->height; ++y) {
    for (x = 0; x < lhs->width; ++x) {
      size_t offset = ((size_t)y * (size_t)lhs->width + (size_t)x) * (size_t)lhs->channels;
      double gray_x = grayscale_value(lhs->data + offset, lhs->channels);
      double gray_y = grayscale_value(rhs->data + offset, rhs->channels);

      (void)gray_x;
      (void)gray_y;

      /* TODO: accumulate mean_x and mean_y. */
    }
  }

  mean_x /= pixel_count;
  mean_y /= pixel_count;

  /* TODO:
   * Write a second pass similar to the loop above.
   * Recompute gray_x and gray_y, then use:
   *   dx = gray_x - mean_x
   *   dy = gray_y - mean_y
   * to accumulate var_x, var_y, and cov_xy.
   */

  var_x /= pixel_count;
  var_y /= pixel_count;
  cov_xy /= pixel_count;

  (void)*out_value;
  return 0;
}
