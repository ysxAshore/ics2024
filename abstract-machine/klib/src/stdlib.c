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

// static uintptr_t hbrk = (uintptr_t)heap.start; //这里不能直接初始化 因为heap不是编译时常量
// 返回的地址必须是对齐的 且size=0时返回NULL
void *malloc(size_t size)
{
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
  static uintptr_t hbrk;
  static bool firstCall = false;
  if (!firstCall)
  {
    extern Area heap;
    hbrk = (uintptr_t)heap.start;
    firstCall = true;
  }
  if (size == 0)
    return NULL;
  size = (size_t)ROUNDUP(size, 8);
  char *old = (char *)hbrk;
  hbrk += size;
  assert((uintptr_t)heap.start <= hbrk && hbrk < (uintptr_t)heap.end);

  // reset
  memset(old, 0, size);
  return old;
#else
  return NULL;
#endif
}

void free(void *ptr)
{
}

#endif
