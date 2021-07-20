#include "scheduler.h"

#ifndef NULL
#define NULL (void*)0
#endif

static Task* pop_task(TaskQueue* queue) {
  Task* task = queue->first;
  if (!task) {
    return NULL;
  }

  if (task != queue->last) {
    queue->first = task->next;
  } else {
    queue->first = NULL;
    queue->last = NULL;
  }

  // just in case
  task->next = NULL;
  return task;
}

static void push_task(TaskQueue* queue, Task* task) {
  if (!queue->first) {
    queue->first = task;
    queue->last = task;
  } else {
    queue->last->next = task;
    queue->last = task;
  }
}

static void schedule(Scheduler* scheduler, Coro* swap) {
  Task* task = pop_task(&scheduler->tasks);
  if (!task) {
    coro_switch(&scheduler->main_ctx);
  }
  
  scheduler->current = task;
  if (swap) {
    coro_swap(swap, &task->coro); 
  } else {
    coro_switch(&task->coro);
  }
}

static void coro_main(void* task) {
  Task* t = (Task*)task;
  t->fn(t->arg);

  Scheduler* scheduler = t->scheduler;  
  scheduler->task_free(scheduler->allocator, t);
  scheduler->current = NULL;
  schedule(scheduler, NULL);
}

void scheduler_init(Scheduler* scheduler, void* allocator, TaskAllocFn task_alloc, TaskFreeFn task_free) {
  char* p = (char*)scheduler;
  for (int i = 0; i < sizeof(Scheduler); ++i) {
    p[i] = 0;
  }
  scheduler->allocator = allocator;
  scheduler->task_alloc = task_alloc;
  scheduler->task_free = task_free;
}

void scheduler_run(Scheduler* scheduler) {
  schedule(scheduler, &scheduler->main_ctx);
}

void scheduler_yield(Scheduler* scheduler) {
  push_task(&scheduler->tasks, scheduler->current);
  schedule(scheduler, &scheduler->current->coro); 
}

void scheduler_spawn(Scheduler* scheduler, Fn fn, void* arg) {
  int stack_size;
  Task* task = scheduler->task_alloc(scheduler->allocator, &stack_size); 
  task->scheduler = scheduler;
  task->fn = fn;
  task->arg = arg;
  task->next = NULL; 
 
  coro_init(&task->coro, task->stack, stack_size, coro_main, task);
  push_task(&scheduler->tasks, task);
}

