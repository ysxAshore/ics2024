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
  char *const argv[] = {NULL};
  char *const envp[] = {NULL};
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  switch_boot_pcb();
  Log("Initializing processes...");
}

static int i = 0;
Context *schedule(Context *prev)
{
  current->cp = prev;
  if (i < 20)
  {
    ++i;
    current = &pcb[1];
  }
  else
  {
    i = 0;
    current = (current == &pcb[1] ? &pcb[0] : &pcb[1]);
  }
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

void context_uload(PCB *pcb, char *filename, char *const argv[], char *const envp[])
{
  Area area;
  area.start = pcb->stack;
  area.end = pcb->stack + STACK_SIZE;

  protect(&pcb->as);

  uintptr_t *user_stack = (uintptr_t *)new_page(8);

  void *pa = (void *)user_stack;
  void *va = pcb->as.area.end;
  for (size_t i = 8; i >= 1; --i)
    map(&pcb->as, va - i * PGSIZE, pa - i * PGSIZE, 3);

  // 将参数按照ABI规定加载到用户栈中 栈是向下的
  // 1.获得argv和envp的数量
  int argc = 0, envc = 0;
  while (argv[argc] != NULL)
    ++argc;
  while (envp[envc] != NULL)
    ++envc;

  // 2.拷贝所有的字符串
  uintptr_t *strArr = (uintptr_t *)malloc((argc + envc) * sizeof(uintptr_t *)); // 相等的字符串重复存储
  int i = envc - 1;
  for (; i >= 0; --i)
  {
    uintptr_t size = strlen(envp[i]) + 1; // include '\0' 012\0 is 4 character but len is 3
    user_stack -= size;
    strArr[argc + i] = (uintptr_t)user_stack;
    strncpy((char *)user_stack, envp[i], size);
  }
  i = argc - 1;
  for (; i >= 0; --i)
  {
    uintptr_t size = strlen(argv[i]) + 1;
    user_stack -= size;
    strArr[i] = (uintptr_t)user_stack;
    strncpy((char *)user_stack, argv[i], size);
  }
  // 对齐到 uintptr_t 边界
  user_stack = (uintptr_t *)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));

  // 填充envp指针和argv指针和argc
  user_stack = user_stack - argc - envc - 3; // 4 is NULL NULL argc
  user_stack[0] = argc;
  for (int i = 0; i < argc; ++i)
    user_stack[i + 1] = (uintptr_t)strArr[i]; // strArr + i是指向字符串地址的那个单元的地址

  user_stack[argc + 1] = 0;
  for (int i = 0; i < envc; ++i)
    user_stack[argc + 2 + i] = (uintptr_t)strArr[argc + i];
  user_stack[argc + envc + 2] = 0;

  pcb->cp = ucontext(&pcb->as, area, (void *)loader(pcb, filename));
  pcb->cp->GPRx = (uintptr_t)user_stack;
}