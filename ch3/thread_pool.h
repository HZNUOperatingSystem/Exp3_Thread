#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <pthread.h>

typedef void (*thread_task_fn)(void*);

typedef struct {
  thread_task_fn fn;
  void* arg;
} ThreadTask;

typedef struct {
  pthread_t* threads;
  ThreadTask* queue;
  int thread_count;
  int queue_capacity;
  int queue_size;
  int queue_head;
  int queue_tail;
  int stop;
  int working_count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  pthread_cond_t all_done;
} ThreadPool;

int thread_pool_init(ThreadPool* pool, int thread_count, int queue_capacity);
int thread_pool_submit(ThreadPool* pool, thread_task_fn fn, void* arg);
int thread_pool_wait(ThreadPool* pool);
int thread_pool_destroy(ThreadPool* pool);

#endif
