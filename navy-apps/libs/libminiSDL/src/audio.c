#include <NDL.h>
#include <SDL.h>
#include <stdlib.h>

static uint32_t gap;
static int freq;
static int channels;
static int samples;
void (*callback)(void *userdata, uint8_t *stream, int len);
static uint8_t *stream;

void CallbackHelper(int flags)
{ // 0 is run,1 is stop,2 is continue
  static uint32_t callTime = 0;
  static int st = 0; // 1 running ; 0 no working
  static int hasCalled = 0;
  if (hasCalled == 1)
    return;
  if (flags == 1)
    st = 1;
  if (flags == 0)
    st = 0;
  if (st == 0)
    return;
  if (SDL_GetTicks() - callTime < gap)
    return;
  hasCalled = 1;
  callTime = SDL_GetTicks();

  // 取完一次的sample样本
  callback(NULL, stream, samples);
  hasCalled = 0;

  // 分次绘制
  int hasPlayed = 0;
  int avaliable = 0;
  do
  {
    avaliable = NDL_QueryAudio();
    int playCount = avaliable > samples ? samples : avaliable;
    NDL_PlayAudio(stream + hasPlayed, playCount);
    hasPlayed += playCount;
  } while (hasPlayed < samples);
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
  freq = desired->freq;
  gap = 1000 / freq;
  channels = desired->channels;
  samples = desired->samples;
  NDL_OpenAudio(freq, channels, samples);
  callback = desired->callback;

  stream = (uint8_t *)malloc(samples);
  CallbackHelper(1);
  return 0;
}

void SDL_CloseAudio()
{
  CallbackHelper(0);
}

void SDL_PauseAudio(int pause_on)
{
  if (pause_on != 0)
    CallbackHelper(0);
  else
    CallbackHelper(1);
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume)
{
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len)
{
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf)
{
}

void SDL_LockAudio()
{
}

void SDL_UnlockAudio()
{
}
