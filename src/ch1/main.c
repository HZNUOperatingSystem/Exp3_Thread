#include "common/pipeline.h"

#define IMAGE_PARALLEL_FOR
/* TODO: replace the line above with:
 * #define IMAGE_PARALLEL_FOR _Pragma("omp parallel for schedule(dynamic, 1)")
 */

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, int compute_ssim,
                        ImageResult results[], int job_count) {
  int i;

  IMAGE_PARALLEL_FOR
  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], config, compute_ssim, &results[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch("ch1", 0, execute_jobs);
}
