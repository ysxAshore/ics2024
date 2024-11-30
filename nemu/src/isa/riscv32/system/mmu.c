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
#include <macro.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define VA_OFFSET(addr) (addr & 0x00000FFF)
#define VA_VPN_0(addr) ((addr >> 12) & 0x000001FF)
#define VA_VPN_1(addr) ((addr >> 21) & 0x000001FF)
#define VA_VPN_2(addr) ((addr >> 30) & 0x000001FF)

#define PTE_V(item) (item & 0x1)
#define PTE_R(item) ((item >> 1) & 0x1)
#define PTE_W(item) ((item >> 2) & 0x1)
#define PTE_X(item) ((item >> 3) & 0x1)
#define PTE_XWRV(item) (item & 0b1111)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type)
{
  word_t satp = cpu.csrs.satp & 0x00000FFFFFFFFFFF;
  vaddr_t first_base = satp << 12; // 一级页表基址

  // access first page use paddr_read
  uint64_t first_content = paddr_read(first_base + VA_VPN_2(vaddr) * 8, 8);
  assert(PTE_XWRV(first_content) == 0b0001); // 一定是4KB的粒度

  // access second page
  vaddr_t second_base = (first_content >> 10) << 12;
  uint64_t second_content = paddr_read(second_base + VA_VPN_1(vaddr) * 8, 8);
  assert(PTE_XWRV(second_content) == 0b0001); // 一定是4KB的粒度

  // access third page
  vaddr_t third_base = (second_content >> 10) << 12;
  uint64_t third_content = paddr_read(third_base + VA_VPN_0(vaddr) * 8, 8);
  assert(PTE_V(third_content) == 0b1); // 一定是有效的
  switch (type)
  {
  case MEM_TYPE_IFETCH: // inst fetch
    assert(PTE_X(third_content) == 0b1);
    break;
  case MEM_TYPE_READ: // load memory
    assert(PTE_R(third_content) == 0b1);
    break;
  case MEM_TYPE_WRITE:
    assert(PTE_R(third_content) == 0b1 && PTE_W(third_content) == 0b1);
    break;
  default:
    panic("Unknown access type\n");
    break;
  }
  // 计算物理地址
  paddr_t ppn = (third_content >> 10) << 12;
  paddr_t paddr = ppn | VA_OFFSET(vaddr);
  // 检查计算的物理地址是否与虚拟地址相等，否则断言失败
  assert(paddr == vaddr);

  return paddr;
}
