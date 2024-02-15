#include "platform.h"

// Portable system includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Non-portable system includes
#if PLATFORM == LINUX
#include <iconv.h>
#include <termios.h>
#include <unistd.h>

#elif PLATFORM == WINDOWS

#endif

// Project includes
#include "matrix.h"
#include "rawMode.h"
#include "window.h"

typedef struct Object {
    char type;
    int x;
    int y;
} Object;

void initObject(Object* object, char type, int x, int y)
{
    object->type = type;
    object->x = x;
    object->y = y;
}

void bufferFree(Buffer* buff);
void refreshScreen();
void move(Object* object, int x, int y);
void spawnTet(Screen* s, matrix* m);
// void update(Screen* s);
char* pixel(Screen* s, int x, int y);

// objects
Screen screen;
Screen header;
Screen console;
Object player;

int main()
{
#if PLATFORM == LINUX
    enableRawMode();
    refreshScreen();
    //-----------------------------------------------------------------WindowBuffer

    Buffer windowBuffer;
    windowBuffer.b = NULL;
    windowBuffer.len = 0;

    //-----------------------------------------------------------------Header

    initScreen(&header, 'X', 4, 66);
    header.buffer = (char*)malloc(header.rows * header.cols * sizeof(char));

    // header buffer
    for (int i = 0; i < header.rows; i++) {
        for (int j = 0; j < header.cols - 2; j++) {
            *(pixel(&header, i, j)) = header.type;
        }
        *(pixel(&header, i, header.cols - 2)) = '\r';
        *(pixel(&header, i, header.cols - 1)) = '\n';
    }

    //-----------------------------------------------------------------Screen

    initScreen(&screen, '*', 32, 66);
    screen.buffer = (char*)malloc(screen.rows * screen.cols * sizeof(char));
    // screen buffer
    for (int i = 0; i < screen.rows; i++) {
        for (int j = 0; j < screen.cols - 2; j++) {
            *(pixel(&screen, i, j)) = screen.type;
        }
        *(pixel(&screen, i, screen.cols - 2)) = '\r';
        *(pixel(&screen, i, screen.cols - 1)) = '\n';
    }

    //-----------------------------------------------------------------Console

    initScreen(&console, '+', 4, 66);
    console.buffer = (char*)malloc(console.rows * console.cols * sizeof(char));
    // console buffer
    for (int i = 0; i < console.rows; i++) {
        for (int j = 0; j < console.cols - 2; j++) {
            *(pixel(&console, i, j)) = console.type;
        }
        *(pixel(&console, i, console.cols - 2)) = '\r';
        *(pixel(&console, i, console.cols - 1)) = '\n';
    }

    // append screen buffers to window buffers
    appendBuffer(&windowBuffer, header.buffer, header.rows * header.cols);
    appendBuffer(&windowBuffer, screen.buffer, screen.rows * screen.cols);
    appendBuffer(&windowBuffer, console.buffer, console.rows * console.cols);
    write(STDOUT_FILENO, "\x1b[H", 3); // clear screen while exiting

    //-----------------------------------------------------------------Player
    initObject(&player, 176, 0, 0);

    //-----------------------------------------------------------------Tetromino
    matrix A;
    createMatrix(&A, 4, 4);
    appendString(&A, "**A*");
    appendString(&A, "*AA*");
    appendString(&A, "**A*");
    appendString(&A, "****");

    while (1) {
        char input = '\0';
        read(STDIN_FILENO, &input, 1);
        if (input == CTRL_KEY('q'))
            break;
        /*
            switch(input)
            {
              case('s'):
                spawnTet(&screen , &A);
                update(&screen);
              break;
            }
        */
        //-----------------------------------------------------------------Rendering

        // memcpy(buffer.b + buffer.len, screen.buffer, screen.rows *
        // screen.cols); buffer.len += screen.rows * screen.cols;

        write(STDOUT_FILENO, "\x1b[H", 3);
        write(STDOUT_FILENO, windowBuffer.b, windowBuffer.len);
    }

    free(header.buffer);
    free(screen.buffer);
    free(console.buffer);
    write(STDOUT_FILENO, "\x1b[H", 3); // clear screen while exiting
    return 0;

#elif PLATFORM == WINDOWS
    // @Implement

#endif
}

void spawnTet(Screen* s, matrix* m)
{
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            *(pixel(s, 0 + i, 31 + j)) = *(cell(m, i, j));
        }
    }
}

/*
void move(Object *object, int x, int y) {

  if (object->x + x >= 0 && object->x + x < screen.rows && object->y + y >= 0 &&
object->y + y < screen.cols)
  {
    screen.ptr[object->x * screen.cols + object->y] = screen.type;
    object->x += x;
    object->y += y;
    screen.ptr[object->x * screen.cols + object->y] = object->type;
  }
}
*/
void refreshScreen()
{
#if PLATFORM == LINUX
    Buffer buffer = {NULL, 0};
    appendBuffer(&buffer, (char*)"\x1b[2J", 4);
    appendBuffer(&buffer, (char*)"\x1b[H", 3);
    write(STDOUT_FILENO, buffer.b, buffer.len);
    bufferFree(&buffer);

#elif PLATFORM == WINDOWS
    // @Implement

#endif
}
