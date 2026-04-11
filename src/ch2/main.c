#include "common/pipeline.h"

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, int compute_ssim,
                        ImageResult results[], int job_count) {
  int i;

  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], config, compute_ssim, &results[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch("ch2", 1, execute_jobs);
}
