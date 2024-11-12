#include <common.h>
#include "syscall.h"
void do_syscall(Context *c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0])
  {
  case 0: // SYS_exit syscall
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_exit syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    halt(c->GPR2);
    break;
  case 1: // SYS_yield　syscall
    yield();
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_yield syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    break;
  case 4: // SYS_write　syscall
    int ret = 0;
    if (c->GPR2 == 1 || c->GPR2 == 2)
    {
      printf("\n%d\n", c->GPR2);
      for (int i = 0; i < c->GPR4; i++)
      {
        putch(((char *)c->GPR3)[i]);
        ++ret;
      }
    }
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_write syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, ret);
#endif
    c->GPRx = ret;
    break;
  case 9: //// SYS_brk　syscall
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_brk syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
