#include <stdio.h>

#include "common/dataset.h"
#include "common/filter.h"
#include "common/pipeline.h"
#include "thread_pool.h"

typedef struct {
  const ImageJob* job;
  const FilterConfig* config;
  ImageResult* result;
} BatchTask;

static void image_job_worker(void* arg) {
  BatchTask* task = (BatchTask*)arg;
  pipeline_process_one_image(task->job, task->config, task->result);
}

int main(void) {
  const char* list_path = "image/list.txt";
  const char* input_dir = "image/input";
  const char* gt_dir = "image/gt";
  const char* output_dir = "output";
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

  if (dataset_ensure_directory(output_dir) != 0) {
    fprintf(stderr, "failed to create %s\n", output_dir);
    return 1;
  }

  /* TODO:
   * 1. Cherry-pick your ch2 thread_pool.c implementation into this branch.
   * 2. Declare a ThreadPool and initialize it with 4 worker threads.
   * 3. Fill tasks[i] so each task points to one image job and one result slot.
   * 4. Submit every task with thread_pool_submit(..., image_job_worker, ...).
   * 5. Wait for all jobs to finish, then destroy the thread pool.
   *
   * The serial loop below is only a starter baseline so this branch still runs.
   * Replace it with your own thread-pool-based image batch processing.
   */
  for (i = 0; i < job_count; ++i) {
    tasks[i].job = &jobs[i];
    tasks[i].config = &config;
    tasks[i].result = &results[i];
    image_job_worker(&tasks[i]);
  }

  if (pipeline_write_metrics_csv("metrics.csv", jobs, results, job_count) != 0) {
    fprintf(stderr, "failed to write metrics.csv\n");
    return 1;
  }

  for (i = 0; i < job_count; ++i) {
    if (results[i].status_code != 0) {
      return 1;
    }
  }

  return 0;
}
