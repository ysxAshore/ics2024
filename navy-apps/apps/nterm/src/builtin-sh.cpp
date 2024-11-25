#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...)
{
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner()
{
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt()
{
  sh_printf("sh> ");
}

static int cmd_help(char *args);

static int cmd_echo(char *args)
{
  if (args == NULL)
    sh_printf("\n");
  else
    sh_printf("%s\n", args);
  return 0;
}
static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"echo", "Display a line of text", cmd_echo},

};

static const char *program_table[] = {
    "nslider",
    "bmp-test",
    "dhrystone",
    "file-test",
    "coremark",
    "fixedptc-test",
    "dummy",
    "hello",
    "typing-game",
    "cpp-test",
    "bird",
    "event-test",
    "menu",
    "nterm",
    "pal",
    "timer-test"};
#define NR_CMD (int)(sizeof(cmd_table) / sizeof(cmd_table[0]))
#define PM_CMD (int)(sizeof(program_table) / sizeof(program_table[0]))

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
      sh_printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    char *tmp = strtok(NULL, " ");
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0 && tmp == NULL)
      {
        sh_printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    sh_printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_execve(char *args)
{
  setenv("PATH", "/bin", 0);
  if (execvp(args, NULL) == -1) // execvp failed return -1
    return -1;
  return 0;
}

void trim_whitespace(char *str)
{
  if (str == NULL)
    return;
  char *start = str; // 指向字符串起始位置
  char *end;

  // 找到第一个非空白字符
  while (*start && *start == ' ')
  {
    start++;
  }

  // 如果全是空格，则将字符串清空
  if (*start == '\0')
  {
    str[0] = '\0';
    return;
  }

  // 找到最后一个非空白字符
  end = start + strlen(start) - 1;
  while (end > start && *start == ' ')
  {
    end--;
  }

  // 截断字符串到正确长度
  *(end + 1) = '\0';

  // 将处理后的字符串移到原来的起始位置
  if (start != str)
  {
    memmove(str, start, end - start + 2);
  }
}

static void sh_handle_cmd(const char *cmd)
{
  // strlen strcpy strtk需要在非空时调用
  int cmd_len = strlen(cmd);
  char *str = (char *)malloc(cmd_len);
  strcpy(str, cmd);
  char *str_end = str + strlen(str);

  char *sub_cmd = strtok(str, " ");

  char *args = sub_cmd + strlen(sub_cmd) + 1;
  if (args >= str_end)
    args = NULL;

  if (sub_cmd[strlen(sub_cmd) - 1] == '\n')
    sub_cmd[strlen(sub_cmd) - 1] = '\0';
  if (args != NULL && args[strlen(args) - 1] == '\n')
    args[strlen(args) - 1] = '\0';

  // args去除前后的空格
  trim_whitespace(args);

  int i;
  if (sub_cmd[0] != '\0')
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(sub_cmd, cmd_table[i].name) == 0)
      {
        cmd_table[i].handler(args);
        return;
      }
    }
    for (i = 0; i < PM_CMD; ++i)
    {
      if (strcmp(sub_cmd, program_table[i]) == 0)
      {
        cmd_execve(sub_cmd);
        return;
      }
    }
    sh_printf("Unknown command '%s'\n", sub_cmd);
  }
}

void builtin_sh_run()
{
  sh_banner();
  sh_prompt();

  while (1)
  {
    SDL_Event ev;
    if (SDL_PollEvent(&ev))
    {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN)
      {
        const char *res = term->keypress(handle_key(&ev));
        if (res)
        {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
