#include <common.h>
#include <proc.h>
#include <sys/time.h>
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
char *getFdName(int fd);
void naive_uload(PCB *pcb, const char *filename);

int time_gettimeofday(struct timeval *tv, struct timezone *tz);

void do_syscall(Context *c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  int result;
  // Note that GPR2 and GPRx are the same register in RISCV
  // the assignment of GPRx needs to be placed after the use of GPR2
  switch (a[0])
  {
  case SYS_exit: // SYS_exit syscall
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_exit syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = 0;
    naive_uload(NULL, "/bin/nterm");
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

  case SYS_execve:
    if (fs_open((char *)c->GPR2, 0, 0) != -1)
    {
#ifdef CONFIG_STRACE
      Log("Thers is a SYS_execve syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
      context_uload(current, (char *)c->GPR2, (char **)c->GPR3, (char **)c->GPR4); // 此时pcb[1]的cp指向新的程序
      switch_boot_pcb();                                                           // 将原进程上下文保存在pcb boot中
      yield();                                                                     // 　终止执行
      c->GPRx = 0;
    }
    else
    {
#ifdef CONFIG_STRACE
      Log("Thers is a SYS_execve syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, -1);
#endif
      c->GPRx = -1;
    }
    break;

  case SYS_gettimeofday:
    result = time_gettimeofday((struct timeval *)c->GPR2, (struct timezone *)c->GPR3);
#ifdef CONFIG_STRACE
    Log("Thers is a SYS_gettimeofday syscall,the arguments is %p,%p,%p,the return value is %p", c->GPR2, c->GPR3, c->GPR4, 0);
#endif
    c->GPRx = result;
    break;

  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
