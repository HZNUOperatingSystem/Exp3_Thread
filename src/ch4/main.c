#include <stdio.h>

#include "ch3/thread_pool.h"
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

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, ImageResult results[],
                        int job_count) {
  BatchTask tasks[MAX_IMAGE_JOBS];
  ThreadPool pool;
  int i;

  if (thread_pool_init(&pool, 4, job_count) != 0) {
    fprintf(stderr, "failed to initialize thread pool\n");
    return 1;
  }

  for (i = 0; i < job_count; ++i) {
    tasks[i].job = &jobs[i];
    tasks[i].config = config;
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

  return 0;
}

int main(void) {
  return pipeline_run_image_batch(execute_jobs);
}
