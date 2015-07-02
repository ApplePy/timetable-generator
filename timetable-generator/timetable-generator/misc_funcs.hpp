//
//  misc_funcs.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef misc_funcs_cpp
#define misc_funcs_cpp

#include <thread>
#include <vector>
#include "dist/json/json.h"

using std::thread;
using std::vector;
using Json::Value;


//DECLARATIONS
///Joins all threads in a thread-containing vector.
template <typename T>
void closeAllThreads (T* threadIndex) {
    assert(threadIndex != NULL);  //QUICK DEBUG CODE
    
    for (auto iter = threadIndex->begin(); iter != threadIndex->end(); iter++) {
        if (iter->joinable())
            iter->join();
    }
    threadIndex->clear(); //delete all items now that they've been joined
}

#endif /* misc_funcs_cpp */
