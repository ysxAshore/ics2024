#include <proc.h>
#include <stdint.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb()
{
  current = &pcb_boot;
}

void hello_fun(void *arg)
{
  int j = 1;
  while (1)
  {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j++;
    yield();
  }
}

void init_proc()
{
  context_kload(&pcb[0], hello_fun, "ysxAshore");
  context_uload(&pcb[1], "/bin/pal");
  switch_boot_pcb();

  Log("Initializing processes...");
}

Context *schedule(Context *prev)
{
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg)
{
  /*按照用户进程中的理解，Areaspace是用来表示用户PCB的可用空间,因此不这样用了
  //pcb->as.area isn't initialled
  pcb->as.area.start = pcb->stack;
  pcb->as.area.end = pcb->stack + STACK_SIZE;
  */
  Area area;
  area.start = pcb->stack;
  area.end = pcb->stack + STACK_SIZE;
  pcb->cp = kcontext(area, entry, arg);
}

void context_uload(PCB *pcb, char *filename)
{
  Area area;
  area.start = pcb->stack;
  area.end = pcb->stack + STACK_SIZE;
  pcb->cp = ucontext(&pcb->as, area, (void *)loader(pcb, filename));
  pcb->cp->GPRx = (uintptr_t)heap.end;
}