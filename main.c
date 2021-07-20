#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>


static Scheduler scheduler;

static Task* task_alloc(void* allocator, int* stack_size) {
  (void)allocator;
  static const int STACK_SIZE = 16 * 1024;
  char* stack = malloc(STACK_SIZE);
  Task* task = malloc(sizeof(Task));
  task->stack = stack;
  *stack_size = STACK_SIZE;
  return task;
}

static void task_free(void* allocator, Task* task) {
  (void)allocator;
  free(task->stack);
  free(task);
}

static void yield(void) {
  scheduler_yield(&scheduler);
}

static void spawn(Fn fn, void* arg) {
  scheduler_spawn(&scheduler, fn, arg);
}

static void run_tasks(void) {
  scheduler_run(&scheduler);
}

static void inner_fn(void* arg) {
  int id = (int)arg;
  printf("Hello from inner coroutine #%d (child of %d)\n", id, id / 2);
}

static void coro_fn(void* arg) {
  int id = (int)arg;
  printf("Hello from coroutine #%d\n", id);
  spawn(inner_fn, (void*)(size_t)(id * 2));
  yield();
  printf("Hello from coroutine #%d, again\n", id);
}

int main(void) {
  scheduler_init(&scheduler, NULL, task_alloc, task_free);
  for (size_t i = 1; i < 4; ++i) {
    spawn(coro_fn, (void*)i);
  }

  run_tasks();
  printf("Finished\n");
  return 0; 
}
