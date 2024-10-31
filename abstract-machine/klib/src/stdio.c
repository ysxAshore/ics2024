#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...)
{
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
  panic("Not implemented");
}

void int2str(char *str, int value)
{
  // 处理负数
  int is_negative = value < 0;
  if (is_negative)
  {
    value = -value;
  }
  // 计算字符串长度
  int length = is_negative ? 1 : 0; // 负号的长度
  int temp = value;
  do
  {
    length++;
    temp /= 10;
  } while (temp != 0);
  for (int i = length - 1; i >= 0; i--)
  {
    str[i] = (value % 10) + '0'; // 转换为字符
    value /= 10;
  }
  if (is_negative)
  {
    str[0] = '-'; // 添加负号
  }
}
int sprintf(char *out, const char *fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);
  int num = 0;
  char tmp_str[20] = {'\0'};
  *out = '\0';
  for (int i = 0; i < strlen(fmt); ++i)
  {
    if (fmt[i] == '%' && i + 1 < strlen(fmt) && (fmt[i + 1] == 'd' || fmt[i + 1] == 's'))
    {
      ++num;
      if (fmt[i + 1] == 'd')
      {
        int temp = va_arg(arglist, int);
        int2str(tmp_str, temp);
        strcat(out, tmp_str);
      }
      else if (fmt[i + 1] == 's')
      {
        char *tmp_str = va_arg(arglist, char *);
        strcat(out, tmp_str);
      }
      i = i + 1;
    }
    else
    {
      tmp_str[0] = fmt[i];
      tmp_str[1] = '\0';
      strcat(out, tmp_str);
    }
  }
  va_end(arglist);
  return strlen(out);
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
