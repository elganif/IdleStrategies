#ifndef PROFILER_TIMER
#define PROFILER_TIMER

class Profiler {
public:
    std::string defaultName = "CoreFrame";
    int frameCounter = 0;
    Profiler();
    ~Profiler() = default;
    struct Event{
        std::string identity;
        const int frameNum;
        int openCount = 1;
        std::chrono::_V2::steady_clock::time_point startT;
        std::chrono::_V2::steady_clock::time_point stopT;

        Event(std::string name,int frameOn):identity(name),frameNum(frameOn) {startT = std::chrono::_V2::steady_clock::now(); }
        std::chrono::_V2::steady_clock::duration passedTime(){ return std::chrono::duration(stopT - startT); }

    };

    std::map<int,Event> coreFrame;
    std::unordered_map<std::string, std::list<Event>> events;

    void frameMark();
    void start(std::string timerID);
    std::chrono::_V2::steady_clock::duration stop(std::string timerID);

    void drawDebug(olc::PixelGameEngine* game);
};

#endif
