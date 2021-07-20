#include "scheduler.h"

#if !defined(FREESTANDING)
#include <stdio.h>
#include <stdlib.h>
#else

#ifndef NULL
#define NULL (void*)0
#endif

#define main _start

typedef unsigned long long size_t;

static _Noreturn void exit(int code) {
  asm volatile(
    "movq $60, %%rax;"
    "movq %0, %%rdi;"
    "syscall;"
    :
    : "r"((size_t)code)
    : "rax", "rdi"
  );
  __builtin_unreachable();
}

static char memory[1024 * 1024];
static size_t allocated = 0; 

static void* malloc(size_t n) {
  if (allocated + n > sizeof(memory)) {
    return NULL;
  }
  char* p = memory + allocated;
  allocated += n;
  return p;
}

static void free(void* p) {
  (void)p;
}

static void puts(const char* s) {
  char buffer[256];
  size_t i = 0;
  while (s[i]) {
    if (i == 254) {
      break;
    }    

    buffer[i] = s[i];
    ++i;
  }
  buffer[i] = '\n';

  asm volatile(
    "movq $1, %%rax;"
    "movq $1, %%rdi;"
    "movq %0, %%rsi;"
    "movq %1, %%rdx;"
    "syscall;"
    :
    : "r" (buffer), "r"(i+1)
    : "rax", "rdi", "rsi", "rdx"
  );
}
#endif

static Task* task_alloc(void* allocator, int* stack_size) {
  (void)allocator;
  static const int STACK_SIZE = 16 * 1024;
  Task* task = malloc(sizeof(Task));
  task->stack = malloc(STACK_SIZE);
  *stack_size = STACK_SIZE;
  return task;
}

static void task_free(void* allocator, Task* task) {
  (void)allocator;
  free(task->stack);
  free(task);
}

static Scheduler scheduler;

static void coro_fn(void* arg) {
  puts(arg);
  scheduler_yield(&scheduler);
  puts(arg);
}

int main(void) {
  scheduler_init(&scheduler, NULL, task_alloc, task_free);
  scheduler_spawn(&scheduler, coro_fn, "Hello from coro 1");
  scheduler_spawn(&scheduler, coro_fn, "Hello from coro 2");
  scheduler_spawn(&scheduler, coro_fn, "Hello from coro 3");
  scheduler_run(&scheduler);
  puts("Finished");
  exit(0);
}
