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
  ThreadPool pool;
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

  if (thread_pool_init(&pool, 4, job_count) != 0) {
    fprintf(stderr, "failed to initialize thread pool\n");
    return 1;
  }

  for (i = 0; i < job_count; ++i) {
    tasks[i].job = &jobs[i];
    tasks[i].config = &config;
    tasks[i].result = &results[i];
    if (thread_pool_submit(&pool, image_job_worker, &tasks[i]) != 0) {
      thread_pool_destroy(&pool);
      return 1;
    }
  }

  if (thread_pool_wait(&pool) != 0) {
    thread_pool_destroy(&pool);
    return 1;
  }

  if (thread_pool_destroy(&pool) != 0) {
    return 1;
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
