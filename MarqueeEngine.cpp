#include "MarqueeEngine.h"
#include "Terminal.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace {

// Background skyline (8 rows). Rows are padded/cut to SCENE_WIDTH in code.
const std::vector<std::string> SKYLINE = {
    "                 ___                                       _____",
    "      |~|       |   |          ____                       |     |",
    "     _|_|_      |[] |   ___   |    |     _______          | [] |      ___",
    "    |     |     |   |  |   |  | [] |    |       |    ___  |     |    |   |",
    "    | []  |__   | []|  |[] |  |    |    | [] [] |   |   | | [] |    |[] |",
    "    |     |  |  |   |  |   |  | [] |    |       |   |[] | |     |    |   |",
    "    | []  |[]|  | []|  |[] |  |    |    | [] [] |   |   | | [] |    |[] |",
    "____|_____|__|__|___|__|___|__|____|____|_______|___|___|_|_____|____|___|______",
};

const std::string ROAD_MARKS =
    "  ====      ====      ====      ====      ====      ====      ====      ====  ";

// Jeepney facing left. Row 1 is the signboard; its text changes every frame.
std::vector<std::string> jeepSprite(const std::string& sign) {
    return {
        "            .----------------------.",
        "            | " + sign + " |",
        "     _______|______________________|____",
        "    / __  |  []   []   []   []   []    |",
        "   / |__| |                            |",
        "  |=======|============================|",
        "   '-(O)--------------------------(O)--'",
    };
}

// Color key -> ANSI code. Every key starts with a reset so bold/background never leak.
const char* ansiFor(char key) {
    switch (key) {
        case 'b': return "\033[0;90m";        // buildings: dark gray
        case 'w': return "\033[0;93m";        // lit windows: bright yellow
        case 'r': return "\033[0;33m";        // lane marks: yellow
        case 'c': return "\033[0;90m";        // curb: dark gray
        case 'j': return "\033[0;91m";        // jeep body: bright red
        case 'g': return "\033[0;96m";        // jeep windows: bright cyan
        case 't': return "\033[0;93m";        // jeep trim stripe: bright yellow
        case 'f': return "\033[0;97m";        // signboard frame: bright white
        case 's': return "\033[0;1;97;44m";   // sign text: bold white on blue
        case 'o': return "\033[0;97m";        // wheels: bright white
        default:  return "\033[0m";
    }
}

// Picks a color key for one character of the jeep sprite (row r, column c).
char jeepColor(std::size_t r, std::size_t c, char ch) {
    switch (r) {
        case 0:  return 'f';                                        // sign top edge
        case 1:  return (c >= 13 && c <= 34) ? 's' : 'f';           // sign text area
        case 3:
        case 4:
            if (ch == '[' || ch == ']') return 'g';                 // side windows
            if (c >= 5 && c <= 9) return 'g';                       // windshield
            return 'j';
        case 5:  return ch == '=' ? 't' : 'j';                      // trim stripe
        case 6:  return (ch == '(' || ch == 'O' || ch == ')') ? 'o' : 'j';  // wheels
        default: return 'j';
    }
}

} // namespace

MarqueeEngine::MarqueeEngine()
    : alive_(true), shouldRun_(false), speedMs_(DEFAULT_SPEED_MS), position_(0),
      jeepX_((SCENE_WIDTH - JEEP_WIDTH) / 2), sceneRow_(1),
      text_("TAFT - VITO CRUZ") {                    // never starts empty
    worker_ = std::thread(&MarqueeEngine::run, this);
}

MarqueeEngine::~MarqueeEngine() { shutdown(); }

void MarqueeEngine::shutdown() {
    alive_ = false;
    if (worker_.joinable()) worker_.join();
}

void MarqueeEngine::setSceneRow(int row) { sceneRow_ = row; }

void MarqueeEngine::drawStill() { render(buildFrame(false)); }

// start_marquee / stop_marquee
void MarqueeEngine::start() {
    if (shouldRun_) {
        std::cout << color::YELLOW << "Marquee is already running." << color::RESET << "\n\n";
        return;
    }
    shouldRun_ = true;
    std::cout << color::GREEN << "Marquee started." << color::RESET << "\n\n";
}

void MarqueeEngine::stop() {
    if (!shouldRun_) {
        std::cout << color::YELLOW << "Marquee is already stopped." << color::RESET << "\n\n";
        return;
    }
    shouldRun_ = false;
    std::cout << color::GREEN << "Marquee stopped." << color::RESET << "\n\n";
}

// set_text
void MarqueeEngine::setText(const std::string& text) {
    std::lock_guard<std::mutex> lock(textMutex_);
    text_ = text;
    position_ = 0;
}

// set_speed <ms>: validates and clamps to a safe range
void MarqueeEngine::setSpeed(const std::string& args) {
    int ms;
    try {
        std::size_t consumed = 0;
        ms = std::stoi(args, &consumed);
        if (consumed != args.size()) throw std::invalid_argument("trailing characters");
    } catch (...) {
        std::cout << color::RED << "Error: set_speed needs a whole number of milliseconds. "
                     "Usage: set_speed <milliseconds>" << color::RESET << "\n\n";
        return;
    }

    if (ms < MIN_SPEED_MS) {
        std::cout << color::YELLOW << ms << "ms is below the minimum. Speed clamped to "
                  << MIN_SPEED_MS << "ms." << color::RESET << "\n\n";
        ms = MIN_SPEED_MS;
    } else if (ms > MAX_SPEED_MS) {
        std::cout << color::YELLOW << ms << "ms is above the maximum. Speed clamped to "
                  << MAX_SPEED_MS << "ms." << color::RESET << "\n\n";
        ms = MAX_SPEED_MS;
    } else {
        std::cout << color::GREEN << "Marquee speed set to " << ms << "ms." << color::RESET << "\n\n";
    }
    speedMs_ = ms;
}

void MarqueeEngine::run() {
    while (alive_) {
        if (shouldRun_) {
            render(buildFrame(true));
            // sleep in small slices so exit/stop never wait out a long delay
            int waited = 0;
            while (alive_ && shouldRun_ && waited < speedMs_.load()) {
                int step = std::min(10, speedMs_.load() - waited);
                std::this_thread::sleep_for(std::chrono::milliseconds(step));
                waited += step;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

MarqueeEngine::Frame MarqueeEngine::buildFrame(bool advance) {
    // 1) the 20-character slice of text shown on the signboard
    std::string sign;
    {
        std::lock_guard<std::mutex> lock(textMutex_);
        std::string loop = text_ + "   ";
        std::string strip = loop;
        while (strip.size() < loop.size() + SIGN_WIDTH) strip += loop;
        std::size_t pos = position_.load() % loop.size();
        sign = strip.substr(pos, SIGN_WIDTH);
        if (advance) position_ = (pos + 1) % loop.size();
    }

    // 2) background: skyline, open road (wheels go here), lane marks, curb
    Frame f;
    f.text = SKYLINE;
    f.text.push_back("");
    f.text.push_back(ROAD_MARKS);
    f.text.push_back(std::string(SCENE_WIDTH, '-'));
    for (std::string& row : f.text) row.resize(SCENE_WIDTH, ' ');

    for (std::size_t r = 0; r < f.text.size(); ++r) {
        std::string keys(SCENE_WIDTH, 'b');
        for (int c = 0; c < SCENE_WIDTH; ++c) {
            char ch = f.text[r][c];
            if (r < SKYLINE.size()) keys[c] = (ch == '[' || ch == ']' || ch == '~') ? 'w' : 'b';
            else                    keys[c] = (ch == '=') ? 'r' : 'c';
        }
        f.color.push_back(keys);
    }

    // 3) jeep on top; leading/trailing spaces of each sprite row are see-through
    int x = jeepX_.load();
    std::vector<std::string> jeep = jeepSprite(sign);
    for (std::size_t r = 0; r < jeep.size(); ++r) {
        const std::string& line = jeep[r];
        std::size_t first = line.find_first_not_of(' ');
        std::size_t last  = line.find_last_not_of(' ');
        if (first == std::string::npos) continue;
        for (std::size_t c = first; c <= last; ++c) {
            int col = x + static_cast<int>(c);
            if (col >= 0 && col < SCENE_WIDTH) {
                f.text[JEEP_TOP + r][col]  = line[c];
                f.color[JEEP_TOP + r][col] = jeepColor(r, c, line[c]);
            }
        }
    }

    // 4) drive left one column; once fully off screen, re-enter from the right
    if (advance) {
        --x;
        if (x < -JEEP_WIDTH) x = SCENE_WIDTH;
        jeepX_ = x;
    }
    return f;
}

void MarqueeEngine::render(const Frame& frame) {
    // Build the whole frame as one string and write it once (less flicker/tearing).
    std::string out = "\0337\033[?25l";          // save cursor, hide it
    int row = sceneRow_.load();
    for (std::size_t i = 0; i < frame.text.size(); ++i) {
        out += "\033[" + std::to_string(row + static_cast<int>(i)) + ";1H";
        char current = 0;
        for (std::size_t c = 0; c < frame.text[i].size(); ++c) {
            char key = frame.color[i][c];
            if (key != current) { out += ansiFor(key); current = key; }  // only switch when it changes
            out += frame.text[i][c];
        }
        out += "\033[0m";
    }
    out += "\0338\033[?25h";                     // back to the prompt, show cursor

    std::lock_guard<std::mutex> lock(screenMutex());
    std::cout << out << std::flush;
}