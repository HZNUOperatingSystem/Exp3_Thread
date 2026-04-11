#include "common/pipeline.h"

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, ImageResult results[],
                        int job_count) {
  int i;

  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], config, 1, &results[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch(execute_jobs);
}
