#include "coro.h"

#include <stdio.h>

void coro_fn(Coro* self, void* main) {
  printf("Hello from coroutine\n");
  coro_swap(self, (Coro*)main);
  printf("Hello from coroutine, again\n");
  coro_swap(self, (Coro*)main);
}

int main(void) {
  Coro main;
  Coro sub;
  
  _Alignas(16) char stack[1024 * 16];
  coro_init(&sub, stack, sizeof(stack), &coro_fn, &main);
  coro_swap(&main, &sub); 
  printf("Hello from main\n");
  coro_swap(&main, &sub);
  printf("Bye\n");

  return 0; 
}
