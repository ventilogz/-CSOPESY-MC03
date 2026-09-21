// ---------------------------------------------------------------------------
// CommandInterpreter.h  
//
// Use:
//   * parse a raw input line into  <command> <arguments>
//   * dispatch it through a table of known commands (std::string comparison)
//   * help, set_text (validation + hand-off), exit, unrecognized commands
//
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

class CommandInterpreter {
public:
    
    struct MarqueeHooks {
        std::function<void(const std::string& text)> onSetText;    // called after set_text passes validation
        std::function<void()>                        onStart;      // start_marquee
        std::function<void()>                        onStop;       // stop_marquee
        std::function<void(const std::string& args)> onSetSpeed;   // set_speed <raw args>
    };

    static constexpr std::size_t MAX_TEXT_LENGTH = 1000;

    CommandInterpreter();

    void setHooks(const MarqueeHooks& hooks) { hooks_ = hooks; }

    // Processes and validates one line typed at the "Command>" prompt.
    bool process(const std::string& line);

    // The text most recently accepted by set_text 
    const std::string& savedText() const { return savedText_; }

private:
    struct Command {
        std::string name;
        std::string description;   // shown by help
        std::string usage;         // shown when arguments are wrong
        bool        takesArgs;     // false -> extra text after the command is an error
        std::function<bool(const std::string& args)> handler; // returns false to quit
    };

    void buildTable();
    bool cmdHelp();
    bool cmdSetText(const std::string& args);
    bool cmdExit();
    bool notConnected(const std::string& name);

    std::vector<Command> table_;   // order here = order printed by help
    MarqueeHooks         hooks_;
    std::string          savedText_;
};
