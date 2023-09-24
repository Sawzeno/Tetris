
typedef struct Buffer{
  char *b;
  int len;

}Buffer;

void appendBuffer(Buffer *buff, char *s, int len) {

  char *new = realloc(buff->b, buff->len + len);
  if (new == NULL) return;
  memcpy(&new[buff->len], s, len);
  buff->b = new;
  buff->len += len;
}

void bufferFree(Buffer *buff) {
  free(buff->b);
}

typedef struct Screen{
  int rows;
  int cols;
  int index;
  char type;
  char* buffer;
}Screen;

void initScreen(Screen *screen , char type , int rows , int cols){
  screen->rows = rows;
  screen->cols = cols;
  screen->type = type;
  screen->buffer = NULL;
}

char* pixel(Screen* s , int x , int y){
  char* temp;
  temp = &(s->buffer[x * (s->cols) + y]);
  return temp;
}

