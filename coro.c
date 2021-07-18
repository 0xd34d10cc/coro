#include "coro.h"

__attribute__((naked)) static void trampoline(void) {
  asm volatile(
    "popq %rdi;"
    "popq %rsi;"
    "popq %rax;"
    "callq *%rax;"
    "ud2;"
  );
}

void coro_init(Coro* coro, char* stack, int stack_size, CoroFn fn, void* arg) {
  // Stack should be aligned by 16, so this is fine 
  *(Reg*)(stack + stack_size - 8) = (Reg)fn;
  *(Reg*)(stack + stack_size - 16) = (Reg)arg;
  *(Reg*)(stack + stack_size - 24) = (Reg)coro;
  
  coro->rbx = 0;
  coro->rbp = (Reg)stack + stack_size - 24;
  coro->rsp = (Reg)stack + stack_size - 24;
  coro->r12 = 0;
  coro->r13 = 0;
  coro->r14 = 0;
  coro->r15 = 0;
  coro->rip = (Reg)trampoline;
  coro->flag = 0;
}

void coro_swap(Coro* current, Coro* next) {
  void* restore = &&end;
  // save current context
  // NOTE: there is no need to save volatile registers because
  //       caller of this function (coro_swap) already assumes
  //       that their values are invalid after the call
  asm volatile(
    "movq %%rbx, 0(%0);"
    "movq %%rbp, 8(%0);"
    "movq %%rsp, 16(%0);"
    "movq %%r12, 24(%0);"
    "movq %%r13, 32(%0);"
    "movq %%r14, 40(%0);"
    "movq %%r15, 48(%0);"
    "movq %1, 56(%0);"
    "pushf;"
    "popq 64(%0);"
    :
    : "r" (current), "r" (restore)
    : "rax"
  );


  // load new context
  asm volatile(
    "movq 0(%0),  %%rbx;"
    "movq 8(%0),  %%rbp;"
    "movq 16(%0), %%rsp;"
    "movq 24(%0), %%r12;"
    "movq 32(%0), %%r13;"
    "movq 40(%0), %%r14;"
    "movq 48(%0), %%r15;"
    "push 64(%0);"
    "popfq;"
    "jmp *56(%0);"
    :
    : "r" (next)
  );

  end:
  // this is a hack to prevent label address from going outside of function 
  // TODO: make this function naked to avoid this problem
  asm volatile("nop");
  return;
}
