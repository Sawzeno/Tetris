#include  "defines.h"
#include  "core/logger.h"

#define ROWS  32
#define COLS  64
#define LINEBREAK_CHAR '\n'
#define LINEBREAK_SIZE 1
#define MAX_SPLITS_ALLOWED 5
#define DEFAULT_FRAMEBUFFER_ROW_SIZE 512
#define DEFAULT_FRAMEBUFFER_COL_SIZE 256

typedef struct Window Window;
typedef struct Buffer Buffer;
typedef struct Screen Screen;
typedef struct ScreenRow ScreenRow;
typedef struct ScreenRowBank ScreenRowBank;
typedef struct FrameBuffer FrameBuffer;
typedef struct FrameBufferRow FrameBufferRow;

Window*       createWindow      (u64 rows, u64 cols, char type);
ScreenRow*    getScreenRows     (Window* window, u64 num);
void          render            (Window* window);
void          update            (Window* window);
FrameBuffer*  createFrameBuffer (u64 rows, u64 cols, char bg);

struct  Window{
  char  type;
  u64   numrows;
  u64   numcols;
  Buffer* buffer;
  Screen* topscreen;
  ScreenRowBank* screenrowbank;
};

struct  Buffer{
  u64   size;
  char* data;
};

struct  Screen{
  u64   numrows;
  u64   numcols;
  ScreenRow*  rows;
  FrameBuffer*  framebuffer;
  Screen* vertchild;
  Screen* horzchild;
};

struct  ScreenRow{
  u64   len;
  u64   clippingValue;
  char* windowbufferline;
  ScreenRow*  leftrow;
  ScreenRow*  rightrow;
  FrameBufferRow* framebufferrow;
};

struct  ScreenRowBank{
  u64 cap;
  u64 size;
  ScreenRow* screenrows;
};

struct FrameBuffer{
  char  bg;
  u64   numrows;
  u64   numcols;
  Buffer* buffer;
  FrameBufferRow* rows;
};

struct  FrameBufferRow{
  u64 len;
  u64 xoffset;
  char* data;
};

Window* createWindow(u64 rows, u64 cols, char type){

  Window* window  = calloc(1, sizeof(Window));
  MEMERR(window);

  window->numrows = rows;
  window->numcols = cols;
  window->type    = type;

  UDEBUG("-----------------INIT WINDOW BUFFER---------------");
  Buffer* winbuff = calloc(1, sizeof(Buffer));
  MEMERR(winbuff);
  winbuff->size = window->numrows * (window->numcols + LINEBREAK_SIZE);
  UINFO("SIZE of Buffer is %d", winbuff->size);
  char* buffer  = calloc(winbuff->size, sizeof(char));
  MEMERR(buffer);
  winbuff->data = buffer;
  window->buffer  = winbuff;

  UDEBUG("-----------------INIT SCREEN ROW BANK-------------");
  ScreenRowBank*  rowbank = calloc(1, sizeof(ScreenRowBank));
  MEMERR(rowbank);
  rowbank->cap  = window->numrows * MAX_SPLITS_ALLOWED;
  rowbank->size = 0;
  rowbank->screenrows = calloc(rowbank->cap, sizeof(ScreenRow));
  MEMERR(rowbank);
  window->screenrowbank = rowbank;

  UDEBUG("-----------------CREATING TOP SCREEN--------------");

  Screen* screen  = calloc(1, sizeof(Screen));
  MEMERR(screen);
  screen->numrows = window->numrows;
  screen->numcols = window->numcols;

  ScreenRow*  nullrows  = getScreenRows(window, screen->numrows);
  ScreenRow*  charrows  = getScreenRows(window, screen->numrows);

  UDEBUG("-----------------CREATING NULL BUFFER-------------");
  FrameBuffer* nullbuffer = createFrameBuffer(screen->numrows,LINEBREAK_SIZE, LINEBREAK_CHAR);
  UDEBUG("-----------------CREATING CHAR BUFFER-------------");
  FrameBuffer* charbuffer = createFrameBuffer(DEFAULT_FRAMEBUFFER_ROW_SIZE,DEFAULT_FRAMEBUFFER_COL_SIZE, window->type);

  UDEBUG("-------------------ASSIGNING BUFFERS--------------");
  u64 buffindex = 0;
  ScreenRow* temp = NULL;
  for(u64 i = 0; i < screen->numrows; ++i){
    if(i == 0){
      charrows[i].windowbufferline  = window->buffer->data + buffindex;
      charrows[i].len               = screen->numcols;
      charrows[i].clippingValue     = 0;
      charrows[i].framebufferrow    = &(charbuffer->rows[i]);
      buffindex                    += charrows[i].len;

      nullrows[i].windowbufferline  = window->buffer->data + buffindex;
      nullrows[i].len               = LINEBREAK_SIZE;
      nullrows[i].clippingValue     = 0;
      nullrows[i].framebufferrow   = &(nullbuffer->rows[i]);
      buffindex                    += nullrows[i].len;
      temp  = &nullrows[i];
      temp->leftrow  = &charrows[i];
    }else{
      charrows[i].windowbufferline  = window->buffer->data + buffindex;
      charrows[i].len               = screen->numcols;
      charrows[i].clippingValue     = 0;
      charrows[i].framebufferrow    = &(charbuffer->rows[i]);
      buffindex                    += charrows[i].len;

      nullrows[i].windowbufferline  = window->buffer->data + buffindex;
      nullrows[i].len               = LINEBREAK_SIZE;
      nullrows[i].clippingValue     = 0;
      nullrows[i].framebufferrow    = &(nullbuffer->rows[i]);
      buffindex                    += nullrows[i].len;
      temp->rightrow = &nullrows[i];
      temp  = temp->rightrow;
      temp->leftrow  = &charrows[i];
    }
  }

  screen->framebuffer = charbuffer;
  screen->rows        = charrows;
  screen->horzchild   = NULL;
  screen->vertchild   = NULL;
  window->topscreen   = screen;
  return window;
}

Screen* ScreenSplitVert(Window* window ,Screen* parent, u64 percent, FrameBuffer* framebuffer){
  UDEBUG("-------------------SPLIT SCREEN VERT--------------");
  Screen* child  = calloc(1, sizeof(Screen));
  MEMERR(child);
  u64 pcols = parent->numcols;
  u64 ccols = (pcols * percent) / 100;
  parent->numcols -= ccols;
  child->numcols  = ccols;
  child->numrows  = parent->numrows;
  child->framebuffer = framebuffer;

  ScreenRow*  childrows = getScreenRows(window, parent->numrows);
  ScreenRow*  parentrows= parent->rows;
  UDEBUG("-------------------ASSIGNING BUFFERS--------------");
  for(u16 i = 0; i < parent->numrows ; ++i){
    parentrows[i].len               = parent->numcols;
    childrows [i].len               = child->numcols;
    childrows [i].clippingValue     = 0;
    childrows [i].windowbufferline  = parentrows[i].windowbufferline + parentrows[i].len;
    childrows [i].framebufferrow    = &(framebuffer->rows[i]);
    if(parentrows[i].leftrow == NULL){
      parentrows[i].leftrow         = &(childrows[i]);
    }else{
      ScreenRow* temp = parentrows[i].leftrow;
      parentrows[i].leftrow         = &(childrows[i]);
      childrows[i].leftrow          = temp;
    }
  }
  child->rows = childrows;
  if(parent->vertchild == NULL){
    parent->vertchild = child;
  }else{
    Screen* temp  = parent->vertchild;
    parent->vertchild  = child;
    child->vertchild  = temp;
  }
  return child;
}

Screen* ScreenSplitHorz(Window* window, Screen* parent, u64 percent, FrameBuffer* framebuffer){
  UDEBUG("-------------------SPLIT SCREEN HORZ--------------");
  Screen* child  = calloc(1, sizeof(Screen));
  MEMERR(child);
  u64 prows = parent->numrows;
  u64 crows = (prows * percent) / 100;
  parent->numrows -=  crows;
  child->numrows  = crows;
  child->numcols  = parent->numcols;
  child->framebuffer = framebuffer; 
  UDEBUG("-------------------ASSIGNING BUFFERS--------------");
  ScreenRow* parentRows  = parent->rows;
  ScreenRow* childRows   = parent->rows + parent->numrows;

  for(u16 i = 0; i < child->numrows; ++i){
    childRows[i].framebufferrow  = &(child->framebuffer->rows[i]);
  }
  child->rows = childRows;
  Screen* temp  = parent->horzchild;
  if(temp == NULL){
    parent->horzchild = child;
  }else{
    parent->horzchild = child;
    child->horzchild  = temp;
  }
  child->vertchild  = NULL;
  return child;
}

FrameBuffer*  createFrameBuffer (u64 rows, u64 cols, char bg){
  FrameBuffer* framebuffer  = calloc(1, sizeof(FrameBuffer));
  MEMERR(framebuffer);
  if(rows == 0 || cols == 0){
    exit(EXIT_FAILURE);
  }
  framebuffer->numrows  = rows;
  framebuffer->numcols  = cols;
  framebuffer->bg       = bg;

  UDEBUG("-----------------INIT FRAME BUFFER---------------");
  Buffer* framebuff  = calloc(1, sizeof(Buffer));
  MEMERR(framebuff);
  framebuff->size  = framebuffer->numrows * framebuffer->numcols;
  UINFO("SIZE of Buffer is %d", framebuff->size);
  char* buffer  = calloc(framebuff->size, sizeof(char));
  MEMERR(buffer);
  framebuff->data = buffer;

  framebuffer->buffer = framebuff;

  for(u64 i = 0; i < framebuff->size; ++i){
    framebuff->data[i]  = framebuffer->bg;
  }

  UDEBUG("-----------------INIT FRAME BUFFER ROWS-----------");
  FrameBufferRow* fbrows  = calloc(framebuffer->numrows, sizeof(FrameBufferRow));  
  MEMERR(fbrows);
  u64 buffindex = 0;
  for(u64 i = 0; i < framebuffer->numrows; ++i){
    fbrows[i].data        = buffer + buffindex;
    fbrows[i].len         = framebuffer->numcols;
    fbrows[i].xoffset     = 0;
    buffindex += framebuffer->numcols;
  }
  framebuffer->rows = fbrows;

  return framebuffer;
}

ScreenRow*  getScreenRows (Window* window, u64 num){
  ScreenRowBank*  rb  = window->screenrowbank;
  if(rb->size + num >= rb->cap){
    UERROR("MAX NUM OF SCREEN ROWS EXCEEDED !!");
    exit(EXIT_FAILURE);
  }else{
    ScreenRow* rowarr = rb->screenrows + rb->size;
    rb->size += num;
    UINFO("ALLOCATED %d ScreenRows", num);
    return rowarr;
  }
}


int main(void){
  Window* win = createWindow(ROWS, COLS, '*');
  Screen* screenA  = win->topscreen;
  FrameBuffer*  framebufferB  = createFrameBuffer(64, 64, '#');
  FrameBuffer*  framebufferC  = createFrameBuffer(64, 64, '+');
  FrameBuffer*  framebufferD  = createFrameBuffer(64, 64, '-');
  FrameBuffer*  framebufferE  = createFrameBuffer(64, 64, '0');

  Screen* screenB  = ScreenSplitVert(win, screenA, 50, framebufferB);
  Screen* screenC  = ScreenSplitVert(win, screenB, 50, framebufferC); 
  Screen* screenD  = ScreenSplitHorz(win, screenC, 50, framebufferD);
  Screen* screenE  = ScreenSplitHorz(win, screenA, 50, framebufferE);
  update(win);
  render(win);
  return 0;
}

void  update(Window* window){
  UDEBUG("-----------------------UPDATE---------------------");
  ScreenRow* right = window->screenrowbank->screenrows;
  ScreenRow* left = NULL;

  while(right != NULL){
    left  = right->leftrow;
    while(left != NULL){
      memcpy(left->windowbufferline, left->framebufferrow->data, left->len);
      left  = left->leftrow;
    }
    memcpy(right->windowbufferline, right->framebufferrow->data, right->len);
    right = right->rightrow;
  }
}

void  render(Window* window){
  UDEBUG("-----------------------RENDER---------------------");
  write(STDOUT_FILENO, window->buffer->data, window->buffer->size);
}
