//
//  math_funcs.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef math_funcs_cpp
#define math_funcs_cpp

#include <vector>

using  std::vector;


//DECLARATIONS
///Creates a vector containing the range of numbers of [begin, end), by a set interval between numbers
vector<int> range(const int begin, const int end, const int interval = 1);

///Returns the mean of a vector of numbers
double mean(const vector<double>& listOfNumbers);

///This function converts regular times into a minutes-from-8:30am integer
int time_conversion(const int hour, const int minute);

#endif /* math_funcs_cpp */
