#include <NDL.h>
#include <SDL.h>
#include <stdlib.h>
#include <assert.h>

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
  if (!dst || !src || len < 0 || volume < 0 || volume > SDL_MIX_MAXVOLUME)
    return;
  for (size_t i = 0; i < len; ++i)
  {
    int mixed_sample = dst[i] + src[i] * volume / SDL_MIX_MAXVOLUME;
    if (mixed_sample > 255)
      mixed_sample = 255;
    else if (mixed_sample < 0)
      mixed_sample = 0;

    dst[i] = (uint8_t)mixed_sample;
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len)
{
  FILE *fp = fopen(file, "rb");

  // 读取RIFF chunk
  fseek(fp, 0, SEEK_SET);
  uint32_t riff_id, riff_type;
  fread(&riff_id, 4, 1, fp);
  assert(riff_id == 0x46464952); // big endian
  fseek(fp, 8, SEEK_SET);
  fread(&riff_type, 4, 1, fp);
  assert(riff_type == 0x45564157); // big endian

  // 读取Format chunk
  uint32_t format_id;
  fread(&format_id, 4, 1, fp);
  assert(format_id == 0x20746D66); // big endian
  fseek(fp, 0x16, SEEK_SET);
  fread(&spec->channels, 2, 1, fp);
  fread(&spec->freq, 4, 1, fp);
  fseek(fp, 0x22, SEEK_SET);
  fread(&spec->samples, 2, 1, fp);

  // 读取Data chunk
  uint32_t data_id;
  fread(&data_id, 4, 1, fp);
  assert(data_id == 0x61746164); // big endian
  fread(audio_len, 4, 1, fp);
  *audio_buf = (uint8_t *)malloc(*audio_len);
  fread(*audio_buf, *audio_len, 1, fp);

  return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf)
{
  free(audio_buf);
}

void SDL_LockAudio()
{
}

void SDL_UnlockAudio()
{
}
