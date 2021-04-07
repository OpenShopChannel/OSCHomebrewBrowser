
#ifndef GRRLIB_COMP
#define GRRLIB_COMP

#include <grrlib.h>

void GRRLIB_InitFreetype();
GRRLIB_texImg *GRRLIB_TextToTexture(const char *string, unsigned int fontSize, unsigned int fontColour);

#endif
