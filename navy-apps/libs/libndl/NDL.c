#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static uint32_t init_time = 0;
static int evtdev = -1; // NWM_APP的/dev/events　ID
static int fbdev = -1;  // NWM_APP的/dev/fb　ID
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

// 以毫秒为单位返回系统时间
uint32_t NDL_GetTicks()
{
  struct timeval tv;
  int result = gettimeofday(&tv, NULL);
  assert(result == 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000 - init_time;
}

int NDL_PollEvent(char *buf, int len)
{
  int fd = open("/dev/events", 0);
  int result = read(fd, buf, len);
  close(fd);
  return result;
}

void NDL_OpenCanvas(int *w, int *h)
{
  if (getenv("NWM_APP"))
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1)
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    printf("111");
    close(fbctl);
  }
  if (*w == 0 && *h == 0) // 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
  {
    *w = screen_w;
    *h = screen_h;
  }
  canvas_w = *w;
  canvas_h = *h;

  // canvas center
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;

  // not over the screen
  assert(canvas_x + canvas_w <= screen_w);
  assert(canvas_y + canvas_h <= screen_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h)
{
  // x y w h都是以像素为单位
  int curScreen_x = x + canvas_x;
  int curScreen_y = y + canvas_y;

  int offset = curScreen_y * screen_w + curScreen_x;

  int fd = open("/dev/fb", 0);

  for (int i = 0; i < h && i + y < canvas_h; i++) // 不要超出画布高度
  {
    lseek(fd, offset * 4, SEEK_SET);          // 　是字节为单位
    w = w <= canvas_w - x ? w : canvas_w - x; // 画布的宽度是否够画
    write(fd, pixels + i * w, w * 4);
    offset = offset + screen_w; // 更新offset
  }
  close(fd);
}

// 打开音频功能,初始化声卡设备
void NDL_OpenAudio(int freq, int channels, int samples)
{
  int fd = open("/dev/sbctl", 0);
  int buf[] = {freq, channels, samples};
  write(fd, (void *)buf, 12);
  close(fd);
}

// 关闭音频功能
void NDL_CloseAudio()
{
}

// 播放缓冲区buf中长度为len的音频数据,返回成功播放的音频数据字节数
int NDL_PlayAudio(void *buf, int len)
{
  int fd = open("/dev/sb", 0);
  int result = write(fd, buf, len);
  close(fd);
  return result;
}

// 返回当前声卡设备流缓冲区中的空闲字节数
int NDL_QueryAudio()
{
  int curSize = 0;
  int fd = open("/dev/sbctl", 0);
  read(fd, (void *)&curSize, 4);
  close(fd);
  return curSize;
}

int strFindChar(char *s, char c)
{
  int len = strlen(s);
  for (int i = 0; i < len; ++i)
    if (s[i] == c)
      return i;
  return -1;
}

int NDL_Init(uint32_t flags)
{
  if (getenv("NWM_APP"))
  {
    evtdev = 3;
    fbdev = 5;
  }
  // 由于gettimeofday获取的时间实际上是AM_TIMER_UPTIME寄存器里的时间，此时间代表的是系统开机后的时间
  // 所以需要在NDL_init中先初始化一下第一次获取的时间，之后用每次获取的时间减去开始时间来获取启动NDL库之后的当前时间
  struct timeval tv;
  int result = gettimeofday(&tv, NULL);
  assert(result == 0);
  init_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  char buf[64], res[64];
  read(4, buf, 64);
  int first_newline = strFindChar(buf, '\n');
  strncpy(res, buf + 6, first_newline - 6);
  screen_w = atoi(res);
  strncpy(res, buf + first_newline + 8, strlen(buf) - first_newline - 8 + 1);
  screen_h = atoi(res);
  printf("%s decode-%d:%d\n", buf, screen_w, screen_h);
  return 0;
}

void NDL_Quit()
{
  evtdev = -1;
  fbdev = -1;
  screen_h = 0;
  screen_w = 0;
  canvas_h = 0;
  canvas_w = 0;
  canvas_x = 0;
  canvas_y = 0;
}
