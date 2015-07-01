//
//  misc_funcs.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "misc_funcs.hpp"

void vectorReverse(vector<vector<Value*>*>& input) {
    auto left (input.begin());
    auto right (--input.end()); //start one before the end of the container
    while (left < right) {
        //Address swap
        auto temp = *right;
        *right = *left;
        *left = temp;
        --right;
        ++left;
    }
}
