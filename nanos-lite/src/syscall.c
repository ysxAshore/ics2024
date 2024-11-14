#include <common.h>
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
char *getFdName(int fd);

void do_syscall(Context *c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  int result;
  switch (a[0])
  {
  case SYS_exit: // SYS_exit syscall
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_exit syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    halt(c->GPR2);
    break;

  case SYS_yield: // SYS_yield　syscall
    yield();
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_yield syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    break;

  case SYS_open:
    result = fs_open((char *)c->GPR2, c->GPR3, c->GPR4);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_open syscall,the arguments is %s,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, result);
#endif
    c->GPRx = result;
    break;

  case SYS_read:
    result = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_read syscall,the arguments is %s,%p,%p,the return value is %p", getFdName(c->GPR2), c->GPR3, c->GPR4, result);
#endif
    c->GPRx = result;
    break;

  case SYS_write: // SYS_write　syscall
    result = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_write syscall,the arguments is %s,%p,%p,the return value is %p", getFdName(c->GPR2), c->GPR3, c->GPR4, result);
#endif
    c->GPRx = result;
    break;

  case SYS_close:
    result = fs_close(c->GPR2);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_close syscall,the arguments is %s,%p,%p,the return value is %p", getFdName(c->GPR2), c->GPR3, c->GPR4, result);
#endif
    c->GPRx = result;
    break;

  case SYS_lseek:
    result = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_lseek syscall,the arguments is %s,%p,%p,the return value is %p", getFdName(c->GPR2), c->GPR3, c->GPR4, result);
#endif
    c->GPRx = result;
    break;

  case SYS_brk: //// SYS_brk　syscall
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_brk syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    break;

  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
