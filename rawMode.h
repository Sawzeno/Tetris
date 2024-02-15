#include "platform.h"
#define CTRL_KEY(k) ((k) & 0x1f) // sets upper three bits to zero

void die(char* s);

struct PortableTermios {
    int placeholder;
};

struct editorConfig {
#if PLATFORM == LINUX
    struct PortableTermios orig_termios;

#elif PLATFORM == WINDOWS
    struct PortableTermios orig_termios;

#endif
};

void disableRawMode();

void enableRawMode();
