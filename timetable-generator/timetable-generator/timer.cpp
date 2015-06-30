//
//  timer.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-29.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "timer.hpp"


timer::timer() {
    started = false;
    paused = false;
    lastDiff = 0;
    pauseDiff = 0;
    lapTime = startTime;
    pauseTime = startTime;
}

int timer::start() {
    if (!started && !paused) {
        started = true;
        startTime = std::time(nullptr);
        pauseDiff = 0;
        lapTime = startTime;
        pauseTime = 0;
        return 0;
    }
    else {
        return -1;
    }
}

int timer::lap() {
    if (!started) {
        return -1;
    }
    else if (paused) {
        return -2;
    }
    else {
        TIME temp = std::time(nullptr);
        
        lastDiff = temp - lapTime - pauseDiff;
        lapTime = temp;
        pauseDiff = 0;
        
        return 0;
    }
}

int timer::pause() {
    if (!started) {
        return -1;
    }
    else if (paused) {
        return -2;
    }
    else {
        paused = true;
        pauseTime = std::time(nullptr);
        return 0;
    }
}

int timer::resume() {
    if (!started) {
        return -1;
    }
    else if (!paused) {
        return -2;
    }
    else {
        paused = false;
        pauseDiff += std::time(nullptr) - pauseTime;
        return 0;
    }
}

int timer::stop() {
    if (!started) {
        return -1;
    }
    else if (paused) {
        return -2;
    }
    else {
        lap();
        started = false;
        return 0;
    }
}

TIME timer::getLastDifference() {
    return lastDiff;
}

TIME timer::getTotalTime() {
    if (started) {
        return std::time(nullptr) - startTime;
    }
    else {
        return lapTime - startTime;
    }
}