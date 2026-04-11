#include "thread_pool.h"

#include <stdlib.h>
#include <string.h>

static void* thread_pool_worker(void* arg) {
  ThreadPool* pool = (ThreadPool*)arg;

  for (;;) {
    ThreadTask task;

    pthread_mutex_lock(&pool->mutex);
    while (pool->queue_size == 0 && !pool->stop) {
      pthread_cond_wait(&pool->not_empty, &pool->mutex);
    }

    if (pool->stop && pool->queue_size == 0) {
      pthread_mutex_unlock(&pool->mutex);
      break;
    }

    task = pool->queue[pool->queue_head];
    pool->queue_head = (pool->queue_head + 1) % pool->queue_capacity;
    pool->queue_size -= 1;
    pool->working_count += 1;
    pthread_cond_signal(&pool->not_full);
    pthread_mutex_unlock(&pool->mutex);

    task.fn(task.arg);

    pthread_mutex_lock(&pool->mutex);
    pool->working_count -= 1;
    if (pool->queue_size == 0 && pool->working_count == 0) {
      pthread_cond_broadcast(&pool->all_done);
    }
    pthread_mutex_unlock(&pool->mutex);
  }

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
  if (pool == NULL || fn == NULL) {
    return -1;
  }

  pthread_mutex_lock(&pool->mutex);
  while (pool->queue_size == pool->queue_capacity && !pool->stop) {
    pthread_cond_wait(&pool->not_full, &pool->mutex);
  }

  if (pool->stop) {
    pthread_mutex_unlock(&pool->mutex);
    return -1;
  }

  pool->queue[pool->queue_tail].fn = fn;
  pool->queue[pool->queue_tail].arg = arg;
  pool->queue_tail = (pool->queue_tail + 1) % pool->queue_capacity;
  pool->queue_size += 1;
  pthread_cond_signal(&pool->not_empty);
  pthread_mutex_unlock(&pool->mutex);
  return 0;
}

int thread_pool_wait(ThreadPool* pool) {
  if (pool == NULL) {
    return -1;
  }

  pthread_mutex_lock(&pool->mutex);
  while (pool->queue_size > 0 || pool->working_count > 0) {
    pthread_cond_wait(&pool->all_done, &pool->mutex);
  }
  pthread_mutex_unlock(&pool->mutex);
  return 0;
}

int thread_pool_destroy(ThreadPool* pool) {
  int i;

  if (pool == NULL) {
    return -1;
  }

  pthread_mutex_lock(&pool->mutex);
  pool->stop = 1;
  pthread_cond_broadcast(&pool->not_empty);
  pthread_cond_broadcast(&pool->not_full);
  pthread_mutex_unlock(&pool->mutex);

  for (i = 0; i < pool->thread_count; ++i) {
    pthread_join(pool->threads[i], NULL);
  }

  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->not_empty);
  pthread_cond_destroy(&pool->not_full);
  pthread_cond_destroy(&pool->all_done);

  free(pool->threads);
  free(pool->queue);
  memset(pool, 0, sizeof(*pool));

  return 0;
}
