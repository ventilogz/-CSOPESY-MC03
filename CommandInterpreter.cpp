#include "CommandInterpreter.h"

#include <iostream>

namespace {

// Strips leading/trailing whitespace (space, tab, CR, LF).
std::string trimWs(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

} // namespace

CommandInterpreter::CommandInterpreter() { buildTable(); }

// ---------------------------------------------------------------------------
// Dispatch table. To add a command later, add one entry here.
// ---------------------------------------------------------------------------
void CommandInterpreter::buildTable() {
    table_ = {
        { "help", "displays the commands and its description", "help", false,
          [this](const std::string&) { return cmdHelp(); } },

        { "start_marquee", "starts the marquee \"animation\"", "start_marquee", false,
          [this](const std::string&) {
              if (hooks_.onStart) hooks_.onStart(); else notConnected("start_marquee");
              return true;
          } },

        { "stop_marquee", "stops the marquee \"animation\"", "stop_marquee", false,
          [this](const std::string&) {
              if (hooks_.onStop) hooks_.onStop(); else notConnected("stop_marquee");
              return true;
          } },

        { "set_text", "accepts a text input and displays it as a marquee", "set_text <your_string>", true,
          [this](const std::string& args) { return cmdSetText(args); } },

        // Arguments are passed through untouched; Ven's engine validates the value.
        { "set_speed", "sets the marquee animation refresh in milliseconds", "set_speed <milliseconds>", true,
          [this](const std::string& args) {
              if (hooks_.onSetSpeed) hooks_.onSetSpeed(args); else notConnected("set_speed");
              return true;
          } },

        { "exit", "terminates the console", "exit", false,
          [this](const std::string&) { return cmdExit(); } },
    };
}

// ---------------------------------------------------------------------------
// Parser + dispatcher
// ---------------------------------------------------------------------------
bool CommandInterpreter::process(const std::string& line) {
    std::string input = trimWs(line);
    if (input.empty()) return true;                       

    
    std::size_t split = input.find_first_of(" \t");
    std::string name = input.substr(0, split);
    std::string args = (split == std::string::npos) ? "" : trimWs(input.substr(split));

    for (const Command& cmd : table_) {
        if (cmd.name == name) {                           
            if (!cmd.takesArgs && !args.empty()) {
                std::cout << "Error: '" << cmd.name << "' does not take any arguments."
                          << " Usage: " << cmd.usage << "\n\n";
                return true;
            }
            return cmd.handler(args);
        }
    }

    std::cout << "Error: unrecognized command \"" << name
              << "\". Type 'help' to see the available commands.\n\n";
    return true;
}

// ---------------------------------------------------------------------------
// Individual commands
// ---------------------------------------------------------------------------
bool CommandInterpreter::cmdHelp() {
    for (const Command& cmd : table_) {
        std::cout << cmd.name << " - " << cmd.description << "\n";
    }
    std::cout << "\n";
    return true;
}

bool CommandInterpreter::cmdSetText(const std::string& args) {
    if (args.empty()) {
        std::cout << "Error: set_text needs some text. Usage: set_text <your_string>\n\n";
        return true;
    }
    if (args.size() > MAX_TEXT_LENGTH) {
        std::cout << "Error: text is too long (" << args.size()
                  << " characters; maximum is " << MAX_TEXT_LENGTH << ").\n\n";
        return true;
    }

    savedText_ = args;                                    
    std::cout << "Text saved for marquee: " << savedText_ << "\n\n";

    if (hooks_.onSetText) hooks_.onSetText(savedText_);  
    return true;
}

bool CommandInterpreter::cmdExit() {
    std::cout << "Terminating console...\n";
    return false;
}

bool CommandInterpreter::notConnected(const std::string& name) {
    std::cout << "'" << name << "' is recognized, but the marquee engine is not connected yet.\n\n";
    return true;
}
