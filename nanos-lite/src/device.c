#include <common.h>
#include <sys/time.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

typedef unsigned long uint64_t;

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len)
{
  int result = 0;
  for (size_t i = 0; i < len; i++)
  {
    putch(((char *)buf)[i]);
    ++result;
  }
  return result;
}

size_t events_read(void *buf, size_t offset, size_t len)
{
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE)
  {
    ((char *)buf)[0] = '\0';
    return 0;
  }
  else
  {
    int result = 0;
    if (ev.keydown)
      result = snprintf((char *)buf, len, "kd %s\n", keyname[ev.keycode]);
    else
      result = snprintf((char *)buf, len, "ku %s\n", keyname[ev.keycode]);
    return result;
  }
}

size_t dispinfo_read(void *buf, size_t offset, size_t len)
{
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  return snprintf((char *)buf, len, "WIDTH:%d\nHEIGHT:%d", gpu_config.width, gpu_config.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len)
{
  AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);

  int y = offset / 4 / t.width; // 行主序
  int x = offset / 4 - y * t.width;

  io_write(AM_GPU_FBDRAW, x, y, (uint32_t *)buf, len / 4, 1, true);

  return len;
}

// 读出可用大小
size_t sbctl_read(void *buf, size_t offset, size_t len)
{
  int hasUsed_size = io_read(AM_AUDIO_STATUS).count;
  int entire_size = io_read(AM_AUDIO_CONFIG).bufsize;
  *((uint32_t *)buf) = entire_size - hasUsed_size;
  return len;
}

size_t sbctl_write(const void *buf, size_t offset, size_t len)
{
  int freq = ((int *)buf)[0];
  int channel = ((int *)buf)[1];
  int samples = ((int *)buf)[2];
  io_write(AM_AUDIO_CTRL, freq, channel, samples);
  return len;
}

size_t sb_write(const void *buf, size_t offset, size_t len)
{
  uint32_t curSize; // 声明可用大小
  do
  {
    sbctl_read(&curSize, 0, 4); // 读取可用大小
  } while (curSize < len); // 直到可用大小够写当前的len长的buf数据
  Area sbuf;
  sbuf.start = (void *)buf;
  sbuf.end = sbuf.start + len;
  io_write(AM_AUDIO_PLAY, sbuf);
  return len;
}

int time_gettimeofday(struct timeval *tv, struct timezone *tz)
{
  // tz可以忽略,设置为NULL
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us / 1000000;
  tv->tv_usec = us % 1000000;
  return 0;
}

void init_device()
{
  Log("Initializing devices...");
  ioe_init();
}
