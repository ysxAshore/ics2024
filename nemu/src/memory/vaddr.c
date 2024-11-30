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
#include <isa-def.h>
#include <memory/paddr.h>

int isa_mmu_check(vaddr_t addr, int len, int type)
{
  return ((cpu.csrs.satp >> 60) & 0b1111) == 0b1000 ? MMU_TRANSLATE : MMU_DIRECT;
}

word_t vaddr_ifetch(vaddr_t addr, int len)
{
  int type = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  if (type == MMU_TRANSLATE)
    return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_IFETCH), len);
  else if (type == MMU_DIRECT)
    return paddr_read(addr, len);
  else
    return MMU_FAIL;
}

word_t vaddr_read(vaddr_t addr, int len)
{
  int type = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  if (type == MMU_TRANSLATE)
    return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_READ), len);
  else if (type == MMU_DIRECT)
    return paddr_read(addr, len);
  else
    return MMU_FAIL;
}

void vaddr_write(vaddr_t addr, int len, word_t data)
{
  int type = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  if (type == MMU_TRANSLATE)
    paddr_write(isa_mmu_translate(addr, len, MEM_TYPE_WRITE), len, data);
  else if (type == MMU_DIRECT)
    paddr_write(addr, len, data);
}
