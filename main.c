#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>

#include  "global.h"
#include  "error.h"

typedef struct Pixel    Pixel;
typedef struct Buffer   Buffer;
typedef struct Scene    Scene;
typedef struct Screen   Screen;
typedef struct Window   Window;
typedef struct ScreenRow      ScreenRow;
typedef enum   ScreenOrient   ScreenOrient;

void        appendBuffer        (Buffer*  buffer  , char* string  , uint64_t size);
Buffer*     createScreenBuffer  (uint64_t size);
void        mapScreenBuffer     (Screen* screen);
Screen*     createScreen        (Screen* parent , ScreenOrient orient , uint64_t percentage);
void        addScreen           (Screen* parent, Screen* child, ScreenOrient orient);
Scene*      createScene         (uint64_t limitX , uint64_t limitY , char* type);
ScreenRow*  createScreenRow     (char*  string , uint64_t size);
void        updateScreen        (Screen*  screen);
void        render              (Screen*  screen);

enum ScreenOrient{
  HORZ,
  VERT,
  HEAD
};

struct Pixel{
  char*     type;
  uint8_t   R;
  uint8_t   G;
  uint8_t   B;
  uint8_t   Z;
};

struct Buffer{
  char*       string;
  uint64_t    size;
};

struct Scene{
  Pixel*      pixelBuffer;
  char*       type;
  uint64_t    limitX;
  uint64_t    limitY;
};

struct ScreenRow{
  char*       string;
  ScreenRow*  currentScreenRow;
  ScreenRow*  nextScreenRow;
  uint64_t    size;
};

struct Screen{
  Buffer*     buffer;
  Scene*      scene;
  Screen*     nextVERT;
  Screen*     nextHORZ;
  ScreenRow*  headrow;
  uint64_t    numrows;
  uint64_t    numcols;
};

struct Window{
  Screen* top;
};


void appendBuffer(Buffer* buffer , char* string , uint64_t size){

  char* new  = realloc(buffer->string, buffer->size + size);
  ISNULL(new)

  memcpy(&(new[buffer->size]), string, size);

  buffer->string  =   new;
  buffer->size    +=  size;
}

Buffer* createScreenBuffer(uint64_t size){

  Buffer* buffer  = malloc(sizeof(Buffer));
  ISNULL(buffer)

  buffer->size  = size * global.PIXELSIZE;

  buffer->string = calloc(buffer->size, sizeof(char));
  ISNULL(buffer->string)
  memset(buffer->string, 0, buffer->size);


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

Screen* createScreen(Screen* parent , ScreenOrient orient , uint64_t percentage){

  if(parent == NULL){
    orient  = HEAD;
  }

  Screen* child  = malloc(sizeof(Screen));
  ISNULL(child)

  child->nextHORZ = NULL;
  child->nextVERT = NULL;

  uint64_t displacement;
  switch (orient) {  
    case  VERT  : {
      displacement      = (parent->numcols * percentage)/100;
      child->numrows    = parent->numrows;
      child->numcols    = displacement;  
      parent->numcols  -= displacement;  

      parent->nextVERT  = child;
      parent->nextHORZ  = NULL;  
      break;
    }
    case  HORZ  : {
      displacement      = (parent->numrows * percentage)/100 ; 
      child->numrows    = displacement;
      parent->numrows  -= displacement;
      child->numcols    = parent->numcols;

      parent->nextHORZ  = child;
      parent->nextVERT  = NULL;
      break;
    }
    case  HEAD  : {
      child->numrows  = global.RESY;
      child->numcols  = global.RESX;
      break;
    }
  }

  mapScreenBuffer(child);
  mapScreenBuffer(parent);

  return child;
}

void mapScreenBuffer(Screen* screen) {
  if (screen == NULL) {
    return;
  }

  if(screen->buffer != NULL){
    free(screen->buffer);
  }


  screen->buffer = createScreenBuffer(screen->numcols * screen->numrows);

  ScreenRow* existingRows = screen->headrow;

  char*       rowstart  = screen->buffer->string;
  uint64_t    stride    = screen->numcols * global.PIXELSIZE;
  ScreenRow*  head;
  ScreenRow*  next;
  if(existingRows == NULL){

    screen->headrow = createScreenRow(rowstart, stride);
    head = screen->headrow;
    next = NULL;

    size_t i = 0;
    while(i < screen->numrows){
      i++;
      rowstart = &(screen->buffer->string[i * stride]);
      next = createScreenRow(rowstart, stride);
      head->currentScreenRow = next;
      head = next;
      next = NULL;
    }

  }else{
    head = existingRows;
    next = NULL;

    size_t i = 0;
    while (i < screen->numrows) {
      rowstart = &(screen->buffer->string[i * stride]);
      head->string  = rowstart;
      head->size    = stride;
      head = next;
      ++i;
    }
  }
}

void addScreen(Screen* parent, Screen* child, ScreenOrient orient) {

  if(parent == NULL){
    return;
  }
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

    case HEAD:  {
      break;
    }
  }
}

Scene* createScene(uint64_t limitX , uint64_t limitY , char* type){
  Scene* scene = malloc(sizeof(Scene));
  ISNULL(scene)

  scene->limitX = limitX;
  scene->limitY = limitY;

  scene->type = malloc(strlen(type));
  ISNULL(scene->type)
  strcpy(scene->type, type);

  scene->pixelBuffer  = malloc(sizeof(Pixel) * scene->limitX * scene->limitY);
  ISNULL(scene->pixelBuffer)

  return scene;
}

void updateScreen(Screen* screen) {
  Scene* scene = screen->scene;

  Pixel* pixels = scene->pixelBuffer;

  uint64_t index  = 0;
  uint64_t pixelBufferIndex = 0;
  for(int i = 0 ; i < screen->numrows ; ++i){
    for(int j = 0 ; j < screen->numcols ; ++j){
      pixelBufferIndex  = j + (i * scene->limitX);

      sprintf(&(screen->buffer->string[index * global.PIXELSIZE]), "\x1b[32;2;%d;%d;%dm%s",pixels[pixelBufferIndex].R,pixels[pixelBufferIndex].G,
              pixels[pixelBufferIndex].B,scene->type);
      index++;
    }
  }

}

void render(Screen* screen){
  ScreenRow endline;
  endline.string            = "\n";
  endline.size              = 1;
  endline.currentScreenRow  = NULL;
  endline.nextScreenRow     = NULL;

  Buffer* outbuffer = createScreenBuffer(0);

  ScreenRow* _currentScreenRow = screen->headrow;
  ScreenRow* _nextScreenRow    = _currentScreenRow;


  while (_currentScreenRow != NULL) {
    _nextScreenRow = _currentScreenRow;
    while (_nextScreenRow != NULL) {
      appendBuffer(outbuffer, _nextScreenRow->string, _nextScreenRow->size);
      _nextScreenRow = _nextScreenRow->nextScreenRow;
    }
    appendBuffer(outbuffer, endline.string, endline.size);
    _currentScreenRow = _currentScreenRow->currentScreenRow;
  }

  write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
  write(STDOUT_FILENO, outbuffer->string, outbuffer->size);

  free(outbuffer->string);
  free(outbuffer);
}

struct Window{

  Screen* top;


};

Window* createWindow(){
  Window* window = malloc(sizeof(Window));
  ISNULL(window)

  window->top  = createScreen(NULL, HEAD, 100);
  
  return window;
}

int main(){

  initGlobals();

  Screen* screenA = createWindow()->top;

  Scene*  sceneA    = createScene(global.RESX     , global.RESY   , "█"); 
  Scene*  sceneB    = createScene(global.RESX     , global.RESY   , "🮘");

  screenA->scene  = sceneA;

  Screen* screenB   = createScreen(screenA, VERT , 20);
  screenB->scene  = sceneB;

  //Update();
  render(screenA);
  return 0;
}

