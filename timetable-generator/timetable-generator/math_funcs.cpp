//
//  math_funcs.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "math_funcs.hpp"


//FUNCTIONS
vector<int> range(const int begin, const int end, const int interval) {
    vector<int> result;
    
    for (int i = begin; i < end; i += interval) {
        result.push_back(i);
    }
    
    return result;
}

double mean(const vector<double>& listOfNumbers) {
    double total = 0;
    
    for (auto iter = listOfNumbers.begin(); iter != listOfNumbers.end(); iter++)
        total+= *iter;
    
    return (total/(listOfNumbers.end()-listOfNumbers.begin()));
}

int time_conversion(const int hour, const int minute) {
    return (hour - 8) * 60 + (minute - 30);
}
