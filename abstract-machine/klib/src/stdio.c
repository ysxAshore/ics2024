#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void int2str(char *str, int value)
{
  // 处理负数
  int is_negative = value < 0;
  if (value == INT_MIN)
  {
    strcpy(str, "-2147483648");
    return;
  }
  if (is_negative)
  {
    value = -value; // 处理INT_MIN时会溢出
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

int printf(const char *fmt, ...)
{
  char temp[256];
  va_list arglist;
  va_start(arglist, fmt);
  int num = vsprintf(temp, fmt, arglist);
  va_end(arglist);
  for (int i = 0; i < num; i++)
    putch(temp[i]);
  return num;
}
int vsprintf(char *out, const char *fmt, va_list ap)
{
  char tmp_str[32] = {'\0'};
  *out = '\0';
  for (int i = 0; i < strlen(fmt); ++i)
  {
    if (fmt[i] == '%' && i + 1 < strlen(fmt))
    {
      int temp;
      int width;
      switch (fmt[i + 1])
      {
      case '0':
        width = fmt[i + 2] - '0';
        temp = va_arg(ap, int);
        int2str(tmp_str, temp);
        if (strlen(tmp_str) >= width)
          strcat(out, tmp_str);
        else
        {
          int right = width - strlen(tmp_str);
          for (int j = strlen(tmp_str); j >= 0; --j)
            tmp_str[j + right] = tmp_str[j];
          for (int j = 0; j < right; ++j)
            tmp_str[j] = '0';
          strcat(out, tmp_str);
        }
        i = i + 2;
        break;
      case '1' ... '9':
        width = fmt[i + 2] - '0';
        temp = va_arg(ap, int);
        int2str(tmp_str, temp);
        if (strlen(tmp_str) >= width)
          strcat(out, tmp_str);
        else
        {
          int right = width - strlen(tmp_str);
          for (int j = strlen(tmp_str); j >= 0; --j)
            tmp_str[j + right] = tmp_str[j];
          for (int j = 0; j < right; ++j)
            tmp_str[j] = ' ';
          strcat(out, tmp_str);
        }
        i = i + 1;
        break;
      case 'c':
        tmp_str[0] = fmt[i + 1];
        tmp_str[1] = '\0';
        strcat(out, tmp_str);
        break;
      case 'd':
        temp = va_arg(ap, int);
        int2str(tmp_str, temp);
        strcat(out, tmp_str);
        break;
      case 's':
        strcpy(tmp_str, va_arg(ap, char *));
        strcat(out, tmp_str);
        break;
      default:
        break;
      }
      ++i;
    }
    else
    {
      tmp_str[0] = fmt[i];
      tmp_str[1] = '\0';
      strcat(out, tmp_str);
    }
  }
  return strlen(out);
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);
  int num = vsprintf(out, fmt, arglist);
  va_end(arglist);
  return num;
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
