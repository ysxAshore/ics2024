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

static uintptr_t loader(PCB *pcb, const char *filename)
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
      fs_read(fd, (void *)phdrs[i].p_vaddr, phdrs[i].p_filesz);
      // void *memset(void *s, int c, size_t n)
      memset((void *)(phdrs[i].p_vaddr + phdrs[i].p_filesz), 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
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
