#include <common.h>
#include <proc.h>

void do_syscall(Context *c);
static Context *do_event(Event e, Context *c)
{
  switch (e.event)
  {
  case EVENT_YIELD:
    Log("Encouter a yield exception event");
    c = schedule(c);
    // c->GPRx = (uintptr_t)heap.end; //不能加在这,这是只有用户程序才设置栈起点，但是不影响运行
    break;
  case EVENT_SYSCALL:
    do_syscall(c);
    break;
  case EVENT_IRQ_TIMER: // native的AM在创建上下文的时候默认会打开中断,需要在事件处理回调函数中识别出时钟中断事件，但是直接返回上下文
    break;
  case EVENT_IRQ_IODEV:
    break;
  default:
    panic("Unhandled event ID = %d", e.event);
  }
  return c;
}

void init_irq(void)
{
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
