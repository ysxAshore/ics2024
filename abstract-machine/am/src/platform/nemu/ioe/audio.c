#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)

void __am_audio_init()
{
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg)
{
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

// output 到地址用来初始化
void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl)
{
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat)
{
  stat->count = inl(AUDIO_COUNT_ADDR);
}

static int sbuf_point = 0;

// 写数据到SBUF
void __am_audio_play(AM_AUDIO_PLAY_T *ctl)
{
  uint8_t *data = ctl->buf.start;
  int data_len = ctl->buf.end - ctl->buf.start;
  int bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  uint8_t *sbuf = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;

  for (int i = 0; i < data_len; i++)
  {
    sbuf[sbuf_point] = data[i];
    sbuf_point = (sbuf_point + 1) % bufsize;
  }
  outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + data_len);
}
