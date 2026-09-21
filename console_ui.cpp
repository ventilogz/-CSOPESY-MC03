#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "CommandInterpreter.h"   // inserted and changed sa file ni mau

#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Config — edit these once and the rest of the file needs no changes
// ---------------------------------------------------------------------------
static const std::string GROUP_MEMBERS[] = {
    "Lance Krystofer Galicia",
    "Raina Helaga",
    "Maurienne Marie Mojica",
    "Venice Raeka Plurad"
};
static const std::string VERSION_DATE = "\nVersion 1.0 | September 2026";

static const char* ASCII_HEADER =
R"(  ____ ____   ___  ____  _____ ______   __
 / ___/ ___| / _ \|  _ \| ____/ ___\ \ / /
| |   \___ \| | | | |_) |  _| \___ \\ V / 
| |___ ___) | |_| |  __/| |___ ___) || |  
 \____|____/ \___/|_|   |_____|____/ |_|  
)";

// ---------------------------------------------------------------------------
// Screen / display formatting
// ---------------------------------------------------------------------------

// Clears the terminal. Works on both Windows (system("cls")) and
// Linux/macOS (ANSI escape codes), so it behaves the same regardless of
// which IDE/OS a teammate is testing on.
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}

// Prints the startup banner: ASCII header, group developer names, version date.
void displayHeader() {
    std::cout << ASCII_HEADER << "\n";
    std::cout << "Group developers:\n";
    for (const std::string& name : GROUP_MEMBERS) {
        std::cout << "  " << name << "\n";
    }
    std::cout << VERSION_DATE << "\n";
    std::cout << std::string(44, '-') << "\n\n";
}


// Full redraw: clears the screen and reprints the header. This is the
// hook Member 2's marquee loop should call on each animation tick once
// it needs to repaint the screen without leaving old frames behind —
// keeping the refresh logic in one place avoids every module clearing
// the screen its own way.
void refreshScreen() {
    clearScreen();
    displayHeader();
}

// ---------------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------------

// Strips leading/trailing whitespace so "  help " and "help" behave the same.
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ---------------------------------------------------------------------------
// Main command loop
// ---------------------------------------------------------------------------

void runConsole() {
    displayHeader();

    CommandInterpreter interpreter;   // Ina part interpreter merged
    
    std::string rawInput;
    bool running = true;

    while (running) {
        std::cout << "Command> ";
        if (!std::getline(std::cin, rawInput)) {
            break; // stdin closed (EOF)
        }

        std::string command = trim(rawInput);

        if (command.empty()) {
            continue; // blank line: just reprompt
        }

        // The interpreter handles every command, including "exit".
        // process() returns false when the user typed exit.
        running = interpreter.process(command);
    }
}

int main() {
    runConsole();
    return 0;
}