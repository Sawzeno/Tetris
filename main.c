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
typedef struct ScreenRowList  ScreenRowList;
typedef enum   ScreenOrient   ScreenOrient;

void        appendBuffer        (Buffer*  buffer  , char* string  , uint64_t size);
Buffer*     createBuffer        (uint64_t size);
void        mapScreenBuffer     (Screen* screen);
Screen*     splitScreen        (Screen* parent , ScreenOrient orient , uint64_t percentage);
void        addScreen           (Screen* parent, Screen* child, ScreenOrient orient);
Scene*      createScene         (uint64_t limitX , uint64_t limitY , char* type);
ScreenRow*  createScreenRow     (char*  string , uint64_t size);
void        updateScreen        (Screen*  screen);
void        render              (Window* window);

enum ScreenOrient{
  HORZ,
  VERT,
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
  ScreenRow*  nextRow;
  ScreenRow*  nextScreenRow;
  uint64_t    size;
};

struct ScreenRowList{
  ScreenRow*  head;
  uint64_t    size;
};

struct Screen{
  Scene*      scene;
  Buffer*     buffer;
  ScreenRowList*  screenrows;
  Screen*     nextVERT;
  Screen*     nextHORZ;
  uint64_t    numrows;
  uint64_t    numcols;
};

struct Window{
  Screen*     top;
  Buffer*     buffer;
  uint64_t    size;
};

void appendBuffer(Buffer* buffer , char* string , uint64_t size){

  char* new  = realloc(buffer->string, buffer->size + size);
  ISNULL(new)

  memcpy(&(new[buffer->size]), string, size);

  buffer->string  =   new;
  buffer->size    +=  size;
}

Buffer* createBuffer(uint64_t size){

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
  screenrow->nextRow          = NULL;
  screenrow->nextScreenRow    = NULL;

  return screenrow;
}

ScreenRowList* createScreenRows(uint64_t numrows){

  ScreenRowList* list = malloc(sizeof(ScreenRowList));
  ISNULL(list)

  list->size  = numrows;

  ScreenRow* temphead = NULL;

  for(int i = 0 ; i < list->size ; ++i){
    if(list->head == NULL){
      list->head  =  createScreenRow(NULL, 0);
      temphead  = list->head;
    }else{
      temphead->nextRow = createScreenRow(NULL, 0);
      temphead  = temphead->nextRow;
    }
  }

  return list;
}

Screen* splitScreen(Screen* parent , ScreenOrient orient , uint64_t percentage){

  if(parent == NULL){
    return NULL;
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
  }

  mapScreenBuffer(child);
  mapScreenBuffer(parent);

  return child;
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


void render(Window* window){
  Window* activeWindow  =  window;

  write(STDOUT_FILENO , window->buffer , window->size);
}

Window* createWindow(){
  Window* window  = malloc(sizeof(Window));
  ISNULL(window)

  window->size    = global.RESX * global.RESY;
  window->buffer  = createWindowBuffer(window->size); 

  Screen* screen  = malloc(sizeof(Screen));
  ISNULL(screen)

  screen->numcols       = global.RESX;
  screen->numrows       = global.RESY;
  screen->nextHORZ      = NULL;
  screen->nextVERT      = NULL;
  screen->scene         = NULL;

  screen->buffer        = getScreenBuffer(window , screen->numrows * screen->numcols);
  mapScreenBuffer(screen); 
  return window;
}

void mapScreenBuffer(Screen* screen){
  
  screen->buffer  = createBuffer(screen->numcols * screen->numrows);

   
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

