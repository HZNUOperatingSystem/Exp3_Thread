#include "thread_pool.h"

#include <stdlib.h>
#include <string.h>

static void* thread_pool_worker(void* arg) {
  /* Starter note:
   * - If you added temporary variables only to silence warnings, delete them
   *   before you start the real implementation.
   */
  ThreadPool* pool = (ThreadPool*)arg;

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
  int i;

  if (pool == NULL || thread_count <= 0 || queue_capacity <= 0) {
    return -1;
  }

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

  for (i = 0; i < thread_count; ++i) {
    if (pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool) != 0) {
      pthread_mutex_lock(&pool->mutex);
      pool->stop = 1;
      pthread_cond_broadcast(&pool->not_empty);
      pthread_mutex_unlock(&pool->mutex);
      while (--i >= 0) {
        pthread_join(pool->threads[i], NULL);
      }
      pthread_mutex_destroy(&pool->mutex);
      pthread_cond_destroy(&pool->not_empty);
      pthread_cond_destroy(&pool->not_full);
      pthread_cond_destroy(&pool->all_done);
      free(pool->threads);
      free(pool->queue);
      memset(pool, 0, sizeof(*pool));
      return -1;
    }
  }

  return 0;
}

int thread_pool_submit(ThreadPool* pool, thread_task_fn fn, void* arg) {
  /* Starter note:
   * - The basic parameter check has already been provided.
   * - Delete the temporary (void) lines below once you start implementing the
   *   real queue logic.
   */
  (void)pool;
  (void)fn;
  (void)arg;
  if (pool == NULL || fn == NULL) {
    return -1;
  }

  /* TODO:
   * 1. Lock the mutex.
   * 2. Wait on not_full while the queue is full and stop == 0.
   */
  
  if (pool->stop) {
    pthread_mutex_unlock(&pool->mutex);
    return -1;
  }

  /* TODO:
   * 3. Push the new task into the circular queue.
   * 4. Signal not_empty and unlock.
   */
  return -1;
}

int thread_pool_wait(ThreadPool* pool) {
  if (pool == NULL) {
    return -1;
  }

  /* TODO:
   * The NULL check has already been provided.
   * Finish the synchronization logic so this function waits until
   * queue_size == 0 and working_count == 0.
   */
  return -1;
}

int thread_pool_destroy(ThreadPool* pool) {
  if (pool == NULL) {
    return -1;
  }

  /* TODO:
   * The NULL check and final cleanup code are already provided.
   * 1. Lock the mutex and set stop = 1.
   * 2. Wake up all workers.
   * 3. Join every worker thread.
   *    This pthread_join loop should look similar to the pthread_create loop in
   *    thread_pool_init().
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
