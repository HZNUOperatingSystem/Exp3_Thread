#include "ch3/thread_pool.h"
#include "common/pipeline.h"

typedef struct {
  const ImageJob* job;
  const FilterConfig* config;
  int compute_ssim;
  ImageResult* result;
} BatchTask;

static void image_job_worker(void* arg) {
  BatchTask* task = (BatchTask*)arg;
  pipeline_process_one_image(task->job, task->config, task->compute_ssim, task->result);
}

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, int compute_ssim,
                        ImageResult results[], int job_count) {
  BatchTask tasks[MAX_IMAGE_JOBS];
  int i;

  /* TODO:
   * 1. Finish src/ch3/thread_pool.c.
   * 2. Create a pool with 4 worker threads.
   *    If thread_pool_init(...) fails, use:
   *    fprintf(stderr, "failed to initialize thread pool\n");
   *    return 1;
   * 3. Fill tasks[i].
   * 4. Submit every image task to the pool.
   * 5. Wait for all tasks to finish, then destroy the pool.
   *    If thread_pool_wait(...) fails, remember to call
   *    thread_pool_destroy(&pool) before returning.
   *
   * The serial loop below is only a starter baseline.
   */
  for (i = 0; i < job_count; ++i) {
    tasks[i].job = &jobs[i];
    tasks[i].config = config;
    tasks[i].compute_ssim = compute_ssim;
    tasks[i].result = &results[i];
    image_job_worker(&tasks[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch("ch4", 1, execute_jobs);
}
