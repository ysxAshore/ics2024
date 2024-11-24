#include <am.h>
#include <stdlib.h>
#include <unistd.h>
Area heap;

void putch(char ch)
{
    printf("%c", ch);
}

void halt(int code)
{
    exit(code);
}
