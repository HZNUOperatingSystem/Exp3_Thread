#include "common/pipeline.h"

#define IMAGE_PARALLEL_FOR
/* TODO: replace the line above with:
 * #define IMAGE_PARALLEL_FOR _Pragma("omp parallel for schedule(dynamic, 1)")
 */

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, ImageResult results[],
                        int job_count) {
  int i;

  IMAGE_PARALLEL_FOR
  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], config, 0, &results[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch(execute_jobs);
}
