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

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum
{
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static int sbuf_point = 0;
void audio_callback(void *userdata, Uint8 *stream, int len)
{
  // 初始化音频数据
  SDL_memset(stream, 0, len); // 例如，初始化为静音
  // 获取缓冲区长度,取缓冲区已用长度和len中的最小值
  uint32_t used_cnt = audio_base[reg_count];
  uint32_t sbuf_size = audio_base[reg_sbuf_size];
  len = len > used_cnt ? used_cnt : len;

  // 使用SDLMixAudio播放,需要注意大于缓冲区长度的情况
  if (sbuf_point + len > sbuf_size)
  {
    // 先取到尾
    SDL_MixAudio(stream, sbuf + sbuf_point, sbuf_size - sbuf_point, SDL_MIX_MAXVOLUME);
    // 然后取头
    SDL_MixAudio(stream + sbuf_size - sbuf_point,
                 sbuf,
                 len - sbuf_size + sbuf_point,
                 SDL_MIX_MAXVOLUME);
  }
  else
    SDL_MixAudio(stream, sbuf + sbuf_point, len, SDL_MIX_MAXVOLUME);

  // 从缓冲区中取出数据，更新可用长度和缓冲区索引
  sbuf_point = (sbuf_point + len) % sbuf_size;
  audio_base[reg_count] -= len;
}

static void init_SDL()
{
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;
  s.userdata = NULL;
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.freq = audio_base[reg_freq];
  s.callback = audio_callback;

  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write)
{
  if (audio_base[reg_init] == 1)
  {
    init_SDL();
    audio_base[reg_init] = 0;
  }
}

void init_audio()
{
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
}
