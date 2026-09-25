#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>

#include "CommandInterpreter.h"
#include "MarqueeEngine.h"
#include "Terminal.h"

#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Config
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

void clearScreen() {
    std::cout << "\033[2J\033[1;1H" << std::flush;   // works on Windows too once terminalInit() runs
}

void displayHeader() {
    std::cout << ASCII_HEADER << "\n";
    std::cout << "Group developers:\n";
    for (const std::string& name : GROUP_MEMBERS) {
        std::cout << "  " << name << "\n";
    }
    std::cout << VERSION_DATE << "\n";
    std::cout << std::string(44, '-') << "\n\n";
}

void refreshScreen() {
    clearScreen();
    displayHeader();
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string readCommandLine(std::vector<std::string>& history) {
    std::string buffer;
    std::string draft;                          // saves your in-progress typing
    std::size_t historyIndex = history.size();  // starts "past the end" = blank line

    auto redrawLine = [&]() {
        std::cout << "\r";
        clearToEndOfLine();
        std::cout << "Command> " << buffer << std::flush;
    };

    while (true) {
        int ch = terminalReadKey();
        std::lock_guard<std::mutex> lock(screenMutex());

        if (ch == KEY_UP) {
            if (historyIndex > 0) {
                if (historyIndex == history.size()) draft = buffer; // stash it first
                historyIndex--;
                buffer = history[historyIndex];
                redrawLine();
            }
            continue;
        }
        if (ch == KEY_DOWN) {
            if (historyIndex < history.size()) {
                historyIndex++;
                buffer = (historyIndex == history.size()) ? draft : history[historyIndex];
                redrawLine();
            }
            continue;
        }
        if (ch == KEY_IGNORE) continue;

        if (ch == '\r' || ch == '\n') {
            std::cout << "\n";
            break;
        } else if (ch == 8 || ch == 127) {
            if (!buffer.empty()) {
                buffer.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else if (ch >= 32 && ch < 127) {
            buffer += static_cast<char>(ch);
            std::cout << static_cast<char>(ch) << std::flush;
        }
    }
    return buffer;
}

void runConsole() {
    terminalInit();
    clearScreen();

    // Screen layout:
    //   rows 1..11  jeepney scene (redrawn by the marquee thread)
    //   row  12     divider
    //   row  13+    header, commands and output (only this part scrolls)
    const int sceneRow   = 1;
    const int dividerRow = sceneRow + MarqueeEngine::SCENE_HEIGHT;
    const int consoleTop = dividerRow + 1;
    const int screenRows = terminalRows();

    std::cout << "\033[" << dividerRow << ";1H" << std::string(MarqueeEngine::SCENE_WIDTH, '=');
    std::cout << "\033[" << consoleTop << ";" << screenRows << "r";   // scroll region
    std::cout << "\033[" << consoleTop << ";1H";                      // cursor into it

    displayHeader();

    CommandInterpreter interpreter;

    MarqueeEngine marquee;
    marquee.setSceneRow(sceneRow);
    marquee.drawStill();                  // parked jeep until start_marquee

    CommandInterpreter::MarqueeHooks hooks;
    hooks.onSetText  = [&marquee](const std::string& text) { marquee.setText(text); };
    hooks.onStart    = [&marquee]() { marquee.start(); };
    hooks.onStop     = [&marquee]() { marquee.stop(); };
    hooks.onSetSpeed = [&marquee](const std::string& args) { marquee.setSpeed(args); };
    interpreter.setHooks(hooks);

    {
        std::lock_guard<std::mutex> lock(screenMutex());
        std::cout << "Command> " << std::flush;
    }

    std::vector<std::string> commandHistory;

    bool running = true;
    while (running) {
        std::string command = trim(readCommandLine(commandHistory));

        std::lock_guard<std::mutex> lock(screenMutex());   // fix 2: all printing is locked

        if (command.empty()) {
            std::cout << "Command> " << std::flush;
            continue;
        }

        commandHistory.push_back(command);
        running = interpreter.process(command);

        if (running) {
            std::cout << "Command> " << std::flush;
        }
    }

    marquee.shutdown();                                         // stop drawing first
    std::cout << "\033[r\033[" << screenRows << ";1H\n" << std::flush;  // undo scroll region
    terminalShutdown();
}

int main() {
    runConsole();
    return 0;
}