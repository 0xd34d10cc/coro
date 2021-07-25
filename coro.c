#include "coro.h"

#define NAKED __attribute__((naked))

NAKED static void trampoline(void) {
  asm volatile(
    "popq %rdi;"
    "popq %rax;"
    "callq *%rax;"
    "ud2;"
  );
}

void coro_init(Coro* coro, char* stack, int stack_size, CoroFn fn, void* arg) {
  // Stack should be aligned by 16, so this is fine 
  *(Reg*)(stack + stack_size - 8) = (Reg)fn;
  *(Reg*)(stack + stack_size - 16) = (Reg)arg;
  
  coro->rbx = 0;
  coro->rbp = (Reg)stack;
  coro->rsp = (Reg)stack + stack_size - 16;
  coro->r12 = 0;
  coro->r13 = 0;
  coro->r14 = 0;
  coro->r15 = 0;
  coro->rip = (Reg)trampoline;
}

NAKED void coro_swap(Coro* current, Coro* next) {
  asm volatile(
    // save current context
    // NOTE: there is no need to save volatile (caller saved) registers 
    "movq %rbx, 0(%rdi);"
    "movq %rbp, 8(%rdi);"
    "movq %rsp, 16(%rdi);"
    "movq %r12, 24(%rdi);"
    "movq %r13, 32(%rdi);"
    "movq %r14, 40(%rdi);"
    "movq %r15, 48(%rdi);"
    "leaq finish(%rip), %rax;"
    "movq %rax, 56(%rdi);"
    // load new context
    "movq 0(%rsi),  %rbx;"
    "movq 8(%rsi),  %rbp;"
    "movq 16(%rsi), %rsp;"
    "movq 24(%rsi), %r12;"
    "movq 32(%rsi), %r13;"
    "movq 40(%rsi), %r14;"
    "movq 48(%rsi), %r15;"
    "jmp *56(%rsi);"
    "finish:"
    "ret"
  );
}

void coro_switch(Coro* coro) {
  Coro waste;
  coro_swap(&waste, coro);
}
