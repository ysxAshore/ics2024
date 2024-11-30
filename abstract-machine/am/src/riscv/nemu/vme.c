#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir)
{
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp()
{
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *))
{
  // 定义申请一个物理页和释放一个物理页的函数
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  // kernel address space
  kas.ptr = pgalloc_f(PGSIZE); // 申请一个PGSIZE大小的物理页,kas的ptr指向页表基地址

  int i;
  for (i = 0; i < LENGTH(segments); i++)
  {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE)
    {
      map(&kas, va, va, 0); // 建立1:1映射关系填写页目录表和页表，通过 1:1 映射机制，确保内核在启用分页后仍可直接访问物理内存。这样既保留了对硬件资源的高效访问，又为后续的虚拟内存管理提供了灵活基础
    }
  }

  set_satp(kas.ptr); // 将kas的页表基地址写入 SATP 寄存器
  vme_enable = 1;    // 启用VME

  return true;
}

void protect(AddrSpace *as)
{
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as)
{
}

void __am_get_cur_as(Context *c)
{
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c)
{
  if (vme_enable && c->pdir != NULL)
  {
    set_satp(c->pdir);
  }
}

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define OFFSET(addr) (addr & 0x00000FFF)
#define VA_VPN_0(addr) ((addr >> 12) & 0x000001FF)
#define VA_VPN_1(addr) ((addr >> 21) & 0x000001FF)
#define VA_VPN_2(addr) ((addr >> 30) & 0x000001FF)

void map(AddrSpace *as, void *va, void *pa, int prot)
{
  // ignore prot
  // as->ptr 获得页表基址　三级页表
  // vaddr 9-9-9-12
  // paddr26-9-9-12
  uintptr_t va_t = (uintptr_t)va;
  uintptr_t pa_t = (uintptr_t)pa;
  assert(OFFSET(va_t) == 0 && OFFSET(pa_t) == 0);
  assert(BITS(va_t, 63, 38) == 0 || BITS(va_t, 63, 38) == -1); // vaddr的63~39必须和38一致

  uint64_t first_content = ((uint64_t *)as->ptr)[VA_VPN_2(va_t)];
  if (first_content == 0)
  {
    void *second_ptr = pgalloc_usr(4 * PGSIZE); // 第二级页表基址
    void *third_ptr = pgalloc_usr(4 * PGSIZE);  // 第三级页表基址
    uintptr_t ppn1 = ((uintptr_t)second_ptr >> 12) << 10;
    ((uint64_t *)as->ptr)[VA_VPN_2(va_t)] = ppn1 | 0b00000001; // x w r = 000
    uintptr_t ppn2 = ((uintptr_t)third_ptr >> 12) << 10;
    ((uint64_t *)second_ptr)[VA_VPN_1(va_t)] = ppn2 | 0b00000001; // x w r = 000
    uintptr_t ppn3 = ((uintptr_t)pa >> 12) << 10;
    ((uint64_t *)third_ptr)[VA_VPN_0(va_t)] = ppn3 | 0b00001111;
  }
  else // 一级页表表项不为空
  {
    assert((first_content & 0b1111) == 0b0001);
    void *second_ptr = (void *)((first_content >> 10) << 12);
    uint64_t second_content = ((uint64_t *)second_ptr)[VA_VPN_1(va_t)];
    if (second_content == 0)
    {
      void *third_ptr = pgalloc_usr(4 * PGSIZE); // 第三级页表基址
      uintptr_t ppn2 = ((uintptr_t)third_ptr >> 12) << 10;
      ((uint64_t *)second_ptr)[VA_VPN_1(va_t)] = ppn2 | 0b00000001; // x w r v = 0001
      uintptr_t ppn3 = ((uintptr_t)pa >> 12) << 10;
      ((uint64_t *)third_ptr)[VA_VPN_0(va_t)] = ppn3 | 0b00001111; // x w r v = 0001
    }
    else // 二级页表表项不为空
    {
      assert((second_content & 0b1111) == 0b0001);
      void *third_ptr = (void *)((second_content >> 10) << 12);
      uintptr_t ppn3 = ((uintptr_t)pa >> 12) << 10;
      ((uint64_t *)third_ptr)[VA_VPN_0(va_t)] = ppn3 | 0b00001111;
    }
  }
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry)
{
  // 因为kcontext还会修改c->GPR2,所以这里为了避免对寄存器的修改，单独设置ucontext内容
  Context *cp = (Context *)(kstack.end - sizeof(Context)); // 按照图示
  cp->mepc = (uintptr_t)entry - 4;                         // 指定入口点,NEMU会加４
  return cp;
}
