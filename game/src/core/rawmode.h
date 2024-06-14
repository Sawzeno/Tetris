#include "platform/platform.h"
#include <stdbool.h>
#define CTRL_KEY(k) ((k) & 0x1f) // sets upper three bits to zero

#if PLATFORM == LINUX
#include <iconv.h>
#include <termios.h>
#include <unistd.h>

struct editorConfig {
    struct termios orig_termios;
};

void die(char *s);

void disableRawMode();

void enableRawMode();

#elif PLATFORM == WINDOWS
#include <Windows.h>

struct WindowsConsoleMode {
    DWORD inputMode;
    DWORD outputMode;
};

bool saveConsoleMode(struct WindowsConsoleMode *w);

void printConsoleMode(void);

bool restoreConsoleMode(struct WindowsConsoleMode const *w);

bool enableRawMode(void);

#endif
