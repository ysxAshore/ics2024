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

#define IRQ_TIMER 0x8000000000000007 // for riscv64

word_t isa_raise_intr(word_t NO, vaddr_t epc)
{
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.csrs.mcause = 0xb; // in m-mode,enviroment call always is 0xb
  cpu.csrs.mepc = epc;
  cpu.gpr[17] = NO;
  word_t mie = (cpu.csrs.mstatus >> 3) & 0x1;
  cpu.csrs.mstatus = (cpu.csrs.mstatus & ~(1 << 3)) | (mie << 7); // 置MIE为0,并将MIE保存到PMIE
  IFDEF(CONFIG_ETRACE, printf("\nThere is a No.%lx exception at " FMT_WORD "\n", NO, epc));
  return cpu.csrs.mtvec;
}

word_t isa_query_intr()
{
  if (cpu.INTR && ((cpu.csrs.mstatus >> 3) & 0x1))
  {
    cpu.INTR = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}