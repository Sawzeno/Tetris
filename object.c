#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

typedef struct Pixel Pixel;
typedef struct Object Object;  
typedef struct Screen Screen;

struct Pixel{

  char* type;
  uint64_t  X;
  uint64_t  Y;
  uint64_t  size;
};

struct Object{
  uint64_t pos_x;
  uint64_t pos_y;
  uint64_t size_x;
  uint64_t size_y;
  uint64_t z_index;
  Pixel* buffer;
};

Object* createObject(uint64_t pos_x , uint64_t pos_y , uint64_t size_x , uint64_t size_y){

  Object* object  = malloc(sizeof(Object));
  if(object  == NULL){
    perror("maler : object");
  }

  object->pos_x   = pos_x;
  object->pos_y   = pos_y;
  object->size_x  = size_x;
  object->size_x  = size_y;
  object->z_index = 0;

  object->buffer  = malloc(sizeof(Pixel) * size_x * size_y);
  if(object->buffer ==  NULL){
    perror("maler : Oject Pixels");
  }

  for(int i = 0  , j = 0; j <  size_y ; ++i){
    Pixel* pixel  = &object->buffer[i];
    if(i == size_x){
      ++j;
    }
    pixel->Y  = j;
    pixel->X  = i; 
  }
  return object;
}

void drawObject(Screen* screen , Object* object){

  
}
