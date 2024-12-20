#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void)
{
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
  next = seed;
}

int abs(int x)
{
  return (x < 0 ? -x : x);
}

int atoi(const char *nptr)
{
  int x = 0;
  while (*nptr == ' ')
  {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9')
  {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

static void *addr = NULL;
static bool hasInitialed = false;
void *malloc(size_t size)
{
  if (!hasInitialed)
  {
    addr = heap.start;
    hasInitialed = true;
  }
  if (size == 0)
    return NULL;
  else
  {
    // 检查是否有足够的内存
    if (addr + size > heap.end)
      return NULL;
    void *this_assigned = addr;
    size = (size_t)ROUNDUP(size, 8);
    addr = addr + size;
    return (void *)this_assigned;
  }
}

void free(void *ptr)
{
}

#endif
