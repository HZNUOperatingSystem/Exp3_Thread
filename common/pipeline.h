#ifndef COMMON_PIPELINE_H_
#define COMMON_PIPELINE_H_

#include "common/dataset.h"
#include "common/filter.h"

typedef struct {
  double psnr_before;
  double psnr_after;
  double ssim_before;
  double ssim_after;
  int ssim_available;
  int status_code;
} ImageResult;

int pipeline_process_one_image(const ImageJob* job,
                               const FilterConfig* config,
                               int compute_ssim,
                               ImageResult* out_result);
int pipeline_write_metrics_csv(const char* path,
                               const ImageJob jobs[],
                               const ImageResult results[],
                               int count);

#endif
