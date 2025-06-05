#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

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
void num2str(char *buf, uint64_t num, bool is_signed, int base, bool upper) {
  char tmp[65]; // 最多64位 + '\0'
  int i = 0;
  bool neg = false;
  const char *digits = upper ? digits_u : digits_l;

  if (base < 2 || base > 36) {
    buf[0] = '\0';
    return;
  }

  if (is_signed && (int64_t)num < 0) {
    neg = true;
    num = (uint64_t)(-(int64_t)num);  // 转为正数处理（INT64_MIN 正确处理）
  }

  if (num == 0) {
    tmp[i++] = '0';
  } else {
    while (num > 0) {
      tmp[i++] = digits[num % base];
      num /= base;
    }
  }

  if (neg) tmp[i++] = '-';

  // 反转复制到 buf
  for (int j = 0; j < i; ++j) {
    buf[j] = tmp[i - j - 1];
  }
  buf[i] = '\0';
}


int sprintf(char *out, const char *fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);
  char * p = out;
  int len = strlen(fmt);
  char tmp_str[20] = {'\0'};
  for (int i = 0; i < len; ++i)
  {
    if (fmt[i] == '%' && i + 1 < len){
      if (fmt[i + 1] == 'd')
      {
        int temp = va_arg(arglist, int);
        num2str(tmp_str, temp,true,10,false);
		for (char *s = tmp_str; *s; ++s) *p++ = *s;
      }
      else if (fmt[i + 1] == 's')
      {
        char *tmp_s = va_arg(arglist, char *);
		for (; *tmp_s; ++tmp_s) *p++ = *tmp_s;
      }
      i = i + 1;//跳过这次的%s或者%d
    }
    else{
       *p++ = fmt[i];
    }
  }
  *p = '\0';
  va_end(arglist);
  return strlen(out);//返回这次转换的字符数
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
