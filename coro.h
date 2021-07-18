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

  // TODO: save these control registers, see https://www.uclibc.org/docs/psABI-x86_64.pdf page 21
  // mxcsr
  // x87 CW
} Coro;

// TODO: remove |self| parameter
typedef void (*CoroFn)(Coro* self, void* arg);

// initialize coroutine
void coro_init(Coro* coro, char* stack, int stack_size, CoroFn fn, void* arg);

// switch current context to |coro|
void coro_switch(Coro* coro);

// save current context to |coro|
void coro_save(Coro* coro);

// save current context to |current| and swap to |next| coro
void coro_swap(Coro* current, Coro* next);
