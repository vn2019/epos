#include "../syscall-nr.h"

#define WRAPPER(name) \
  .globl _ ## name; \
_ ## name: \
    movl $SYSCALL_ ## name, %eax; \
    int $0x82; \
    ret

WRAPPER(task_exit)
WRAPPER(task_create)
WRAPPER(task_getid)
WRAPPER(task_yield)
WRAPPER(task_wait)
WRAPPER(beep)
WRAPPER(vm86)
WRAPPER(putchar)
WRAPPER(getchar)
