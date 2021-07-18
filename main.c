#include "coro.h"

#include <stdio.h>
#include <stdlib.h>

typedef void (*Fn)(void*);

typedef struct Task {
  char* stack;
  Coro* coro;
  Fn fn;
  void* arg;

  struct Task* next;
} Task;

typedef struct TaskQueue {
  Task* first;
  Task* last;
} TaskQueue;


static TaskQueue tasks = {
  .first = NULL,
  .last = NULL
};

static Task* current = NULL;

static Task* pop_task(TaskQueue* queue) {
  Task* task = queue->first;
  if (!task) {
    return NULL;
  }

  Task* last = queue->last;
  if (task != last) {
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

static Coro main_ctx;

static void schedule(Coro* swap) {
  Task* task = pop_task(&tasks);
  if (!task) {
    Coro waste;
    coro_swap(&waste, &main_ctx);
  }
  
  current = task; 
  coro_swap(swap, task->coro); 
}

static void run_tasks(void) {
  schedule(&main_ctx);
}

static void yield(void) {
  push_task(&tasks, current);
  schedule(current->coro); 
}

static void coro_main(Coro* self, void* task) {
  Task* t = (Task*)task;
  t->fn(t->arg);
  
  current = NULL;
  free(t->stack);
  free(t->coro);
  free(t);

  Coro waste;
  schedule(&waste);
}

static void spawn(Fn fn, void* arg) {
  static int STACK_SIZE = 16 * 1024;
  char* stack = malloc(STACK_SIZE);
  Coro* coro = malloc(sizeof(Coro));
  
  Task* task = malloc(sizeof(Task));
  task->stack = stack;
  task->coro = coro;
  task->fn = fn;
  task->arg = arg;
  task->next = NULL; 
 
  coro_init(coro, stack, STACK_SIZE, coro_main, task);
  push_task(&tasks, task);
}

static void inner_fn(void* arg) {
  int id = (int)arg;
  printf("Hello from inner coroutine #%d (child of %d)\n", id, id / 2);
}

static void coro_fn(void* arg) {
  int id = (int)arg;
  printf("Hello from coroutine #%d\n", id);
  spawn(inner_fn, (void*)(id * 2));
  yield();
  printf("Hello from coroutine #%d, again\n", id);
}

int main(void) {
  for (int i = 1; i < 4; ++i) {
    spawn(coro_fn, (void*)i);
  }

  run_tasks();
  printf("Finished\n");
  return 0; 
}
