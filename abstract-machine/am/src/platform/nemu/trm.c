#include <am.h>
#include <nemu.h>

extern char _heap_start; //外部符号 在链接器脚本scripts/linker.ld中指定
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END); //定义了一个[_heap_start,PMEM_END)的heap
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in scripts/platform/nemu.mk CFLAGS 通过insert-arg脚本将mainargs指定的内容替换到MAINARGS_PLACEHOLDER处

//输出字符
void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

//结束程序运行
void halt(int code) {
  nemu_trap(code); //nemu_trap宏屏蔽了底层硬件的抽象

  // should not reach here
  while (1);
}

//初始化TRM
void _trm_init() {
  int ret = main(mainargs); //执行main函数
  halt(ret);
}
