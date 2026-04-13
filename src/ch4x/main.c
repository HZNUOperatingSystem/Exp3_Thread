#include "ch4x/filter_cnn.h"
#include "common/pipeline.h"

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, ImageResult results[],
                        int job_count) {
  int i;
  FilterConfig local_config = *config;

  local_config.kind = FILTER_KIND_CNN;

  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], &local_config, 1, &results[i]);
  }

  return 0;
}

int main(void) {
  filter_set_cnn_impl(filter_apply_cnn_impl);
  return pipeline_run_image_batch(execute_jobs);
}
