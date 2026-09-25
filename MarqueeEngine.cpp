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

} // namespace

MarqueeEngine::MarqueeEngine()
    : alive_(true), shouldRun_(false), speedMs_(DEFAULT_SPEED_MS), position_(0),
      jeepX_((SCENE_WIDTH - JEEP_WIDTH) / 2), sceneRow_(1),
      text_("TAFT - VITO CRUZ") {                    // fix 4: never starts empty
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
        std::cout << "Marquee is already running.\n\n";
        return;
    }
    shouldRun_ = true;
    std::cout << "Marquee started.\n\n";
}

void MarqueeEngine::stop() {
    if (!shouldRun_) {
        std::cout << "Marquee is already stopped.\n\n";
        return;
    }
    shouldRun_ = false;
    std::cout << "Marquee stopped.\n\n";
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
        std::cout << "Error: set_speed needs a whole number of milliseconds. "
                     "Usage: set_speed <milliseconds>\n\n";
        return;
    }

    if (ms < MIN_SPEED_MS) {
        std::cout << ms << "ms is below the minimum. Speed clamped to " << MIN_SPEED_MS << "ms.\n\n";
        ms = MIN_SPEED_MS;
    } else if (ms > MAX_SPEED_MS) {
        std::cout << ms << "ms is above the maximum. Speed clamped to " << MAX_SPEED_MS << "ms.\n\n";
        ms = MAX_SPEED_MS;
    } else {
        std::cout << "Marquee speed set to " << ms << "ms.\n\n";
    }
    speedMs_ = ms;
}

void MarqueeEngine::run() {
    while (alive_) {
        if (shouldRun_) {
            render(buildFrame(true));
            // fix 3: sleep in small slices so exit/stop never wait out a long delay
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

std::vector<std::string> MarqueeEngine::buildFrame(bool advance) {
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
    std::vector<std::string> canvas(SKYLINE);
    canvas.push_back("");
    canvas.push_back(ROAD_MARKS);
    canvas.push_back(std::string(SCENE_WIDTH, '-'));
    for (std::string& row : canvas) row.resize(SCENE_WIDTH, ' ');

    // 3) jeep on top; leading/trailing spaces of each sprite row are see-through
    int x = jeepX_.load();
    std::vector<std::string> jeep = jeepSprite(sign);
    for (std::size_t r = 0; r < jeep.size(); ++r) {
        const std::string& line = jeep[r];
        std::size_t first = line.find_first_not_of(' ');
        std::size_t last  = line.find_last_not_of(' ');
        if (first == std::string::npos) continue;
        std::string& dest = canvas[JEEP_TOP + r];
        for (std::size_t c = first; c <= last; ++c) {
            int col = x + static_cast<int>(c);
            if (col >= 0 && col < SCENE_WIDTH) dest[col] = line[c];
        }
    }

    // 4) drive left one column; once fully off screen, re-enter from the right
    if (advance) {
        --x;
        if (x < -JEEP_WIDTH) x = SCENE_WIDTH;
        jeepX_ = x;
    }
    return canvas;
}

void MarqueeEngine::render(const std::vector<std::string>& frame) {
    // Build the whole frame as one string and write it once (less flicker/tearing).
    std::string out = "\0337\033[?25l";          // save cursor, hide it
    int row = sceneRow_.load();
    for (std::size_t i = 0; i < frame.size(); ++i) {
        out += "\033[" + std::to_string(row + static_cast<int>(i)) + ";1H";
        out += frame[i];
    }
    out += "\0338\033[?25h";                     // back to the prompt, show cursor

    std::lock_guard<std::mutex> lock(screenMutex());
    std::cout << out << std::flush;
}
