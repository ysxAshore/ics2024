#include <memory.h>

static void *pf = NULL;

void *new_page(size_t nr_page)
{
  // 堆是向上增长的
  pf = pf + nr_page * PGSIZE;
  return pf;
}

#ifdef HAS_VME
static void *pg_alloc(int n)
{
  size_t nr_page = n / PGSIZE;
  nr_page = n % PGSIZE == 0 ? nr_page : nr_page + 1;
  void *ptr = new_page(nr_page); // 分配的是未更新pf前的pf~pf+nr_page*PGSIZE
  void *start = ptr - nr_page * PGSIZE;
  memset(start, 0, nr_page * PGSIZE);
  return start;
}
#endif

void free_page(void *p)
{
  // 顺序分配不会回收
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk)
{
  return 0;
}

void init_mm()
{
  pf = (void *)ROUNDUP(heap.start, PGSIZE); // initialize heap pointer
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
