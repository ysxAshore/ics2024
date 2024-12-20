#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    _KEYS(keyname)};

#define SIZE (sizeof(keyname) / sizeof(keyname[0]))
static uint8_t isKeyDown[SIZE];

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
    for (int i = 0; i < SIZE; ++i)
    {
      if (keyname[i] && strcmp(keyname[i], buf + 3) == 0)
      {
        keySym.sym = i;
        break;
      }
    }
    keyEvent.type = event->type;
    keyEvent.keysym = keySym;
    event->key = keyEvent;
    if (event->type == SDL_KEYDOWN)
      isKeyDown[keySym.sym] = (uint8_t)1;
    else
      isKeyDown[keySym.sym] = (uint8_t)0;
    CallbackHelper(2); // 先响应按键，不然会按动卡卡的
    return 1;
  }
  CallbackHelper(2);
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
  for (int i = 0; i < SIZE; ++i)
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
  if (event->type == SDL_KEYDOWN)
    isKeyDown[keySym.sym] = (uint8_t)1;
  else
    isKeyDown[keySym.sym] = (uint8_t)0;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask)
{
  return 0;
}

// 这个没实现
// 301f62a pc->PAL_ProcessEvent->input.c->PAL_UpdateKeyboardState->SDL_GetKeyboardState-># define SDL_GetKeyboardState        SDL_GetKeyState
uint8_t *SDL_GetKeyState(int *numkeys)
{
  int num = 0;
  for (int i = 0; i < SIZE; ++i)
    if (isKeyDown[i] == (uint8_t)1)
      ++num;
  if (numkeys != NULL) // numkeys调用时可以指定为NULL
    *numkeys = num;
  return isKeyDown;
}
