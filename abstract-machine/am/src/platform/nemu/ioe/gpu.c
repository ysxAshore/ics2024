#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init()
{
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg)
{
  *cfg = (AM_GPU_CONFIG_T){
      .present = true, .has_accel = false, .width = 0, .height = 0, .vmemsz = 0};
  uint32_t size = inl(VGACTL_ADDR);
  cfg->height = size & 0xffff;
  cfg->width = size >> 16;
  cfg->vmemsz = cfg->height * cfg->width * sizeof(uint32_t);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl)
{
  // 以(x,y)绘制[w,h]的矩形
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0)) // 高或者宽为0且不需要立刻同步时,直接返回
    return;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pix = (uint32_t *)ctl->pixels;
  uint32_t size = inl(VGACTL_ADDR);
  int screenWidth = size >> 16;
  int screenHeight = size & 0xffff;
  // (3,4)时,但是pix应该是起始(0,0)开始
  for (int i = y; i < y + h && i < screenHeight; ++i)
  {
    for (int j = x; j < x + w && j < screenWidth; ++j)
      fb[screenWidth * i + j] = pix[w * (i - y) + (j - x)];
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
