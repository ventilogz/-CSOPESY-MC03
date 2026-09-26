#pragma once
#include <mutex>

std::mutex& screenMutex();

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