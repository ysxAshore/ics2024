#include <am.h>
#include <NDL.h>
#include <string.h>

#define NAME(key) \
    [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)};

#define SIZE sizeof(keyname) / sizeof(keyname[0])

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd)
{
    char buf[10];
    if (NDL_PollEvent(buf, 10))
    {
        buf[strlen(buf) - 1] = '\0';
        kbd->keydown = buf[0] == 'k' && buf[1] == 'd';
        for (int i = 0; i < SIZE; i++)
        {
            if (strcmp(keyname[i], buf + 3) == 0)
            {
                kbd->keycode = i;
                return;
            }
        }
    }
    else
    {
        kbd->keycode = AM_KEY_NONE;
    }
}
