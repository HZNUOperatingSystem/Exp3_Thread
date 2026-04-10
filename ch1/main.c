#include <stdio.h>

#include "common/dataset.h"
#include "common/filter.h"
#include "common/pipeline.h"

#define IMAGE_PARALLEL_FOR
/* TODO: replace the line above with:
 * #define IMAGE_PARALLEL_FOR _Pragma("omp parallel for schedule(dynamic, 1)")
 */

int main(void) {
  const char* list_path = "image/list.txt";
  const char* input_dir = "image/input";
  const char* gt_dir = "image/gt";
  const char* output_root = "output";
  const char* output_dir = "output/ch1";
  const char* metrics_path = "output/ch1/metrics.csv";
  ImageJob jobs[MAX_IMAGE_JOBS];
  ImageResult results[MAX_IMAGE_JOBS] = {0};
  FilterConfig config;
  int job_count;
  int i;

  filter_default_config(&config);

  job_count = dataset_load_jobs(list_path, input_dir, gt_dir, output_dir, jobs, MAX_IMAGE_JOBS);
  if (job_count <= 0) {
    fprintf(stderr, "failed to load jobs from %s\n", list_path);
    return 1;
  }

  if (dataset_ensure_directory(output_root) != 0 || dataset_ensure_directory(output_dir) != 0) {
    fprintf(stderr, "failed to create output directories\n");
    return 1;
  }

  IMAGE_PARALLEL_FOR
  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], &config, &results[i]);
  }

  if (pipeline_write_metrics_csv(metrics_path, jobs, results, job_count) != 0) {
    fprintf(stderr, "failed to write %s\n", metrics_path);
    return 1;
  }

  for (i = 0; i < job_count; ++i) {
    if (results[i].status_code != 0) {
      return 1;
    }
  }

  return 0;
}
