#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

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
// Command hand-off (placeholder until Member 3's interpreter is merged in)
// ---------------------------------------------------------------------------

// This is intentionally NOT the real command interpreter — Member 3 owns
// parsing/dispatch for help, start_marquee, stop_marquee, set_text, and
// set_speed. This stub only proves the prompt correctly reads and echoes
// input, per the "prompt accepts and echoes input as expected" test case.
// Replace the body of this function with a call into Member 3's module,
// e.g.: CommandInterpreter::process(command);
void handleCommand(const std::string& command) {
    std::cout << "You entered: \"" << command << "\"\n";
    std::cout << "(Command interpreter not yet wired in - placeholder echo only.)\n\n";
}

// ---------------------------------------------------------------------------
// Main command loop
// ---------------------------------------------------------------------------

void runConsole() {
    displayHeader();

    std::string rawInput;
    bool running = true;

    while (running) {
        std::cout << "Command> ";
        if (!std::getline(std::cin, rawInput)) {
            // stdin closed (e.g. EOF) — exit cleanly instead of looping forever
            break;
        }

        std::string command = trim(rawInput);

        if (command.empty()) {
            continue; // blank line: just reprompt, don't echo an error
        }

        if (command == "exit") {
            std::cout << "\nTerminating console...\n";
            running = false;
            continue;
        }

        handleCommand(command);
    }
}

// ---------------------------------------------------------------------------
// Entry point — lets Mau compile and run this file completely on its own.
// Once merged with teammates' modules, main() will likely move to a shared
// file; runConsole() itself does not need to change.
// ---------------------------------------------------------------------------
int main() {
    runConsole();
    return 0;
}