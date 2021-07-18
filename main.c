#include "coro.h"

static unsigned long strlen(const char* s) {
  unsigned long len = 0;
  while (*s++) ++len;
  return len;
}

static void printf(const char* s) {
  asm volatile(
    "movq $1, %%rax;"
    "movq $1, %%rdi;"
    "movq %0, %%rsi;"
    "movq %1, %%rdx;"
    "syscall;"
   :
   : "r" (s), "r"(strlen(s))
   : "rax", "rdi", "rsi", "rdx"
  );
}

void coro_fn(Coro* self, void* main) {
  printf("Hello from coroutine\n");
  coro_swap(self, (Coro*)main);
  printf("Hello from coroutine, again\n");
  coro_swap(self, (Coro*)main);
}

int _start(void) {
  asm volatile(
    "andq $-16, %rsp;"
    "andq $-16, %rbp;"
  );  

  _Alignas(16) Coro main;
  _Alignas(16) Coro sub;
  
  _Alignas(16) char stack[1024];
  coro_init(&sub, stack, sizeof(stack), &coro_fn, &main);
  coro_swap(&main, &sub); 
  printf("Hello from main\n");
  coro_swap(&main, &sub);
  printf("Bye\n");

  asm volatile(
    "movq $60, %rax;"
    "movq $0,  %rdi;"
    "syscall;"
  );
  return 0; 
}
