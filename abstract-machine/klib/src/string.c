#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  assert(s != NULL);
  size_t i = 0;
  for (;; ++i)
  {
    if (s[i] == '\0')
      break;
  }
  return i;
}

char *strcpy(char *dst, const char *src)
{
  size_t i;
  assert(dst != NULL && src != NULL);
  for (i = 0; src[i] != '\0'; ++i)
    dst[i] = src[i];
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  size_t i;
  assert(dst != NULL && src != NULL);
  for (i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for (; i < n; i++)
    dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src)
{
  assert(dst != NULL && src != NULL);
  size_t dest_len = strlen(dst);
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dst[dest_len + i] = src[i];
  dst[dest_len + i] = '\0';

  return dst;
}
int strcmp(const char *s1, const char *s2)
{
  size_t i = 0;
  for (; s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i]; ++i)
    ;

  return (unsigned char)s1[i] - (unsigned char)s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  assert(s1 != NULL && s2 != NULL);
  size_t i = 0;
  for (; i < n && s1[i] != '\0' && s2[i] != '\0'; ++i)
  {
    if (s1[i] == s2[i])
      continue;
    else if (s1[i] < s2[i])
      return -1;
    else
      return 1;
  }
  if ((s1[i] == '\0' && s2[i] == '\0') || i == n)
    return 0;
  else if (s1[i] == '\0')
    return -1;
  else
    return 1;
}

void *memset(void *s, int c, size_t n)
{
  assert(s != NULL);
  char *tmp = (char *)s;
  size_t i = 0;
  for (; i < n; ++i)
  {
    tmp[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  // 如果 dest 和 src 是相同的地址，直接返回
  if (d == s)
  {
    return dst;
  }

  // 判断是否是重叠区域，src 在 dest 前：从后往前复制  src oveleap dest
  if (s < d && s + n > d)
  {
    // 从后向前复制
    d += n;
    s += n;
    while (n--)
    {
      *(--d) = *(--s);
    }
  }
  else
  {
    // 非重叠情况或 src 在 dest 前：从前向后复制
    while (n--)
    {
      *d++ = *s++;
    }
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
  unsigned char *d = (unsigned char *)out;
  const unsigned char *s = (const unsigned char *)in;
  while (n--)
  {
    *d++ = *s++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  unsigned char *tmp1 = (unsigned char *)s1;
  unsigned char *tmp2 = (unsigned char *)s2;
  for (size_t i = 0; i < n; i++)
  {
    if (*tmp1 > *tmp2)
      return 1;
    else if (*tmp1 < *tmp2)
      return -1;
    tmp1 = tmp1 + 1;
    tmp2 = tmp2 + 1;
  }
  return 0;
}

#endif
