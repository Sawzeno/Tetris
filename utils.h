#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

typedef struct{
  uint8_t* string;
  uint8_t   size;
}UtilBuff;

UtilBuff* inttostr(uint64_t* integer);


#endif /* UTILS_H */
/*


UtilBuff* inttostr(void* integer){

  uint64_t  _integer  = *((uint64_t*)integer);

  uint8_t*  string   =  malloc(sizeof(uint8_t) * 24);
  if(string == NULL){
    perror("maler : utils temp string");
  }
  string[0] = 0 + '0';

  uint8_t   rem    = 0;
  uint8_t   count  = 0;

  do{
    rem           = (uint8_t)(_integer % 10);
    string[count] = rem + '0';
    _integer      = _integer/10;
    count++;
  }while(_integer != 0);

  printf("len %"PRIu8"\n",count);
  UtilBuff*  buffer  = calloc(1 , sizeof(UtilBuff));
  if(buffer == NULL){
    perror("maler : util buffer");
  }

  buffer->size    = count;
  buffer->string  = malloc(sizeof(uint8_t) * buffer->size);
  if(buffer->string ==  NULL){
    perror("maler : util buffer string");
  }

  for(uint8_t i = 0 ; i < count ; i++){
    buffer->string[i] = string[count - i - 1];
  }

  return buffer;
}

*/
