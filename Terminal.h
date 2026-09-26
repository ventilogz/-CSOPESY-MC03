#pragma once
#include <mutex>

std::mutex& screenMutex();

// ANSI colors for console text (Windows supports these once terminalInit() runs)
namespace color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* RED    = "\033[91m";
    constexpr const char* GREEN  = "\033[92m";
    constexpr const char* YELLOW = "\033[93m";
    constexpr const char* CYAN   = "\033[96m";
    constexpr const char* GRAY   = "\033[90m";
    constexpr const char* BOLD   = "\033[1m";
}

constexpr int KEY_IGNORE = -1;
constexpr int KEY_UP = -2;
constexpr int KEY_DOWN = -3;
constexpr int KEY_EOF = -4;   

void terminalInit();      
void terminalShutdown();  

int terminalReadKey();
int terminalRows();      
int terminalCols();     

void cursorSave();
void cursorRestore();
void cursorUp(int n = 1);
void clearToEndOfLine();