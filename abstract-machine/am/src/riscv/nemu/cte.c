#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c)
{
  __am_get_cur_as(c);
  if (user_handler)
  {
    Event ev = {0};
    switch (c->mcause)
    {
    case 0xb: // m-mode下　0xb是自陷+系统调用
      if (c->GPR1 == -1)
        ev.event = EVENT_YIELD;
      else
        ev.event = EVENT_SYSCALL;
      break;
    case 0x8000000000000007:
      ev.event = EVENT_IRQ_TIMER;
      break;
    default:
      ev.event = EVENT_ERROR;
      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *))
{
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap)); // 将操作系统异常服务程序地址写入mtvec

  // register event handler
  user_handler = handler; // 使用handler来作为全局的user_handler
  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg)
{
  Context *cp = (Context *)(kstack.end - sizeof(Context)); // 按照图示
  cp->mepc = (uintptr_t)entry;                             // 指定入口点,NEMU会加４
  cp->mstatus = 0xa00001880;
  cp->GPR2 = (uintptr_t)arg; // #define GPR2 gpr[10] a0
  cp->pdir = NULL;
  cp->np = 1;
  cp->gpr[2] = (uintptr_t)kstack.end; // sp
  return cp;
}

void yield()
{
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled()
{
  return false;
}

void iset(bool enable)
{
}
