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

typedef int (*ImageBatchExecutor)(const ImageJob jobs[], const FilterConfig* config,
                                  int compute_ssim, ImageResult results[], int job_count);

int pipeline_process_one_image(const ImageJob* job, const FilterConfig* config, int compute_ssim,
                               ImageResult* out_result);
int pipeline_run_image_batch(const char* chapter, int compute_ssim, ImageBatchExecutor executor);
int pipeline_write_metrics_csv(const char* path, const ImageJob jobs[], const ImageResult results[],
                               int count);

#endif
