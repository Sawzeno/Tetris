#include <stdio.h>
#include <stdlib.h>

#include "error.h"

inline void printError(const char* message , const char* func , size_t line){

  fprintf(stderr, "ERROR[%zu]  : %s at %s", line , message , func);
  exit(EXIT_FAILURE);
}
