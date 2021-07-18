#pragma once

typedef unsigned long long Reg;

typedef struct Coro {
  Reg rbx;
  Reg rbp;
  Reg rsp;
  Reg r12;
  Reg r13;
  Reg r14;
  Reg r15;
  Reg rip;  
  Reg flag;
  _Alignas(16) char fxsave[512];
} Coro;

typedef void (*CoroFn)(Coro* self, void* arg);

// initialize coroutine
void coro_init(Coro* coro, char* stack, int stack_size, CoroFn fn, void* arg);

// save current context to |current| and swap to |next| coro
void coro_swap(Coro* current, Coro* next);
