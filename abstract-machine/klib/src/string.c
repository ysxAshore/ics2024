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
  assert(s1 != NULL && s2 != NULL);
  size_t i = 0;
  for (; s1[i] != '\0' && s2[i] != '\0'; ++i)
  {
    if (s1[i] == s2[i])
      continue;
    else if (s1[i] < s2[i])
      return -1;
    else
      return 1;
  }
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;
  else if (s1[i] == '\0')
    return -1;
  else
    return 1;
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
  char *tmp1 = (char *)dst;
  char *tmp2 = (char *)src;
  char tmp[n];
  strncpy(tmp, tmp2, n);
  strncpy(tmp1, tmp, n);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
  uintptr_t out_start = (uintptr_t)out;
  uintptr_t out_end = out_start + n;
  uintptr_t in_start = (uintptr_t)in;
  uintptr_t in_end = in_start + n;
  assert((out_start < in_end) && (in_start < out_end));

  // 不重叠才能用memcpy
  char *tmp1 = (char *)out;
  char *tmp2 = (char *)in;
  strncpy(tmp1, tmp2, n);
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  char *tmp1 = (char *)s1;
  char *tmp2 = (char *)s2;
  return strncmp(tmp1, tmp2, n);
}

#endif
