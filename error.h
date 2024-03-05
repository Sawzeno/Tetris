#ifndef _ERROR_H
#define _ERROR_H

#include<stdio.h>

#define ISNULL(X) \
if(X == NULL){    \
printError("MEM ALLOC FAILED", __FUNCTION__ , __LINE__);  \
}                 \

void printError(const char* message , const char* func , size_t line);

#endif//_ERROR_H
//
