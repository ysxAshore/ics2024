#ifndef AM_H__
#define AM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include ARCH_H // this macro is defined in $CFLAGS
                // examples: "arch/x86-qemu.h", "arch/native.h", ...

// Memory protection flags
#define MMAP_NONE  0x00000000 // no access
#define MMAP_READ  0x00000001 // can read
#define MMAP_WRITE 0x00000002 // can write

// Memory area for [@start, @end)
typedef struct {
  void *start, *end;
} Area;

// Arch-dependent processor context
typedef struct Context Context;

// An event of type @event, caused by @cause of pointer @ref
typedef struct {
  enum {
    EVENT_NULL = 0,
    EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
    EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,
  } event;
  uintptr_t cause, ref;
  const char *msg;
} Event;

// A protected address space with user memory @area
// and arch-dependent @ptr
typedef struct {
  int pgsize;
  Area area;
  void *ptr;
} AddrSpace;

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------- TRM: Turing Machine -----------------------
extern   Area        heap; //可用的物理内存堆
void     putch       (char ch);
void     halt        (int code) __attribute__((__noreturn__)); //立即终止AM的运行并返回code

// -------------------- IOE: Input/Output Devices --------------------
bool     ioe_init    (void); //初始化IO设备
void     ioe_read    (int reg, void *buf);
void     ioe_write   (int reg, void *buf);
#include "amdev.h"

// ---------- CTE: Interrupt Handling and Context Switching ----------
bool     cte_init    (Context *(*handler)(Event ev, Context *ctx));
void     yield       (void); //陷入内核执行
bool     ienabled    (void); //读取/写入当前处理器的中断打开/屏蔽状态
void     iset        (bool enable);
Context *kcontext    (Area kstack, void (*entry)(void *), void *arg); //创建内核态运行的上下文

// ----------------------- VME: Virtual Memory -----------------------
bool     vme_init    (void *(*pgalloc)(int), void (*pgfree)(void *));
void     protect     (AddrSpace *as); //创建/销毁一个地址空间
void     unprotect   (AddrSpace *as);
void     map         (AddrSpace *as, void *vaddr, void *paddr, int prot); //对地址空间as建立vaddr->paddr的映射
Context *ucontext    (AddrSpace *as, Area kstack, void *entry); //创建被保护的用户态进程上下文

// ---------------------- MPE: Multi-Processing ----------------------
bool     mpe_init    (void (*entry)()); //启动多处理器
int      cpu_count   (void);			//返回系统中的处理器个数
int      cpu_current (void);			//当前处理器编号
int      atomic_xchg (int *addr, int newval); //原子的交换内存中数值 写入新值 返回旧值

#ifdef __cplusplus
}
#endif

#endif
