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

#include "sdb.h"

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expression;
  uint64_t val;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].expression = malloc(256);
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp()
{
  if (free_ == NULL)
  {
    printf("Theres is no space for requesting a watch from watch pool!\n");
    assert(0);
  }
  else
  {
    WP *tmp = free_;
    free_ = free_->next;
    return tmp;
  }
}
void free_wp(WP *wp)
{
  wp->next = free_;
  free_ = wp;
}
void displayAllWatch()
{
  WP *p = head;
  while (p != NULL)
  {
    printf("The expression of the %d watch is %s,and the current value is %lu\n", p->NO, p->expression, p->val);
    p = p->next;
  }
  if (p == NULL)
  {
    printf("There is no watch\n");
  }
}
void deleteOneWatch(int N)
{
  WP *p = head, *q = head, *tmp = NULL;
  while (p != NULL)
  {
    if (p->NO == N)
    {
      tmp = p;
      break;
    }
    q = p;
    p = p->next;
  }
  if (p == head && p != NULL)
    head = tmp->next;
  else
    q->next = tmp->next;
  printf("The %d watch has deleted\n", N);
  free_wp(tmp);
}
void createAWatch(char *args)
{
  WP *awp = new_wp();
  bool sign = true;
  uint64_t val = expr(args, &sign);
  if (sign)
  {
    strcpy(awp->expression, args); // 不能直接赋值，因为直接赋值只能保证第一次是对的，其他时变量被回收值不确定
    awp->val = val;
    awp->next = head;
    head = awp;
    printf("The %d watch has created,%s = %lu\n", awp->NO, awp->expression, awp->val);
  }
  else
    printf("The expression %s isn't solvable", args);
}
void checkWatchesStatus()
{
  WP *p = head;
  while (p != NULL)
  {
    bool sign = true;
    uint64_t nowVal = expr(p->expression, &sign);
    if (nowVal != p->val && sign)
    {
      printf("The %d watch watches the expression %s has changed,from %lu to %lu\n", p->NO, p->expression, p->val, nowVal);
      p->val = nowVal;
      nemu_state.state = NEMU_STOP;
    }
    else
      printf("The expression %s isn't solvable", p->expression);

    p = p->next;
  }
}