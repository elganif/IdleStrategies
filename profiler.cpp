#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "profiler.h"

/// class Profiler {

    Profiler::Profiler()
    {
        coreFrame.emplace(frameCounter++,Event(defaultName,frameCounter));
    }

    void Profiler::frameMark()
    { /// Called to mark when a new frame cycle is started.
        coreFrame.emplace(frameCounter++, Event(defaultName,frameCounter));

        /// set the stop time of last counter to the start time we just created.
        /// because an event is added in contruction and we just added a second we can safely access the second member of the list.
        std::next(coreFrame.rbegin())->second.stopT = coreFrame.rbegin()->second.startT;
        std::next(coreFrame.rbegin())->second.openCount--;

        /// remove events keeping track of only the last 60.
        while(coreFrame.size() > 61){
            coreFrame.erase(coreFrame.begin());
        }

        /// remove map events that nolonger have a corosponding coreFrame event
        for(auto& [key,eventList] : events ){
            while(eventList.size() > 0 && eventList.back().frameNum < coreFrame.begin()->first)
                eventList.pop_back();
        }
    }

    /// Create an event and add it at first time in list
    void Profiler::start(std::string timerID)
    {
        /// if this list is empty or if the last timer has been closed properly we create new timer.
        /// this is to handle calls in recursive functions and treat them as a single event until fully closed.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            events[timerID].push_front(Event(timerID,coreFrame.rbegin()->second.frameNum));
            return;
        }
        /// otherwise increment open Counters on this timmer.
        events[timerID].front().openCount++ ;

    }

    /// Update the end time of the front item.
    std::chrono::_V2::steady_clock::duration Profiler::stop(std::string timerID)
    {
        /// if the event list is empty, or the prior timer is completed we do not want to access or overwright stopT so return 0.
        if(events[timerID].size() == 0 || events[timerID].front().openCount == 0){
            return std::chrono::_V2::steady_clock::duration {0};
        }
        /// record the current stop time, decriment open counter, return the passed time.
        events[timerID].front().stopT = std::chrono::_V2::steady_clock::now();
        events[timerID].front().openCount-- ;
        return events[timerID].front().passedTime();
    }

    void Profiler::drawDebug(olc::PixelGameEngine* game)
    {
        start("debug"); /// valuable to know how much time is lost to createing this display.

        /// Go through default event to gather total tracked history data
        float totalTimeTracked = std::chrono::duration<float>(std::next(coreFrame.rbegin())->second.stopT - coreFrame.begin()->second.startT ).count();

        if(coreFrame.size() <= 1)
            return; /// theres no closed frame events, nothing to do.

        int numFramesShown = 5;
        float timePerFrame = 1.0f/60.0f;  /// 60 fps
        int lastFrameNum = std::next(coreFrame.rbegin())->first;


        game->SetDrawTarget(nullptr); /// Draw to default layer above all others
        int lineHeight = 10;

        int wide = game->ScreenWidth();
        int height = game->ScreenHeight();

        float frameScale = (wide/numFramesShown) / timePerFrame;
        float displayStart = (wide/numFramesShown);


        int drawHeight = height - lineHeight;

        for(auto& [key,eventList] : events ){
            if(eventList.size() == 0){
                continue; /// no need to operate on empty lists
            }
            /// Check if a timer is being called an excessive number of times (more than 50 per frame).
            /// It will still be calculated for times over the last frame and adjusted, but draw is skipped
            bool skipExcessiveDraws = false;
            if(eventList.size() >= 50 * events[defaultName].size() )
                skipExcessiveDraws = true;

            /// iterate through this event drawing the time frames it occupied
            float eventTimeRun = 0;
            int eventsTracked = 0;
            for(Event& timer : eventList){
                if(timer.openCount > 0 || timer.frameNum > lastFrameNum)
                    continue; /// this timer event or cycle is not yet closed or has aged out and will be skipped.

                float length = std::chrono::duration<float>(timer.passedTime()).count();

                /// Accumulate our runtime data
                eventTimeRun += length;
                eventsTracked ++;

                if(skipExcessiveDraws){ /// skip drawing phase for excessive data
                    continue;
                }

                bool lastFrame = timer.frameNum == lastFrameNum; // ? colour = olc::GREEN : olc::DARK_YELLOW;

                float start = std::chrono::duration<float>(timer.startT - coreFrame.find(timer.frameNum)->second.startT ).count();

                int frameStart = (1 + (lastFrameNum - timer.frameNum)) * displayStart;
                int rectStart = start * frameScale + frameStart;
                int rectLong = length * frameScale;
                if (lastFrame)
                    game->FillRect(rectStart,drawHeight,rectLong,lineHeight,olc::GREEN);
                if (rectStart < wide)
                    game->DrawRect(rectStart,drawHeight,rectLong,lineHeight,olc::DARK_YELLOW);
            }
            float aveEvents = (float)eventsTracked / ((float)coreFrame.size() - 1);
            float percentOfFrame = (eventTimeRun / totalTimeTracked) * 100;
            /// Write our analytics about the bar on screen
            game->DrawString(lineHeight,drawHeight+1,key + " "+ std::to_string(aveEvents).substr(0,5),olc::YELLOW);

            std::string percentage = std::to_string(percentOfFrame);
            percentage = percentage.substr(0,percentage.find(".")+4);
            game->DrawString(displayStart - (percentage.size() * 8),drawHeight+1,percentage,olc::YELLOW);

            drawHeight -= lineHeight;
        }
        float percentOfFrame = totalTimeTracked / (coreFrame.size() - 1) / timePerFrame * 100;
        game->DrawString(lineHeight,drawHeight+1,"coreFrame "+ std::to_string(coreFrame.size() - 1).substr(0,5),olc::YELLOW);

        std::string percentage = std::to_string(percentOfFrame);
        percentage = percentage.substr(0,percentage.find(".")+4);
        game->DrawString(displayStart - (percentage.size() * 8),drawHeight+1,percentage,olc::YELLOW);

        for(int i = displayStart; i < wide;i += displayStart){
            game->DrawLine(i,drawHeight,i,height,olc::BLUE);
        }
    stop("debug");
    }
