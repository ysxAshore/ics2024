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
  str[length] = '\0';
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

void uint32_to_hex_string(uint32_t num, char *hex_str)
{
  const char hex_digits[] = "0123456789abcdef";
  int started = 0; // 标记是否遇到第一个非0字符
  int idx = 0;     // hex_str的索引

  // 如果num是0，则直接返回"0"
  if (num == 0)
    hex_str[idx++] = '0';
  else
  {
    for (int i = 7; i >= 0; i--)
    {
      // 提取最高位的4位
      uint8_t nibble = (num >> (i * 4)) & 0xF;
      // 跳过前导0
      if (nibble != 0 || started)
      {
        hex_str[idx++] = hex_digits[nibble];
        started = 1;
      }
    }
  }
  hex_str[idx] = '\0'; // 字符串结尾
}

int printf(const char *fmt, ...)
{
  char temp[1500];
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
  char tmp_str[1500] = {'\0'};
  *out = '\0';
  for (int i = 0; i < strlen(fmt); ++i)
  {
    if (fmt[i] == '%' && i + 1 < strlen(fmt))
    {
      int temp;
      int width;
      uint32_t temp_u;
      switch (fmt[i + 1])
      {
      case '0':
        width = fmt[i + 2] - '0';
        if (fmt[i + 3] == 'd')
        {
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
        }
        else if (fmt[i + 3] == 'x')
        {
          temp_u = va_arg(ap, uint32_t);
          uint32_to_hex_string(temp_u, tmp_str);
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
        }
        i = i + 2;
        break;
      case '1' ... '9':
        width = fmt[i + 1] - '0';
        if (fmt[i + 2] == 'd')
        {
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
        }
        else if (fmt[i + 2] == 'x')
        {
          temp_u = va_arg(ap, int);
          uint32_to_hex_string(temp_u, tmp_str);
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
        }
        i = i + 1;
        break;
      case 'c':
        tmp_str[0] = (char)va_arg(ap, int);
        tmp_str[1] = '\0';
        strcat(out, tmp_str);
        break;
      case 'd':
        temp = va_arg(ap, int);
        int2str(tmp_str, temp);
        printf("\n%s\n", tmp_str);
        strcat(out, tmp_str);
        break;
      case 's':
        strcpy(tmp_str, va_arg(ap, char *));
        strcat(out, tmp_str);
        break;
      case 'x':
        temp_u = va_arg(ap, uint32_t);
        uint32_to_hex_string(temp_u, tmp_str);
        strcat(out, tmp_str);
        break;
      case 'p':
        temp_u = va_arg(ap, uint32_t);
        tmp_str[0] = '0';
        tmp_str[1] = 'x';
        uint32_to_hex_string(temp_u, tmp_str + 2);
        strcat(out, tmp_str);
        break;
      default:
        tmp_str[0] = fmt[i];
        tmp_str[1] = '\0';
        strcat(out, tmp_str);
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
