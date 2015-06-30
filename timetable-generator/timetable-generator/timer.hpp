//
//  timer.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-29.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef timer_cpp
#define timer_cpp

#include <ctime>

typedef std::time_t TIME;

class timer {
    bool started;
    bool paused;
    TIME startTime;
    TIME pauseTime;
    TIME lapTime;
    TIME lastDiff;
    TIME pauseDiff;
    
public:
    timer();
    int start();
    int pause();
    int resume();
    int stop();
    int lap();
    TIME getLastDifference();
    TIME getTotalTime();
};

#endif /* timer_cpp */
