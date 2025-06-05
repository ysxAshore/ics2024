#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

//统一使用指针处理 更加高效
size_t strlen(const char *s) {
	const char *p = s;
	while(*p++);
	return p - s - 1;
}

//不考虑重叠 按照字符串增序复制
char *strcpy(char *dst, const char *src) {
  assert(src!=NULL && dst!=NULL);
  char *p = dst;
  while ((*p++ = *src++)) {} 
  return dst;
}

//strncpy不保证以'\0'结尾
char *strncpy(char *dst, const char *src, size_t n) {
	assert(src!=NULL && dst != NULL);
	char *p = dst;
	while(n-- && (*p++ = *src++));
	while(n--) *p++ = '\0';
	return dst;
}

char *strcat(char *dst, const char *src) {
	size_t len = strlen(dst);
	strcpy(dst + len,src);
    return dst;
}

int strcmp(const char *s1, const char *s2) {
	assert(s1!=NULL && s2!=NULL);
	while(*s1 && (*s1 == *s2)){
		++s1;
		++s2;
	}
	return *s1 - *s2;	
}

int strncmp(const char *s1, const char *s2, size_t n) {
	if(n == 0) return 0;
	while(--n && *s1 && (*s1 == *s2)){ //当n=1时 执行 *s1-*s2 即上面的i=n-1的情况
		++s1;
		++s2;
	 }
	return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
	unsigned char * p = s;
	while(n--)
		*p++ = (unsigned char)c;
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
	//为什么不总进行反向复制——性能差
	//正向遍历更容易cahce命中 
	//一些平台对于不重叠时会调用memcpy
	//编译器对0...n的loop优化比n...0更好
	unsigned char *d = dst;
    const unsigned char *s = src;

	if(d == s)
		return dst;
    if (d < s || d >= s + n) {
        // 可以正向复制 d < s时可能会重叠 但是此时可以进行正向复制
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else {
        // 有重叠，反向复制
        for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
	unsigned char *d = out;
    const unsigned char *s = in;
	while(n--)
	    *d++ = *s++;
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const unsigned char *p1 = s1;
	const unsigned char *p2 = s2;
	if(n == 0) return 0;
	while(--n && *p1 && (*p1 == *p2)){ //当n=1时 执行 *s1-*s2 即上面的i=n-1的情况
		++p1;
		++p2;
	 }
	return *p1 - *p2;
	
}

#endif
