#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>

#include  "global.h"
#include  "error.h"

typedef struct ScreenRow ScreenRow;
typedef struct Buffer   Buffer;
typedef struct Screen   Screen;
typedef struct WindowManager  WindowManager;
typedef enum   ScreenOrient   ScreenOrient;

Buffer* createBuffer  (uint64_t size);
Buffer* createScreenBuffer  (uint64_t size);
void    appendBuffer  (Buffer*  buffer  , char* string  , uint64_t size);
Screen* createScreen  (uint64_t cols    , uint64_t rows , char* type);
void    addScreen     (Screen*  parent  , Screen* child , ScreenOrient orient);
void    render        (Screen*  screen);

enum ScreenOrient{
  HORZ,
  VERT
};

struct Buffer{
  char*       string;
  uint64_t    size;
};

struct ScreenRow{
  char*       string;
  ScreenRow*  currentScreenRow;
  ScreenRow*  nextScreenRow;
  uint64_t    size;
};

struct Screen{
  Buffer*     buffer;
  ScreenRow*  headrow;
  uint64_t    numrows;
  uint64_t    numcols;
};

struct WindowManager{
  Buffer* outbuffer;
  Screen** screens;
};

Buffer* createBuffer(uint64_t size){

  Buffer* buffer  = malloc(sizeof(Buffer));
  ISNULL(buffer)

  buffer->size  = size ;
  buffer->string = calloc(buffer->size, sizeof(char));
  ISNULL(buffer->string)

  return buffer;
}

Buffer* createScreenBuffer(uint64_t size){

  Buffer* buffer  = malloc(sizeof(Buffer));
  ISNULL(buffer)

  buffer->size  = size * global.PIXELSIZE;
  buffer->string = calloc(buffer->size, sizeof(char));
  ISNULL(buffer->string)

  return buffer;
}


ScreenRow* createScreenRow(char* string , uint64_t size){
  ScreenRow* screenrow  = malloc(sizeof(ScreenRow));
  ISNULL(screenrow);

  screenrow->string           = string;
  screenrow->size             = size;
  screenrow->currentScreenRow = NULL;
  screenrow->nextScreenRow    = NULL;

  return screenrow;
}

Screen* createScreen(uint64_t cols , uint64_t rows , char* type){

  Screen* screen  = malloc(sizeof(Screen));
  ISNULL(screen)

  screen->numcols = cols;
  screen->numrows = rows;
  screen->buffer  = createScreenBuffer(screen->numcols * screen->numrows);

  for(size_t i = 0 ; i < screen->numcols * screen->numrows ; ++i){
    strcpy(&(screen->buffer->string[i * global.PIXELSIZE]), type);
  }

  char*   rowstart  = screen->buffer->string;
  uint64_t  stride  = screen->numcols * global.PIXELSIZE;

  screen->headrow   = createScreenRow(rowstart, stride);

  ScreenRow* head   = screen->headrow;
  ScreenRow* next   = NULL;

  size_t  i = 0;
  while(i < screen->numrows){
    rowstart  = &(screen->buffer->string[i * stride]);
    next  = createScreenRow(rowstart, stride);
    head->currentScreenRow  = next;
    head  = next;
    next  = NULL;
    ++i;
  }

  return screen;
}


void addScreen(Screen* parent, Screen* child, ScreenOrient orient) {
  ScreenRow* parentCurrentRow = parent->headrow;
  ScreenRow* parentNextRow    = parentCurrentRow;

  ScreenRow* childCurrentRow  = child->headrow;

  switch (orient) {

    case VERT: {
      while (parentCurrentRow != NULL && childCurrentRow != NULL) {
        ScreenRow* parentNextRow = parentCurrentRow;
        while (parentNextRow->nextScreenRow != NULL) {
          parentNextRow = parentNextRow->nextScreenRow;
        }

        parentNextRow->nextScreenRow = childCurrentRow;

        parentCurrentRow = parentCurrentRow->currentScreenRow;
        childCurrentRow  = childCurrentRow->currentScreenRow;
      }

      break;
    }


    case HORZ: {
      while (parentNextRow->currentScreenRow != NULL) {
        parentNextRow = parentNextRow->currentScreenRow;
      }
      parentNextRow->currentScreenRow = child->headrow;
      break;
    }
  }
}



void appendBuffer(Buffer* buffer , char* string , uint64_t size){

  char* new  = realloc(buffer->string, buffer->size + size);
  ISNULL(new)

  memcpy(&(new[buffer->size]), string, size);

  buffer->string  =   new;
  buffer->size    +=  size;
}

void render(Screen* screen){
  ScreenRow endline;
  endline.string            = "\n";
  endline.size              = 1;
  endline.currentScreenRow  = NULL;
  endline.nextScreenRow     = NULL;

  Buffer* outbuffer = createBuffer(0);
  ISNULL(outbuffer)

  ScreenRow* _currentScreenRow = screen->headrow;
  ScreenRow* _nextScreenRow    = _currentScreenRow;

  while (_currentScreenRow != NULL) {
    while (_nextScreenRow != NULL) {
      appendBuffer(outbuffer, _nextScreenRow->string, _nextScreenRow->size);
      _nextScreenRow = _nextScreenRow->nextScreenRow;
    }
    appendBuffer(outbuffer, endline.string, endline.size);
    _currentScreenRow = _currentScreenRow->currentScreenRow;
    _nextScreenRow = _currentScreenRow;
  }

  write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
  write(STDOUT_FILENO, outbuffer->string, outbuffer->size);

  free(outbuffer->string);
  free(outbuffer);
}



int main(){

  initGlobals();

  Screen* screenA  =  createScreen(global.RESX    , global.RESY , "█");
  Screen* screenB  =  createScreen(1  , global.RESY , "🮘");
  Screen* screenC  =  createScreen(10  , global.RESY , "");

  addScreen(screenA, screenB , VERT);
  addScreen(screenB, screenC , VERT);
  render(screenA);

  return 0;
}
