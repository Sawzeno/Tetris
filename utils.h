#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

typedef struct{
  uint8_t* string;
  uint8_t   size;
}UtilBuff;

UtilBuff* inttostr(uint64_t* integer);


#endif /* UTILS_H */
