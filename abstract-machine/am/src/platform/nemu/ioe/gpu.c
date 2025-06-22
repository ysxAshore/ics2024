#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init()
{
  //  int i;
  //  int data = inl(VGACTL_ADDR);
  //  int w = data >> 16;
  //  int h = data & 0xffff;
  //  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  //  for (i = 0; i < w * h; i++)
  //    fb[i] = i;
  //  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg)
{
  uint32_t data = inl(VGACTL_ADDR);
  uint32_t height = data & 0xffff;
  uint32_t width = data >> 16;
  *cfg = (AM_GPU_CONFIG_T){
      .present = true, .has_accel = false, .width = width, .height = height, .vmemsz = 0};
}

// 向屏幕(x,y)绘制w*h的矩形图像
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl)
{
  uint32_t data = inl(VGACTL_ADDR);
  uint32_t screen_w = data >> 16;
  uint32_t screen_h = data & 0xffff;

  uint32_t begin = 0;
  uint32_t current = 0;
  for (int i = 0; i < ctl->h && ctl->y + i < screen_h; i++)
  {
    begin = (ctl->y + i) * screen_w + ctl->x;
    for (int j = 0; j < ctl->w && ctl->x + j < screen_w; j++)
    {
      current = i * ctl->w + j;
      uint32_t data = *((uint32_t *)ctl->pixels + current);
      outl(FB_ADDR + 4 * (begin + j), data);
    }
  }
  if (ctl->sync)
  {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status)
{
  status->ready = true;
}
