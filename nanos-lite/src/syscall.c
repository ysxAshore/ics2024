#include <common.h>
#include "syscall.h"
void do_syscall(Context *c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0])
  {
  case 0: // SYS_exit syscall
    c->GPRx = 0;
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_exit syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif
    halt(c->GPR2);
    break;
  case 1: // SYS_yield　syscall
    yield();
    c->GPRx = 0;
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_yield syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
