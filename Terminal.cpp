#include "Terminal.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <sys/ioctl.h>
#endif

std::mutex& screenMutex() {
    static std::mutex m;
    return m;
}

#ifdef _WIN32

static DWORD g_oldMode = 0;

void terminalInit() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(h, &g_oldMode);
    SetConsoleMode(h, g_oldMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void terminalShutdown() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(h, g_oldMode);
}

int terminalReadKey() {
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        int ch2 = _getch();
        if (ch2 == 72) return KEY_UP;
        if (ch2 == 80) return KEY_DOWN;
        return KEY_IGNORE;
    }
    return ch;
}

int terminalRows() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return info.srWindow.Bottom - info.srWindow.Top + 1;
    return 30;
}

#else

static termios g_oldTermios;

void terminalInit() {
    tcgetattr(STDIN_FILENO, &g_oldTermios);
    termios raw = g_oldTermios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void terminalShutdown() {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_oldTermios);
}

int terminalReadKey() {
    int ch = getchar();
    if (ch == 27) {
        int ch2 = getchar();
        if (ch2 == '[') {
            int ch3 = getchar();
            if (ch3 == 'A') return KEY_UP;
            if (ch3 == 'B') return KEY_DOWN;
            return KEY_IGNORE;
        }
        return KEY_IGNORE;
    }
    return ch;
}

int terminalRows() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) return ws.ws_row;
    return 24;
}

#endif

void cursorSave()       { std::cout << "\0337"; }
void cursorRestore()    { std::cout << "\0338"; }
void cursorUp(int n)    { std::cout << "\033[" << n << "A\r"; }
void clearToEndOfLine() { std::cout << "\033[K"; }