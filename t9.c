#include  "utils.h"
#include <string.h>
#include <unistd.h>
#define   ROWS 16
#define   COLS 32
#define   LINEBRK_SIZE 1
#define   LINBRK_CHAR '\n'
#define   MAX_SPLITS_ALLOWED 5

typedef struct Window Window;
typedef struct Screen Screen;
typedef struct ScreenRow ScreenRow;
typedef struct ScreenRowBank  ScreenRowBank;
typedef struct FrameBuffer FrameBuffer;
typedef struct FrameBufferRow FrameBufferRow;

int           testFrame(FrameBuffer* framebuffer);
Window*       createWindow(u16 rows, u16 cols, char type);
Screen*       createScreen(Window* window, FrameBuffer* framebuffer);
Screen*       splitScreen(Window* window, Screen* parent, FrameBuffer* frambuffer, u16 percent);
ScreenRow*    getScreenRows(u16 num, Window* window);
ScreenRow*    createRow   (char* ptr, u16 len);
FrameBuffer*  createFrameBuffer(u16 resy, u16 resx, char bg);

struct Screen{
  FrameBuffer* framebuffer;
  u16   numRows;
  u16   numCols;
  ScreenRow*  rows;
};

struct  ScreenRow{
  u16   len;
  char* data;
  ScreenRow*  left;
  ScreenRow*  right;
};

struct  ScreenRowBank{
  u16   cap;
  u16   len;
  ScreenRow*  rows;
};

struct  FrameBufferRow{
  u16   len;
  char* data;
};

struct FrameBuffer{
  char  bg;
  u16   resx;
  u16   resy;
  char* buffer;
  FrameBufferRow*  rows;
};

struct Window{
  char    type;
  u16     numrows;
  u16     numcols;
  u16     buffsize;
  char*   buffer;
  Screen* head;
  ScreenRowBank*  rowbank;
};

Window* createWindow(u16 rows, u16 cols, char type){
  Window* window  = calloc(1, sizeof(Window));
  MEMERR(window);
  window->type      = type;
  window->numrows   = rows;
  window->numcols   = cols;
  window->buffsize  = window->numrows * (window->numcols + LINEBRK_SIZE);
  window->buffer    = calloc(window->buffsize, sizeof(char));
  MEMERR(window->buffer);
  ScreenRowBank* rowbank = window->rowbank   = calloc(1, sizeof(ScreenRowBank));
  MEMERR(rowbank);
  rowbank->cap      = MAX_SPLITS_ALLOWED * window->numrows;
  rowbank->rows     = calloc(rowbank->cap, sizeof(ScreenRow));
  MEMERR(rowbank->rows);
  rowbank->len      = 0;
  window->head      = createScreen(window, createFrameBuffer(rows, cols, type));

  return window;
}

ScreenRow*  getScreenRows(u16 num, Window* window){
  ScreenRowBank*  rowbank = window->rowbank;
  if(rowbank->len + num >= rowbank->cap){
    return NULL;
  }
  ScreenRow* ret  = rowbank->rows + rowbank->len;
  rowbank->len += num;
  return ret;
}

Screen* createScreen(Window* window, FrameBuffer* framebuffer){
  Screen* screen = calloc(1, sizeof(Screen));
  MEMERR(screen);
  screen->framebuffer = framebuffer;
  screen->numRows = window->numrows;
  screen->numCols = window->numcols;
  ScreenRow*  nullcols    = getScreenRows(screen->numRows, window); 
  ScreenRow*  charcols    = getScreenRows(screen->numRows, window);  
  screen->rows  = calloc(1, sizeof(ScreenRow));
  MEMERR(screen->rows);
  ScreenRow*  temp        = screen->rows;

  u16 buffindex = 0;

  for(u16 i = 0; i < screen->numRows; ++i){
    charcols[i].data  = window->buffer + buffindex;
    charcols[i].len   = screen->numCols;
    buffindex += charcols[i].len;

    nullcols[i].data  = window->buffer + buffindex;
    nullcols[i].len   = LINEBRK_SIZE;
    *(nullcols[i].data) = LINBRK_CHAR;
    buffindex +=  nullcols[i].len;

    temp->left  = &charcols[i];
    temp->right = &nullcols[i];
    temp  = temp->right;
  }
  return screen;
}

Screen* splitScreen(Window* window, Screen* parent, FrameBuffer* framebuffer, u16 percent){
  Screen* screen  = calloc(1, sizeof(Screen));
  u16 pcols = parent->numCols;
  u16 ccols = (pcols * percent)/100;
  parent->numCols = pcols - ccols;
  MEMERR(screen);
  screen->framebuffer = framebuffer;
  screen->numRows     = parent->numRows;
  screen->numCols     = ccols; 

  ScreenRow* crowarr  = getScreenRows(screen->numRows, window);

  screen->rows  = crowarr;
  ScreenRow* prowarr = parent->rows->left;

  for(u16 i = 0; i < screen->numRows ; ++i){
    if (!prowarr) {
      perror("Parent row is NULL");
      free(screen);
      return NULL;
    }
    crowarr[i].data = prowarr[i].data + parent->numCols;
    crowarr[i].len  = screen->numCols;

    prowarr[i].len = parent->numCols;
    prowarr[i].left= &crowarr[i];
  }

  UDEBUG("SPLIT");
  UDEBUG("parent->numrows : %d", parent->numRows);
  UDEBUG("%d", window->rowbank->len);
  UDEBUG("%d", window->rowbank->cap);
  ScreenRow* temp = parent->rows;
  return screen;
}

FrameBuffer*  createFrameBuffer(u16 resy, u16 resx, char bg){
  FrameBuffer* framebuffer  = calloc(1, sizeof(FrameBuffer));
  MEMERR(framebuffer);

  framebuffer->resy = resy;
  framebuffer->resx = resx;

  framebuffer->buffer = calloc(framebuffer->resy * framebuffer->resx, sizeof(char));
  MEMERR(framebuffer->buffer);
  framebuffer->bg = bg;

  for(u16 i = 0; i < framebuffer->resy * framebuffer->resx; ++i){
    framebuffer->buffer[i]  = framebuffer->bg;
  }

  framebuffer->rows = calloc(framebuffer->resy, sizeof(FrameBufferRow));
  MEMERR(framebuffer->rows);

  u16 buffindex = 0;
  for(u16 i = 0; i < framebuffer->resy; ++i){
    framebuffer->rows[i].data =  framebuffer->buffer + buffindex;
    framebuffer->rows[i].len  = framebuffer->resx;
    buffindex +=  framebuffer->resx;
  }
  return framebuffer;
}

void render(Window* window){
  UDEBUG("RENDER");
  write(STDOUT_FILENO, window->buffer, window->buffsize * sizeof(char));
}

void update(Screen* screen) {
  FrameBuffer* fb = screen->framebuffer;
  FrameBufferRow* fbhead = fb->rows;
  ScreenRow* schead = screen->rows;

  for (u16 i = 0; i < screen->numRows; ++i) {
    memcpy(schead->left->data, fbhead[i].data, schead->left->len * sizeof(char));
    schead = schead->right;
  }
}

int main(void){

  Window* window = createWindow(ROWS , COLS, '*');
  FrameBuffer*  fb  = createFrameBuffer(ROWS, COLS, '#');
  Screen* screenB= splitScreen(window,window->head, fb, 50);
  update(window->head);
  render(window);
  return 0;
}

int testFrame(FrameBuffer* framebuffer){
  UDEBUG("TEST FRAME");
  u16 numchars = 0;
  numchars =  write(STDOUT_FILENO, framebuffer->rows->data, framebuffer->resx * framebuffer->resy);
  UDEBUG("renderer charachters %d",numchars);
  return numchars;
}
