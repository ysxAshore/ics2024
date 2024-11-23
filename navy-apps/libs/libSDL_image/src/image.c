#define SDL_malloc malloc
#define SDL_free free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc)
{
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface *IMG_Load(const char *filename)
{
  // open file
  FILE *file = fopen(filename, "rb");

  // get the file size
  fseek(file, 0, SEEK_END);     // set the file end
  long file_size = ftell(file); // get the file size
  fseek(file, 0, SEEK_SET);     // restore

  // malloc a buf which size is file_size
  unsigned char *buf = (char *)malloc(file_size);

  // read the file
  fread((void *)buf, file_size, 1, file);

  // call STBIMD_LoadFromMemory
  SDL_Surface *res = STBIMG_LoadFromMemory(buf, file_size);

  // release the resource
  fclose(file);
  free(buf);

  return res;
}

int IMG_isPNG(SDL_RWops *src)
{
  return 0;
}

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src)
{
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError()
{
  return "Navy does not support IMG_GetError()";
}
