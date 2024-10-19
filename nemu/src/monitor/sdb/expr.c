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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <memory/vaddr.h>
#include <isa.h>

enum
{
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  DEREF
  /* TODO: Add more token types */
};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},              // spaces one or more
    {"\\+", '+'},                   // plus
    {"\\-", '-'},                   // sub
    {"\\*", '*'},                   // mul
    {"/", '/'},                     // div
    {"\\(", '('},                   // left (
    {"\\)", ')'},                   // right )
    {"0[xX][0-9a-fA-F]{1,8}", 'h'}, // hexNum
    {"[0-9]+", 'd'},                // digit
    {"\\$[0-9a-zA-Z]{1,3}", 'r'},   // regName
    {"==", TK_EQ},                  // equal
    {"!=", TK_NEQ},                 // not_equal
    {"&&", TK_AND},                 // and
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[100] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case '+':
          tokens[nr_token].type = '+';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case '-':
          tokens[nr_token].type = '-';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case '*':
          tokens[nr_token].type = '*';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case '/':
          tokens[nr_token].type = '/';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case 'd':
          tokens[nr_token].type = 'd';
          if (substr_len > 32)
          {
            printf("the num %.*s is so big\n", substr_len, substr_start);
            return false;
          }
          else
          {
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0'; // 手动添加空字符
            ++nr_token;
          }
          break;
        case 'r':
          tokens[nr_token].type = 'r';
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0'; // 手动添加空字符
          ++nr_token;
          break;
        case 'h':
          tokens[nr_token].type = 'h';
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0'; // 手动添加空字符
          ++nr_token;
          break;
        case '(':
          tokens[nr_token].type = '(';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case ')':
          tokens[nr_token].type = ')';
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case TK_EQ:
          tokens[nr_token].type = TK_EQ;
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case TK_NEQ:
          tokens[nr_token].type = TK_NEQ;
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        case TK_AND:
          tokens[nr_token].type = TK_AND;
          tokens[nr_token].str[0] = '\0';
          ++nr_token;
          break;
        default:
        }
      }
    }
    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int error = 0;
bool check_parentheses(int p, int q)
{
  int i = 0;
  for (int j = p; j <= q; ++j)
  {
    if (tokens[j].type == '(')
      ++i;
    if (tokens[j].type == ')')
      --i;
    if (i < 0)
    {
      error = 1;
      return false;
    }
    // 需要排除()op()这种情况
    if (i == 0 && j != q)
    {
      return false;
    }
  }
  if (i == 0)
    return true;
  return false;
}

int getTheMainOp(int p, int q)
{
  int op = p;
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == 'd' || tokens[i].type == 'r' || tokens[i].type == 'h')
      continue;
    if (tokens[i].type == '(')
    { // 匹配到相对应的右括号
      int sum = 1;
      int j;
      for (j = i + 1; j <= q; ++j)
      {
        if (tokens[j].type == '(')
          ++sum;
        if (tokens[j].type == ')')
          --sum;
        if (sum < 0)
        {
          error = 1;
          return -1;
        }
        if (sum == 0)
          break;
      }
      i = j; // 跳过匹配的()内的所有运算符，这里如果括号存在不匹配会使得error=1
    }
    else
    {
      // 优先级比当前运算符低的才换
      if (tokens[op].type == '+' || tokens[op].type == '-')
      { // 只有ｉ表示+/-/==/!=/&&才替换
        if (tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ || tokens[i].type == TK_AND)
          op = i;
      }
      else if (tokens[op].type == '*' || tokens[op].type == '/')
      {
        if (tokens[i].type == '*' || tokens[i].type == '/' || tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ || tokens[i].type == TK_AND)
          op = i;
      }
      else if (tokens[op].type == TK_EQ || tokens[op].type == TK_NEQ)
      {
        if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ || tokens[i].type == TK_AND)
          op = i;
      }
      else if (tokens[op].type == TK_AND)
      {
      }
      else if (tokens[op].type == '_') // - * 5 - AND 5
      {
        if (i == op + 1)
          ;
        else
          op = i;
      }
      else if (tokens[op].type == DEREF)
      {
        if (i == op + 1 && (tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '_' || tokens[i].type == DEREF))
          ;
        else
          op = i;
      }
      else
        op = i;
    }
  }
  return op;
}

u_int64_t eval(int p, int q)
{
  if (p > q)
    return 0;
  else if (p == q)
  {
    if (tokens[p].str[0] == '\0')
      return 0;
    if (tokens[p].type == 'd')
      return strtoull(tokens[p].str, NULL, 10);
    if (tokens[p].type == 'h')
      return strtoull(tokens[p].str, NULL, 16);
    if (tokens[p].type == 'r')
    {
      bool find = false;
      word_t value = isa_reg_str2val(tokens[p].str, &find);
      if (find)
        return value;
      else
      {
        error = 1;
        return 0;
      }
    }
  }
  else if (tokens[p].type == '(' && tokens[q].type == ')' && check_parentheses(p, q))
    return eval(p + 1, q - 1);
  else
  {
    int op = getTheMainOp(p, q);

    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);
    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      return val1 / val2;
    case '_':
      return -1 * val2;
    case TK_EQ:
      return val1 == val2;
    case TK_NEQ:
      return val1 != val2;
    case TK_AND:
      return val1 && val2;
    case DEREF:
      return vaddr_read(val2, sizeof(uint32_t));
    default:
      error = 1;
    }
  }
  return 0;
}

word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i++)
  {
    if (tokens[i].type == '-' &&
        (i == 0 ||
         (i > 0 && tokens[i - 1].type != ')' && tokens[i - 1].type != 'd')))
      tokens[i].type = '_';
    if (tokens[i].type == '*' &&
        (i == 0 ||
         (i > 0 && tokens[i - 1].type != 'd' && tokens[i - 1].type != ')')))
      tokens[i].type = DEREF;
  }
  error = 0;
  u_int64_t val = eval(0, nr_token - 1);
  if (error == 1)
  {
    *success = false;
    return 0;
  }
  return val;
}
