#include <memory.h>
#include <proc.h>

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
  current->max_brk = ROUNDUP(current->max_brk, PGSIZE);
  if (brk > current->max_brk)
  {
    int page_count = ROUNDUP(brk - current->max_brk, PGSIZE) >> 12;
    void *va = (void *)current->max_brk;
    void *pages_start = new_page(page_count) - PGSIZE * page_count;
    memset(pages_start, 0, page_count * PGSIZE);
    for (int i = 0; i < page_count; ++i)
      map(&current->as,
          va + i * PGSIZE,
          pages_start + i * PGSIZE,
          3);
    current->max_brk += page_count * PGSIZE;
  }
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
