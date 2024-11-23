#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    _KEYS(keyname)};

int SDL_PushEvent(SDL_Event *ev)
{
  return 0;
}

// 类似于SDL_WaitEvent,不同的是, 如果当前没有任何事件, 就会立即返回
int SDL_PollEvent(SDL_Event *event)
{
  char buf[10];
  if (NDL_PollEvent(buf, sizeof(buf)))
  {
    buf[strlen(buf) - 1] = '\0';
    event->type = ((buf[0] == 'k' && buf[1] == 'd') && SDL_KEYDOWN) || ((buf[0] == 'k' && buf[1] == 'u') && SDL_KEYUP);
    SDL_KeyboardEvent keyEvent;
    SDL_keysym keySym;
    // 因为navy-native的键位号和main.cpp中的不一样,所以不能这么做
    // strncpy(intNum, buf + 3, findChar(buf, ':') - 3);
    // keySym.sym = atoi(intNum);
    for (int i = 0; i < 83; ++i)
    {
      if (strcmp(keyname[i], buf + 3) == 0)
      {
        keySym.sym = i;
        break;
      }
    }
    keyEvent.type = event->type;
    keyEvent.keysym = keySym;
    event->key = keyEvent;
    return 1;
  }
  return 0;
}

// int findChar(char *buf, char c)
// {
//   int len = strlen(buf);
//   for (size_t i = 0; i < len; i++)
//     if (buf[i] == c)
//       return i;

//   return -1;
// }
int SDL_WaitEvent(SDL_Event *event)
{
  char buf[10];
  while (1)
  {
    if (NDL_PollEvent(buf, sizeof(buf)))
      break;
  }
  buf[strlen(buf) - 1] = '\0';
  event->type = ((buf[0] == 'k' && buf[1] == 'd') && SDL_KEYDOWN) || ((buf[0] == 'k' && buf[1] == 'u') && SDL_KEYUP);
  SDL_KeyboardEvent keyEvent;
  SDL_keysym keySym;
  // 因为navy-native的键位号和main.cpp中的不一样,所以不能这么做
  // strncpy(intNum, buf + 3, findChar(buf, ':') - 3);
  // keySym.sym = atoi(intNum);
  for (int i = 0; i < 83; ++i)
  {
    if (strcmp(keyname[i], buf + 3) == 0)
    {
      keySym.sym = i;
      break;
    }
  }
  keyEvent.type = event->type;
  keyEvent.keysym = keySym;
  event->key = keyEvent;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask)
{
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys)
{
  return NULL;
}
