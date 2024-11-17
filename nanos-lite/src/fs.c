#include <fs.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct
{
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum
{
  FD_STDIN,
  FD_STDOUT,
  FD_STDERR,
  FD_FB
};

size_t invalid_read(void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len)
{
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
// 通过添加 __attribute__((used))，你告诉编译器即使在程序中没有显式引用 file_table，也不要将其优化掉
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode)
{
  int num = sizeof(file_table) / sizeof(file_table[0]);
  for (size_t i = 0; i < num; i++)
  {
    if (strcmp(file_table[i].name, pathname) == 0)
    {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("The %s not found", pathname);
}

size_t fs_read(int fd, void *buf, size_t len)
{
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(file_table[0]));
  if (file_table[fd].read != NULL)
    return file_table[fd].read(buf, 0, len);
  else
  {
    len = file_table[fd].open_offset + len <= file_table[fd].size ? len : file_table[fd].size - file_table[fd].open_offset;
    int result = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    file_table[fd].open_offset += result;
    return result;
  }
}

size_t fs_write(int fd, const void *buf, size_t len)
{
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(file_table[0]));
  if (file_table[fd].write != NULL)
    return file_table[fd].write(buf, 0, len);
  else
  {
    len = file_table[fd].open_offset + len <= file_table[fd].size ? len : file_table[fd].size - file_table[fd].open_offset;
    int result = ramdisk_write(buf, file_table[fd].open_offset, len);
    file_table[fd].open_offset += result;
    return result;
  }
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  // whence : 0-SEEK_SET,1-SEEK_CUR,2-SEEK_END
  // lseek()  allows  the  file  offset to be set beyond the end of the file(but this does not change the size of the file)
  // but in sfs,not allowed,the seek_end is not allowed
  // Upon successful completion, lseek() returns the resulting offset
  //     location as measured in bytes from the beginning  of  the  file.
  //     On  error,  the value (off_t) -1 is returned and errno is set to
  //     indicate the error.

  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(file_table[0]));
  if (fd <= 2)
    return 0;
  switch (whence)
  {
  case SEEK_SET:
    assert(offset <= file_table[fd].size && offset >= 0);
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:
    assert(file_table[fd].open_offset + offset <= file_table[fd].size && file_table[fd].size + offset >= 0);
    file_table[fd].open_offset += offset;
    break;
  case SEEK_END:
    assert(file_table[fd].size + offset <= file_table[fd].size && file_table[fd].size + offset >= 0);
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  default:
    break;
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd)
{
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(file_table[0]));
  return 0;
}

char *getFdName(int fd)
{
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(file_table[0]));
  return file_table[fd].name;
}

void init_fs()
{
  // TODO: initialize the size of /dev/fb
}