#include "MarqueeEngine.h"
#include "Terminal.h"

#include <chrono>
#include <iostream>
#include <stdexcept>

MarqueeEngine::MarqueeEngine()
    : alive_(true), shouldRun_(false), speedMs_(DEFAULT_SPEED_MS), position_(0) {
    // Thread starts immediately but sits idle (shouldRun_ == false)
    // until start_marquee flips the flag.
    worker_ = std::thread(&MarqueeEngine::run, this);
}

MarqueeEngine::~MarqueeEngine() {
    alive_ = false;
    if (worker_.joinable()) worker_.join();
}

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
    std::cout << "\nMarquee stopped.\n\n";
}

// set_text
void MarqueeEngine::setText(const std::string& text) {
    std::lock_guard<std::mutex> lock(textMutex_);
    text_ = text;
    position_ = 0; // restart scroll from the beginning on new text
}

// set_speed <ms> — validates and clamps to a safe range
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
        std::cout << "Error: " << ms << "ms is too fast (min " << MIN_SPEED_MS
                   << "ms). Clamped to " << MIN_SPEED_MS << "ms.\n\n";
        ms = MIN_SPEED_MS;
    } else if (ms > MAX_SPEED_MS) {
        std::cout << "Error: " << ms << "ms is too slow (max " << MAX_SPEED_MS
                   << "ms). Clamped to " << MAX_SPEED_MS << "ms.\n\n";
        ms = MAX_SPEED_MS;
    } else {
        std::cout << "Marquee speed set to " << ms << "ms.\n\n";
    }

    speedMs_ = ms;
}

void MarqueeEngine::run() {
    while (alive_) {
        if (shouldRun_) {
            render(buildFrame());
            std::this_thread::sleep_for(std::chrono::milliseconds(speedMs_.load()));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // idle poll
        }
    }
}

// Computes the next MARQUEE_WIDTH-character slice of the (looping) text.
std::string MarqueeEngine::buildFrame() {
    std::lock_guard<std::mutex> lock(textMutex_);

    if (text_.empty()) {
        return std::string(MARQUEE_WIDTH, ' ');
    }

    std::string loop = text_ + "   "; // gap so the text doesn't smash into its repeat
    std::size_t len = loop.size();
    std::string doubled = loop + loop; // lets substr() wrap around cleanly

    std::size_t pos = position_.load() % len;
    std::string frame = doubled.substr(pos, MARQUEE_WIDTH);
    if (frame.size() < MARQUEE_WIDTH) {
        frame += std::string(MARQUEE_WIDTH - frame.size(), ' ');
    }

    position_ = (pos + 1) % len;
    return frame;
}

void MarqueeEngine::render(const std::string& frame) {
    std::lock_guard<std::mutex> lock(screenMutex());
    cursorSave();
    cursorUp(1);
    std::cout << "[MARQUEE] " << frame;
    clearToEndOfLine();
    cursorRestore();
    std::cout.flush();
}