#include <stdio.h>

#include "ch3/thread_pool.h"
#include "common/dataset.h"
#include "common/filter.h"
#include "common/pipeline.h"

typedef struct {
  const ImageJob* job;
  const FilterConfig* config;
  ImageResult* result;
} BatchTask;

static void image_job_worker(void* arg) {
  BatchTask* task = (BatchTask*)arg;
  pipeline_process_one_image(task->job, task->config, 1, task->result);
}

int main(void) {
  const char* list_path = "image/list.txt";
  const char* input_dir = "image/input";
  const char* gt_dir = "image/gt";
  const char* output_root = "output";
  const char* output_dir = "output/ch4";
  const char* metrics_path = "output/ch4/metrics.csv";
  ImageJob jobs[MAX_IMAGE_JOBS];
  ImageResult results[MAX_IMAGE_JOBS] = {0};
  BatchTask tasks[MAX_IMAGE_JOBS];
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

  /* TODO:
   * 1. Finish ch3/thread_pool.c.
   * 2. Create a pool with 4 worker threads.
   * 3. Fill tasks[i].
   * 4. Submit every image task to the pool.
   * 5. Wait for all tasks to finish, then destroy the pool.
   *
   * The serial loop below is only a starter baseline.
   */
  for (i = 0; i < job_count; ++i) {
    tasks[i].job = &jobs[i];
    tasks[i].config = &config;
    tasks[i].result = &results[i];
    image_job_worker(&tasks[i]);
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
