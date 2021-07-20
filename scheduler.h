#pragma once

#include "coro.h"

typedef void (*Fn)(void*);

typedef struct Task {
  char* stack;
  int stack_size;
  Coro coro;
  Fn fn;
  void* arg;

  struct Scheduler* scheduler;
  struct Task* next;
} Task;

typedef struct TaskQueue {
  Task* first;
  Task* last;
} TaskQueue;

// Allocate Task struct with initialized stack
typedef Task* (*TaskAllocFn)(void* allocator, int* stack_size);
// Deallocate task and its stack
typedef void (*TaskFreeFn)(void* allocator, Task* task);

typedef struct Scheduler {
  TaskQueue tasks;
  Task* current;
  Coro main_ctx;

  void* allocator;
  TaskAllocFn task_alloc;
  TaskFreeFn task_free;
} Scheduler;

// initialize the scheduler
void scheduler_init(Scheduler* scheduler, void* allocator, TaskAllocFn task_alloc, TaskFreeFn task_free);
// spawn a new task for fn
void scheduler_spawn(Scheduler* scheduler, Fn fn, void* arg);
// yield from current task to scheduler
void scheduler_yield(Scheduler* scheduler);
// run tasks
void scheduler_run(Scheduler* scheduler);
