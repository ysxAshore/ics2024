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

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  if (args == NULL)
  {
    cpu_exec(-1);
  }
  else
  {
    printf("Unknown command 'c %s'\n", args);
  }
  return 0;
}

static int cmd_q(char *args)
{
  if (args == NULL)
  {
    nemu_state.state = NEMU_QUIT;
    cpu_exec(1);
  }
  else
  {
    printf("Unknown command 'q %s\n", args);
  }
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
  int N = 1;
  if (args != NULL)
  {
    if (strspn(args, "0123456789") == strlen(args))
    {
      N = atoi(args);
    }
    else
    {
      printf("Unknown command 'si %s\n", args);
      return 0;
    }
  }
  cpu_exec(N);
  return 0;
}

static int cmd_info(char *args)
{
  char *arg = strtok(NULL, " ");
  if (arg == NULL)
  {
    printf("Unknown command,info needs a subcmd,like r or w\n");
  }
  else
  {
    char *tmp = strtok(NULL, " ");
    if (tmp == NULL)
    {
      if (strcmp(arg, "r") == 0)
        isa_reg_display();
      else if (strcmp(arg, "w") == 0)
        displayAllWatch();
      else
        printf("now the info only is supported to display the status of registers and watches\n");
    }
    else
    {
      printf("Unknown command,info only needs a subcmd,like r or w\n");
    }
  }
  return 0;
}

static int cmd_x(char *args)
{
  char *num = strtok(NULL, " ");
  char *address = strtok(NULL, " ");
  int N = 0;
  paddr_t begin = CONFIG_MBASE;
  if (num == NULL || address == NULL)
  {
    printf("unknown command,the x command must need two aurguments- num and address\n");
  }
  else
  {
    N = atoi(num); // 没检测，由函数去做
    begin = (paddr_t)strtoul(address, NULL, 16);
    for (int i = 0; i < N; ++i)
    {
      printf("%#x: ", begin);
      printf(FMT_WORD, vaddr_read(begin, sizeof(word_t)));
      printf("\n");
      begin = begin + sizeof(word_t);
    }
  }
  return 0;
}

static int cmd_p(char *args)
{
  if (args == NULL)
    printf("p cmd needs one or more arguments");
  else
  {
    bool sign = true;
    word_t ans = expr(args, &sign);
    if (sign)
    {
      printf(FMT_WORD, ans);
      printf("\n");
    }
    else
      printf("The expr(%s) is not correct expression\n", args);
  }
  return 0;
}

static int cmd_w(char *args)
{
  if (args == NULL)
    printf("The w command needs a argument as expression to watch\n");
  else
    createAWatch(args);
  return 0;
}

static int cmd_d(char *args)
{
  int N;
  if (args != NULL)
  {
    if (strspn(args, "0123456789") == strlen(args))
    {
      N = atoi(args);
      if (N >= 0 && N < NR_WP)
        deleteOneWatch(N);
      else
        printf("The d command needs a digit as its arguments,from 0 to %d\n", NR_WP - 1);
    }
    else
      printf("The d command needs a digit as its arguments,from 0 to %d\n", NR_WP - 1);
  }
  else
    printf("The d command needs a digit as its arguments,from 0 to %d\n", NR_WP - 1);
  return 0;
}

static int cmd_test(char *args)
{
  FILE *fp = fopen("tools/gen-expr/output.txt", "r");
  assert(fp != NULL);
  char buf[200], myRes[11]; // buf不能设置太短
  while (fgets(buf, sizeof(buf), fp) != NULL)
  {
    char *ref_result = strtok(buf, " ");
    char *ref_expr = strtok(NULL, "\n");
    bool *success = NULL;
    uint32_t res = expr(ref_expr, success); // 这里不改64,因为output中是32
    if (success == NULL)
    {
      sprintf(myRes, "%u", res);
      if (strcmp(ref_result, myRes) == 0)
      {
        printf("%s=%s success\n", ref_expr, ref_result);
        continue;
      }
    }
    printf("%s=%s falied\n", ref_expr, ref_result);
  }
  return 0;
}
static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "'si N' excute program of N step(s)", cmd_si},
    {"info", "'info r/w' print the information of registers or watches", cmd_info},
    {"x", "'x N address' print the N memory from address beginning", cmd_x},
    {"p", "'p expr' print the answer of expr", cmd_p},
    {"w", "'w expr' record the expression as a watch", cmd_w},
    {"d", "'d N' is delete the watch of No.N", cmd_d},
    {"test", "test the function of expr", cmd_test},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    char *tmp = strtok(NULL, " ");
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0 && tmp == NULL)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
