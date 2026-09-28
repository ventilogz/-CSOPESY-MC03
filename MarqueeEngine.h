#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class MarqueeEngine {
public:
    static constexpr int SCENE_WIDTH  = 80;
    static constexpr int SCENE_HEIGHT = 11;

    MarqueeEngine();
    ~MarqueeEngine();

    // Hooked up to CommandInterpreter::MarqueeHooks
    void start();
    void stop();
    void setText(const std::string& text);
    void setSpeed(const std::string& args);

    // Called from console_ui.cpp only (never from inside a hook)
    void setSceneRow(int row);   // 1-based screen row where the scene starts
    void drawStill();            // draws one frame without moving anything
    void shutdown();             // stops the thread; safe to call twice

private:
    struct Frame {
        std::vector<std::string> text;    // the characters
        std::vector<std::string> color;   // one color key per character
    };

    void run();
    Frame buildFrame(bool advance);
    void render(const Frame& frame);

    std::thread              worker_;
    std::atomic<bool>        alive_;
    std::atomic<bool>        shouldRun_;
    std::atomic<int>         speedMs_;
    std::atomic<std::size_t> position_;   // signboard text scroll offset
    std::atomic<int>         jeepX_;      // jeep's left column (goes negative while exiting)
    std::atomic<int>         sceneRow_;

    std::mutex  textMutex_;
    std::string text_;

    static constexpr int MIN_SPEED_MS     = 1; // Made lower for possible test cases
    static constexpr int MAX_SPEED_MS     = 60000; // Made higher for possible test cases
    static constexpr int DEFAULT_SPEED_MS = 200;
    static constexpr int SIGN_WIDTH       = 20;
    static constexpr int JEEP_WIDTH       = 40;
    static constexpr int JEEP_TOP         = 2;   // scene row where the jeep's sign starts
};
