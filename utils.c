#include<stdio.h>
#include<stdlib.h>

#include"utils.h"

UtilBuff* inttostr(uint64_t* integer){

  uint64_t _int = *integer;

  uint8_t* string  = malloc(sizeof(uint8_t) * 24);
  if(string == NULL){
    perror("maler : temp string");
  }

  uint8_t count = 0;
  uint8_t rem   = 0;

  do{
    rem = (uint8_t)(_int % 10);
    string[count] = rem + '0';
    _int /= 10;
    ++count;
  }while(_int != 0);

  UtilBuff* buffer  = malloc(sizeof(UtilBuff));
  if(buffer ==  NULL){
    perror("maler : UtilBuffer");
  }

  buffer->size      = count;
  buffer->string    = malloc(sizeof(uint8_t) * buffer->size);
  if(buffer->string ==  NULL){
    perror("maler : UtilBuffer string");
  }

  for(int i = 0 ; i < buffer->size ; ++i){
    buffer->string[i] = string[buffer->size - i - 1];
  }

  free(string);
  return buffer;
}
