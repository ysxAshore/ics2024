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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc)
{
  bool sign = true;
  if (!difftest_check_reg("npc", pc, ref_r->pc, cpu.pc))
    return false;
  if (!difftest_check_reg("mstatus", pc, ref_r->csrs.mstatus, cpu.csrs.mstatus))
    return false;
  if (!difftest_check_reg("mepc", pc, ref_r->csrs.mepc, cpu.csrs.mepc))
    return false;
  if (!difftest_check_reg("mtvec", pc, ref_r->csrs.mtvec, cpu.csrs.mtvec))
    return false;
  if (!difftest_check_reg("mcause", pc, ref_r->csrs.mcause, cpu.csrs.mcause))
    return false;
  for (int i = 0; i < 32; ++i)
  {
    sign = difftest_check_reg(reg_name(i), pc, ref_r->gpr[i], cpu.gpr[i]);
    if (!sign)
      break;
  }
  return sign;
}

void isa_difftest_attach()
{
}
