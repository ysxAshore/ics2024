#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 支持2~36进制的数字字符
static const char digits_l[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char digits_u[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/**
 * 通用整数转字符串函数
 *
 * @param buf      输出缓冲区，需足够大
 * @param num      数值（可以是有符号或无符号的 64 位）
 * @param is_signed 是否是有符号数
 * @param base     进制（2~36）
 * @param upper    是否使用大写（用于16进制）
 */
void num2str(char *buf, uint64_t num, bool is_signed, int base, bool upper)
{
  char tmp[65]; // 最多64位 + '\0'
  int i = 0;
  bool neg = false;
  const char *digits = upper ? digits_u : digits_l;

  if (base < 2 || base > 36)
  {
    buf[0] = '\0';
    return;
  }

  if (is_signed && (int64_t)num < 0)
  {
    neg = true;
    num = (uint64_t)(-(int64_t)num); // 转为正数处理（INT64_MIN 正确处理）
  }

  if (num == 0)
  {
    tmp[i++] = '0';
  }
  else
  {
    while (num > 0)
    {
      tmp[i++] = digits[num % base];
      num /= base;
    }
  }

  if (neg)
    tmp[i++] = '-';

  // 反转复制到 buf
  for (int j = 0; j < i; ++j)
  {
    buf[j] = tmp[i - j - 1];
  }
  buf[i] = '\0';
}

int my_atoi_safe(const char *str)
{
  long res = 0;
  int sign = 1;

  while (*str == ' ')
    str++;

  if (*str == '+' || *str == '-')
  {
    if (*str == '-')
      sign = -1;
    str++;
  }

  while (*str >= '0' && *str <= '9')
  {
    res = res * 10 + (*str - '0');
    if (sign * res > INT_MAX)
      return INT_MAX;
    if (sign * res < INT_MIN)
      return INT_MIN;
    str++;
  }

  return (int)(sign * res);
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  char *p = out;
  int len = strlen(fmt);
  char tmp_str[20] = {'\0'};
  for (int i = 0; i < len; ++i)
  {
    if (fmt[i] == '%' && i + 1 < len)
    {
      int width = 0;
      char padding_char = ' ';
      int j = i + 1;

      if (fmt[j] == '0')
      {
        padding_char = '0';
        ++j;
      }

      // 字符串转数值
      while (fmt[j] >= '0' && fmt[j] <= '9')
      {
        width = width * 10 + fmt[j] - '0';
        if (width > INT_MAX)
          width = INT_MAX;
        if (width < INT_MIN)
          width = INT_MIN;
        ++j;
      }

      char spec = fmt[j];

      if (spec == 'd' || spec == 'o' || spec == 'O' || spec == 'x' || spec == 'X')
      {
        int base = 0;
        bool upper = false;
        switch (spec)
        {
        case 'd':
          base = 10;
          break;
        case 'O':
          upper = true;
        case 'o':
          base = 8;
          break;
        case 'X':
          upper = true;
        case 'x':
          base = 16;
          break;
        }
        long temp = va_arg(ap, long);
        num2str(tmp_str, temp, true, base, upper);
        int size = strlen(tmp_str);
        if (size >= width)
          for (char *s = tmp_str; *s; ++s)
            *p++ = *s;
        else
        {
          int rem = width - size;
          while (rem--)
            *p++ = padding_char;
          for (char *s = tmp_str; *s; ++s)
            *p++ = *s;
        }
      }
      else
      {
        switch (spec)
        {
        case 'c':
          char c = va_arg(ap, int);
          *p++ = c;
          break;
        case 's':
          char *tmp_s = va_arg(ap, char *);
          for (; *tmp_s; ++tmp_s)
            *p++ = *tmp_s;
          break;
        default:
          *p++ = spec;
          break;
        }
      }
      i = j;
    }
    else
      *p++ = fmt[i];
  }
  *p = '\0';
  va_end(ap);
  return strlen(out); // 返回这次转换的字符数
}

int printf(const char *fmt, ...)
{
  char s[strlen(fmt) + 256];
  va_list ap;
  va_start(ap, fmt);
  int n = vsprintf(s, fmt, ap);
  for (int i = 0; i < n; ++i)
    putch(s[i]);
  return n;
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);
  int n = vsprintf(out, fmt, arglist);
  return n;
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

#endif
