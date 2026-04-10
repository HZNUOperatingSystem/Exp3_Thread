#include "thread_pool.h"

#include <stdlib.h>
#include <string.h>

static void* thread_pool_worker(void* arg) {
  (void)arg;

  /* TODO:
   * 1. Lock the queue mutex.
   * 2. Wait on not_empty while the queue is empty and stop == 0.
   * 3. If stop == 1 and the queue is empty, unlock and exit the worker.
   * 4. Pop one task from the queue, update queue_size / queue_head / working_count.
   * 5. Signal not_full, unlock, run the task, then lock again.
   * 6. Decrease working_count and signal all_done when the pool becomes idle.
   */
  return NULL;
}

int thread_pool_init(ThreadPool* pool, int thread_count, int queue_capacity) {
  if (pool == NULL || thread_count <= 0 || queue_capacity <= 0) {
    return -1;
  }

  (void)thread_pool_worker;
  memset(pool, 0, sizeof(*pool));
  pool->thread_count = thread_count;
  pool->queue_capacity = queue_capacity;
  pool->threads = (pthread_t*)malloc((size_t)thread_count * sizeof(pthread_t));
  pool->queue = (ThreadTask*)malloc((size_t)queue_capacity * sizeof(ThreadTask));
  if (pool->threads == NULL || pool->queue == NULL) {
    free(pool->threads);
    free(pool->queue);
    memset(pool, 0, sizeof(*pool));
    return -1;
  }

  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->not_empty, NULL);
  pthread_cond_init(&pool->not_full, NULL);
  pthread_cond_init(&pool->all_done, NULL);

  /* TODO: create thread_count worker threads with pthread_create(..., thread_pool_worker, pool). */
  return -1;
}

int thread_pool_submit(ThreadPool* pool, thread_task_fn fn, void* arg) {
  (void)pool;
  (void)fn;
  (void)arg;

  /* TODO:
   * 1. Lock the mutex.
   * 2. Wait on not_full while the queue is full and stop == 0.
   * 3. Reject new tasks if stop == 1.
   * 4. Push the new task into the circular queue.
   * 5. Signal not_empty and unlock.
   */
  return -1;
}

int thread_pool_wait(ThreadPool* pool) {
  (void)pool;

  /* TODO: wait until queue_size == 0 and working_count == 0. */
  return -1;
}

int thread_pool_destroy(ThreadPool* pool) {
  if (pool == NULL) {
    return -1;
  }

  /* TODO:
   * 1. Lock the mutex and set stop = 1.
   * 2. Wake up all workers.
   * 3. Join every worker thread.
   */

  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->not_empty);
  pthread_cond_destroy(&pool->not_full);
  pthread_cond_destroy(&pool->all_done);

  free(pool->threads);
  free(pool->queue);
  memset(pool, 0, sizeof(*pool));

  return -1;
}
