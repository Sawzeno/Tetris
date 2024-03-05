#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "stdint.h"

typedef struct{
  uint64_t  RESX;
  uint64_t  RESY;
  uint64_t  PIXELSIZE;
}Global;

extern Global global;

void initGlobals();
#endif//CONFIG_H


