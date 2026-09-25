#pragma once
#include <mutex>

std::mutex& screenMutex();

constexpr int KEY_IGNORE = -1;
constexpr int KEY_UP = -2;
constexpr int KEY_DOWN = -3;

void terminalInit();      
void terminalShutdown();  

int terminalReadKey();
int terminalRows();      

void cursorSave();
void cursorRestore();
void cursorUp(int n = 1);
void clearToEndOfLine();