#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void __libc_init_array(void);

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args)
{
  int argc = (int)(args[0]);
  char **argv = (char **)(args + 1); // 不能用args[1] 相当于 *(args+1)指向字符串地址了 就是一个char*
  char **envp = (char **)(args + argc + 2);
  __libc_init_array();
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
