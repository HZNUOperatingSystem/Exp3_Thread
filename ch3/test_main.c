#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "thread_pool.h"

typedef struct {
  pthread_mutex_t mutex;
  int value;
} Counter;

static void counter_add_one(void* arg) {
  Counter* counter = (Counter*)arg;
  pthread_mutex_lock(&counter->mutex);
  counter->value += 1;
  pthread_mutex_unlock(&counter->mutex);
}

static void counter_add_one_slow(void* arg) {
  struct timespec delay = {0, 200000000L};
  nanosleep(&delay, NULL);
  counter_add_one(arg);
}

static double elapsed_seconds(const struct timespec* start, const struct timespec* end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

int main(void) {
  ThreadPool pool;
  Counter counter;
  struct timespec start;
  struct timespec end;
  double seconds;
  int i;

  counter.value = 0;
  if (pthread_mutex_init(&counter.mutex, NULL) != 0) {
    return 1;
  }

  if (thread_pool_init(&pool, 4, 8) != 0) {
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < 8; ++i) {
    if (thread_pool_submit(&pool, counter_add_one_slow, &counter) != 0) {
      thread_pool_destroy(&pool);
      pthread_mutex_destroy(&counter.mutex);
      return 1;
    }
  }
  if (thread_pool_wait(&pool) != 0) {
    thread_pool_destroy(&pool);
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  seconds = elapsed_seconds(&start, &end);
  if (counter.value != 8 || seconds > 1.20) {
    thread_pool_destroy(&pool);
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }

  for (i = 0; i < 4; ++i) {
    if (thread_pool_submit(&pool, counter_add_one, &counter) != 0) {
      thread_pool_destroy(&pool);
      pthread_mutex_destroy(&counter.mutex);
      return 1;
    }
  }
  if (thread_pool_wait(&pool) != 0) {
    thread_pool_destroy(&pool);
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }

  if (counter.value != 12) {
    thread_pool_destroy(&pool);
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }

  if (thread_pool_destroy(&pool) != 0) {
    pthread_mutex_destroy(&counter.mutex);
    return 1;
  }

  pthread_mutex_destroy(&counter.mutex);
  return 0;
}
