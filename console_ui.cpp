#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "CommandInterpreter.h"
#include "MarqueeEngine.h"
#include "Terminal.h"

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

// Green "Command>" prompt. PROMPT_WIDTH is what you see on screen (color codes take no space).
static const std::string PROMPT = std::string(color::GREEN) + color::BOLD + "Command>" + color::RESET + " ";
static const int PROMPT_WIDTH = 9;

void clearScreen() {
    std::cout << "\033[2J\033[1;1H" << std::flush;   // works on Windows too once terminalInit() runs
}

void displayHeader() {
    std::cout << color::CYAN << ASCII_HEADER << color::RESET << "\n";
    std::cout << color::BOLD << "Group developers:" << color::RESET << "\n";
    for (const std::string& name : GROUP_MEMBERS) {
        std::cout << "  " << name << "\n";
    }
    std::cout << color::GRAY << VERSION_DATE << color::RESET << "\n";
    std::cout << color::GRAY << std::string(44, '-') << color::RESET << "\n\n";
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
    const int room = std::max(10, terminalCols() - PROMPT_WIDTH - 1);

    std::string buffer;
    std::string draft;                          // saves your in-progress typing
    std::size_t historyIndex = history.size();  // starts "past the end" = blank line

    // Redraws the prompt on ONE row. If the text is longer than the row,
    // only the last part is shown, so the line never wraps.
    auto redrawLine = [&]() {
        std::string shown = buffer;
        if (static_cast<int>(shown.size()) > room) shown = shown.substr(shown.size() - room);
        std::cout << "\r" << PROMPT << shown;
        clearToEndOfLine();
        std::cout << std::flush;
    };

    while (true) {
        int ch = terminalReadKey();
        std::lock_guard<std::mutex> lock(screenMutex());

        // Ctrl+C, Ctrl+D or closed input: behave exactly like typing "exit"
        if (ch == 3 || ch == 4 || ch == KEY_EOF) {
            buffer = "exit";
            redrawLine();
            std::cout << "\n";
            break;
        }

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
            // Print the full command once (it may wrap now; editing is over)
            std::cout << "\r" << PROMPT << buffer;
            clearToEndOfLine();
            std::cout << "\n";
            break;
        } else if (ch == 8 || ch == 127) {
            if (!buffer.empty()) {
                buffer.pop_back();
                redrawLine();
            }
        } else if (ch >= 32 && ch < 127) {
            buffer += static_cast<char>(ch);
            redrawLine();
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
    const int minRows    = consoleTop + 8;   // room for at least a few lines of output

    // Wait until the window is big enough for the layout
    while (terminalRows() < minRows || terminalCols() < MarqueeEngine::SCENE_WIDTH) {
        clearScreen();
        std::cout << "Window too small: " << terminalCols() << "x" << terminalRows()
                  << ". Resize to at least " << MarqueeEngine::SCENE_WIDTH << "x" << minRows
                  << ", then press any key.\n" << std::flush;
        if (terminalReadKey() == KEY_EOF) break;
    }
    clearScreen();
    const int screenRows = terminalRows();

    std::cout << "\033[" << dividerRow << ";1H" << color::CYAN
              << std::string(MarqueeEngine::SCENE_WIDTH, '=') << color::RESET;
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
        std::cout << PROMPT << std::flush;
    }

    std::vector<std::string> commandHistory;

    bool running = true;
    while (running) {
        std::string command = trim(readCommandLine(commandHistory));

        std::lock_guard<std::mutex> lock(screenMutex());   // all printing is locked

        if (command.empty()) {
            std::cout << PROMPT << std::flush;
            continue;
        }

        commandHistory.push_back(command);
        running = interpreter.process(command);

        if (running) {
            std::cout << PROMPT << std::flush;
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