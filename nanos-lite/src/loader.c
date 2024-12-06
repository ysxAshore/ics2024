#include <proc.h>
#include <elf.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_386
#elif defined(__ISA_MIPS32__)
#define EXPECT_TYPE EM_MIPS_RS3_LE
#elif defined(__riscv)
#define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_LOONGARCH32R__)
#define EXPECT_TYPE EM_LOONGARCH
#else
#error Unsupported ISA
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);

uintptr_t loader(PCB *pcb, const char *filename)
{
  int fd = fs_open(filename, 0, 0);
  // read ELF Header and examine
  Elf_Ehdr ehdr;
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  if (ehdr.e_ident[0] != 0x7F ||
      ehdr.e_ident[1] != 'E' ||
      ehdr.e_ident[2] != 'L' ||
      ehdr.e_ident[3] != 'F')
    panic("The ramdisk content is not a elf file\n");
  if (ehdr.e_machine != EXPECT_TYPE)
    panic("The ramdisk content is not match the current machine\n");

  fs_lseek(fd, ehdr.e_phoff, 0);
  // read ELF Program Segement Header
  Elf_Phdr phdrs[ehdr.e_phnum];
  // ramdisk_read(phdrs, ehdr.e_phoff, sizeof(Elf_Phdr) * ehdr.e_phnum);
  fs_read(fd, phdrs, sizeof(Elf_Phdr) * ehdr.e_phnum);
  for (int i = 0; i < ehdr.e_phnum; i++)
  {
    if (phdrs[i].p_type == PT_LOAD)
    {
      fs_lseek(fd, phdrs[i].p_offset, 0);

      void *va = (void *)phdrs[i].p_vaddr;
      uintptr_t offset = (uintptr_t)va % PGSIZE;
      void *filesize = va + phdrs[i].p_filesz;
      size_t N = (phdrs[i].p_memsz + offset) / PGSIZE + ((phdrs[i].p_memsz + offset) % PGSIZE ? 1 : 0);
      printf("There is virtual address %x,offset is %x,filesize pointer is %x,needs %x page\n", va, offset, filesize, N);
      for (size_t j = 0; j < N; j++)
      {
        void *pa = new_page(1) - PGSIZE;
        memset(pa, 0, offset);

        printf("Calling map funtion to map %x to %x\n", va - offset, pa);
        map(&pcb->as, va - offset, pa, 3); // in am_native,prot & MMAP_READ0x1 prot & MMAP_WRITE0x2

        uintptr_t readsize = PGSIZE - offset;
        if (va + readsize > filesize)
        {
          readsize = filesize - va > 0 ? filesize - va : 0;
          int len = fs_read(fd, pa + offset, readsize);
          memset(pa + offset + len, 0, PGSIZE - offset - len);
          printf("Will read %x size from file to %x,and set %x numbers of 0 to %x\n", readsize, pa + offset, PGSIZE - offset - len, pa + offset + len);
        }
        else
        {
          fs_read(fd, pa + offset, readsize);
          printf("Will read %x size from file to %x\n", readsize, pa + offset);
        }
        va = va - offset + PGSIZE;
        offset = 0;
      }
      if (pcb->max_brk < phdrs[i].p_vaddr + phdrs[i].p_memsz)
        pcb->max_brk = phdrs[i].p_vaddr + phdrs[i].p_memsz;
      // fs_read(fd, (void *)phdrs[i].p_vaddr, phdrs[i].p_filesz);
      // // void *memset(void *s, int c, size_t n)
      // memset((void *)(phdrs[i].p_vaddr + phdrs[i].p_filesz), 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
    }
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename)
{

  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
