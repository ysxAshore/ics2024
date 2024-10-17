/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\\n\", result); "
    "  return 0; "
    "}";
static uint16_t position = 0;
uint32_t choose(uint32_t n)
{
  return rand() % n;
}
uint16_t getFirstNullPos()
{
  for (uint16_t i = position; i < 65536; i++)
  {
    if (buf[i] == '\0')
    {
      return i;
    }
  }
}
void genSpace()
{
  position = getFirstNullPos();
  int N = choose(10);
  for (int i = 0; i < N; i++)
  {
    buf[position + i] = ' ';
  }
}
void gen(char c)
{
  position = getFirstNullPos();
  buf[position] = c;
}
void gen_num()
{
  position = getFirstNullPos();

  uint32_t n = choose(1000);
  sprintf(buf + position, "%d", n);
}
void gen_rand_op()
{
  position = getFirstNullPos();
  switch (choose(4))
  {
  case 0:
    buf[position] = '+';
    break;
  case 1:
    buf[position] = '-';
    break;
  case 2:
    buf[position] = '*';
  case 3:
    buf[position] = '/';
  default:
    break;
  }
}
static void gen_rand_expr()
{
  switch (choose(3))
  {
  case 0:
    genSpace();
    gen_num();
    break;
  case 1:
    genSpace();
    gen('(');
    gen_rand_expr();
    genSpace();
    gen(')');
    break;
  case 2:
    gen_rand_expr();
    genSpace();
    gen_rand_op();
    gen_rand_expr();
    break;
  }
}
uint16_t getFirstNotEmpty(int begin)
{
  for (uint16_t i = begin; i < 65536; i++)
  {
    if (buf[i] != ' ')
      return i;
  }
}
int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++)
  {
    position = 0;
    memset(buf, '\0', 65536);
    gen_rand_expr();

    position = getFirstNullPos();
    if (position > 100) // 表达式不超过300字符
    {
      --i;
      continue;
    }
    bool sign = false;
    for (int j = 0; j < position; j++)
    {
      if (buf[j] == '/' && buf[getFirstNotEmpty(j + 1)] == '0')
      {
        sign = true;
        break;
      }
    }
    if (sign)
    {
      --i;
      continue;
    }

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr"); // 执行错误的不写入
    if (ret != 0)
    {
      --i;
      continue;
    }

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
