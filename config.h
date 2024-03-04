#ifndef _CONFIG_H
#define _CONFIG_H

#include "stdint.h"

typedef struct{
  uint64_t  RESX;
  uint64_t  RESY;
  uint64_t  PIXELSIZE;
}Config;

extern Config config;

void initConfig();
#endif//CONFIG_H


