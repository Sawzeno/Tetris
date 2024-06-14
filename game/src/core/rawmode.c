#include "rawmode.h"
#include "platform/platform.h"
#include <stdio.h>

#if PLATFORM == LINUX
#include  <iconv.h>
#include  <termios.h>
#include  <unistd.h>
#include  <stdlib.h>

struct editorConfig E;

void die(char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 4);
    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

    // input flags
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output flags
    raw.c_oflag &= ~(OPOST);

    // control flags
    raw.c_cflag |= (CS8);

    // local flags
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // special characters
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

#elif PLATFORM == WINDOWS
#include <Windows.h>

struct Feature {
    char const *name;
    DWORD field;
};

static struct Feature inputModes[] = {{"Echo input", 0x0004},
                                      {"Insert mode", 0x0020},
                                      {"Line input", 0x0002},
                                      {"Mouse input", 0x0010},
                                      {"Processed input", 0x0001},
                                      {"Quick edit mode", 0x0040},
                                      {"Window input", 0x0008},
                                      {"Virtual terminal input", 0x0200}};

static struct Feature outputModes[] = {{"Processed output", 0x0001},
                                       {"Wrap at EOL output", 0x0002},
                                       {"Virtual terminal processing", 0x0004},
                                       {"Disable newline auto return", 0x0008},
                                       {"LVB grid worldwide", 0x0010}};

int const NUM_INPUT_FLAGS = 8;
int const NUM_OUTPUT_FLAGS = 5;

/**
 * @brief Given an array of features and a feature name, returns the bit
 * position of the feature flag
 *
 * @param name String representing the name of the feature
 * @param featureList Array of features
 * @param listSize Size of the feature array
 * @return Bitmask with the feature's bit set
 */
static DWORD getFeatureBitField(char const *name,
                                struct Feature const *featureList,
                                int listSize)
{
    for (int i = 0; i < listSize; ++i) {
        if (!strcmp(featureList[i].name, name)) {
            return featureList[i].field;
        }
    }
    // Not in list
    return 0;
}

/**
 * @brief Prints the state of a feature given a status bitmask
 *
 * @param f
 * @param flags Status flags
 */
static void printFeature(struct Feature f, DWORD flags)
{
    if (flags & f.field) {
        printf("%s: Enabled\n", f.name);
    }
    else {
        printf("%s: Disabled\n", f.name);
    }
}

static bool getConsoleMode(DWORD stdHandle, LPDWORD destination)
{
    HANDLE const handle = GetStdHandle(stdHandle);

    if (handle == INVALID_HANDLE_VALUE) {
        fputs("Error retrieving device handle\n", stdout);
        return false;
    }

    if (!GetConsoleMode(handle, destination)) {
        fputs("Error retrieving console mode for the device\n", stdout);
        return false;
    }

    return true;
}

static bool disableInputMode(DWORD flags)
{
    HANDLE const handle = GetStdHandle(STD_INPUT_HANDLE);

    if (handle == INVALID_HANDLE_VALUE) {
        fputs("Error retrieving device handle\n", stdout);
        return false;
    }

    struct WindowsConsoleMode current;
    saveConsoleMode(&current);
    DWORD attempt = current.inputMode & (~flags);
    if (!SetConsoleMode(handle, attempt)) {
        fputs("Error disabling input mode\n", stdout);
        printf("Error code: %d\n", GetLastError());
        return false;
    }

    return true;
}

static bool disableOutputMode(DWORD flags)
{
    HANDLE const handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (handle == INVALID_HANDLE_VALUE) {
        fputs("Error retrieving device handle\n", stdout);
        return false;
    }

    struct WindowsConsoleMode current;
    saveConsoleMode(&current);
    DWORD attempt = current.outputMode & (~flags);
    if (!SetConsoleMode(handle, attempt)) {
        fputs("Error disabling output mode\n", stdout);
        printf("Error code: %d\n", GetLastError());
        return false;
    }

    return true;
}

bool saveConsoleMode(struct WindowsConsoleMode *w)
{
    bool noErrors = true;
    noErrors = noErrors && getConsoleMode(STD_INPUT_HANDLE, &(w->inputMode));
    noErrors = noErrors && getConsoleMode(STD_OUTPUT_HANDLE, &(w->outputMode));
    return noErrors;
}

void printConsoleMode(void)
{
    struct WindowsConsoleMode current;
    saveConsoleMode(&current);

    printf("Console input mode: 0x%04X\n", current.inputMode);

    printFeature(inputModes[0], current.inputMode);
    printFeature(inputModes[1], current.inputMode);
    printFeature(inputModes[2], current.inputMode);
    printFeature(inputModes[3], current.inputMode);
    printFeature(inputModes[4], current.inputMode);
    printFeature(inputModes[5], current.inputMode);
    printFeature(inputModes[6], current.inputMode);
    printFeature(inputModes[7], current.inputMode);

    printf("\n");

    printf("Console output mode: 0x%04X\n", current.outputMode);

    printFeature(outputModes[0], current.outputMode);
    printFeature(outputModes[1], current.outputMode);
    printFeature(outputModes[2], current.outputMode);
    printFeature(outputModes[3], current.outputMode);
    printFeature(outputModes[4], current.outputMode);

    printf("\n");
}

bool restoreConsoleMode(struct WindowsConsoleMode const *w)
{
    HANDLE const inHandle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE const outHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (inHandle == INVALID_HANDLE_VALUE || outHandle == INVALID_HANDLE_VALUE) {
        fputs("Error retrieving device handle\n", stdout);
        return false;
    }

    if (!SetConsoleMode(inHandle, w->inputMode)) {
        fputs("Error restoring input mode\n", stdout);
        printf("Error code: %d\n", GetLastError());
        return false;
    }

    if (!SetConsoleMode(outHandle, w->outputMode)) {
        fputs("Error restoring output mode\n", stdout);
        printf("Error code: %d\n", GetLastError());
        return false;
    }

    return true;
}

bool enableRawMode(void)
{
    bool noErrors = true;

    noErrors = noErrors &&
               disableInputMode(ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
                                ENABLE_ECHO_INPUT);

    noErrors = noErrors && disableOutputMode(ENABLE_PROCESSED_OUTPUT |
                                             ENABLE_WRAP_AT_EOL_OUTPUT);

    return noErrors;
}

#endif
