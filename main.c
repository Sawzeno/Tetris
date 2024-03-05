#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>

#include  "config.h"

typedef struct Pixel    Pixel;
typedef struct PixelRow PixelRow;
typedef struct Buffer   Buffer;
typedef struct Screen   Screen;

struct PixelRow{
  char*       string;
  PixelRow*   next;
  uint64_t    size;
};

struct Buffer{
  char*     string;
  uint64_t  size;
};

struct Screen{
  Buffer*     buffer;
  PixelRow**  rows;
  uint64_t    numrows;
  uint64_t    numcols;
};

Buffer* createBuffer(uint64_t size){

  Buffer* buffer  = malloc(sizeof(Buffer));
  if(buffer ==  NULL){
    perror("maler : buffer global");
    exit(EXIT_FAILURE);
  }

  buffer->size  = size * config.PIXELSIZE;
  buffer->string = calloc(buffer->size, sizeof(char));
  if(buffer->string == NULL){
    perror("maler : buffer string");
  }

  return buffer;
}

Screen* createScreen(uint64_t cols , uint64_t rows , char* type){

  Screen* screen  = malloc(sizeof(Screen));
  if(screen == NULL){
    perror("maler : screen");
  }

  screen->numcols = cols;
  screen->numrows = rows;
  screen->buffer  = createBuffer(screen->numcols * screen->numrows);

  for(size_t i = 0 ; i < screen->numcols * screen->numrows ; ++i){
    strcpy(&(screen->buffer->string[i * config.PIXELSIZE]), type);
  }

  screen->rows    = calloc(screen->numrows , sizeof(PixelRow*));
  if(screen->rows == NULL){
    perror("maler : screen rows");
  }

  for(size_t i = 0 ; i < screen->numrows ; ++i){
    PixelRow* row;

    row = malloc(sizeof(PixelRow));
    if(row  ==  NULL){
      perror("screen rows row");
    }

    row->string = &(screen->buffer->string[i * screen->numcols * config.PIXELSIZE]);
    row->size = screen->numcols * config.PIXELSIZE;
    row->next = NULL;
    screen->rows[i] = row;
  }

  return screen;
}

void appendBuffer(Buffer* buffer , char* string , uint64_t size){

  char* new  = realloc(buffer->string, buffer->size + size);
  if(new == NULL){
    perror("realloc error : appendBuffer");
  }

  memcpy(&(new[buffer->size]), string, size);

  buffer->string  =   new;
  buffer->size    +=  size;
}

void render(Screen* screen){

  PixelRow endline;
  endline.string  = "\n";
  endline.size    = 1;
  endline.next    = NULL;

  Buffer* outbuffer = malloc(sizeof(Buffer));
  if(outbuffer == NULL){
   perror("maler  : outbuffer"); 
  }
  outbuffer->size = 0;
  outbuffer->string = malloc(sizeof(char) * outbuffer->size);
  if(outbuffer  == NULL){
    perror("maler : outbuffer string");
  }
  
  for(size_t i = 0 ; i < screen->numrows ; ++i){
    PixelRow* row = screen->rows[i];
    while(row != NULL){
      appendBuffer(outbuffer, row->string, row->size);
      row = row->next;
    }
    appendBuffer(outbuffer, endline.string, endline.size);
  }

  write(STDOUT_FILENO , "\x1b[2J\x1b[H", 7);
  write(STDOUT_FILENO , outbuffer->string , outbuffer->size);
}

void addScreen(Screen* parent , Screen* child){

  PixelRow* row;
  for(size_t i = 0 ; i < child->numrows ; ++i){
    row = parent->rows[i];
    while(row->next != NULL){
      row = row->next;
    }
    row->next = child->rows[i];
  }
}

int main(){

  initConfig();

  Screen* screenA  =  createScreen(config.RESX , config.RESY , "█");
  Screen* screenB  =  createScreen(1 , config.RESY , "⏽");
  Screen* screenC  =  createScreen(config.RESX/2 , config.RESY , "▓");

  addScreen(screenB, screenC);
  addScreen(screenA, screenB);
  //input();
  //update();
  render(screenA);

  return 0;
}
