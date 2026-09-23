#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class MarqueeEngine {
public:
    MarqueeEngine();
    ~MarqueeEngine();

    // Hooked up to CommandInterpreter::MarqueeHooks
    void start();                          
    void stop();                           
    void setText(const std::string& text); 
    void setSpeed(const std::string& args);

private:
    void run();                   
    std::string buildFrame();     
    void render(const std::string& frame);

    std::thread       worker_;
    std::atomic<bool> alive_;      
    std::atomic<bool> shouldRun_;  
    std::atomic<int>  speedMs_;
    std::atomic<std::size_t> position_;

    std::mutex  textMutex_;        
    std::string text_;

    static constexpr int MIN_SPEED_MS   = 10;    
    static constexpr int MAX_SPEED_MS   = 5000;  
    static constexpr int DEFAULT_SPEED_MS = 200;
    static constexpr int MARQUEE_WIDTH  = 40;     
};