//
//  weighting.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef weighting_cpp
#define weighting_cpp

#include <vector>
#include <string>
#include <cmath>
#include "dist/json/json.h"
#include "math_funcs.hpp"

#include <assert.h>

using std::vector;
using std::string;
using std::pow;
using Json::Value;


//EXTERNAL VARAIBLES
extern std::mutex globalMutex;
extern const std::vector<std::string> componentsList;


//DECLARATIONS
/**"weight_course" helper function
 
 Covers weighting overlap into lunch time, distance from 8:30am, and weights longer courses heavier since they're more difficult to move.
 */
double weight_time(const int startTime, const int endTime);

///"weight_course" helper function
void weight_component(Value& course, const string selector);

/**
 Weight design:
 
 Each course's component starts off at a 5000 weighting
 
 Each course component's weight gets subtracted by the formula x^1.2 where x is the number of minutes from 8:30am
 
 A course component that goes into the 12:00-1:00pm block is subtracted 800 weighting points
 
 A course component gets 300/60 * x added to it's weighting, where x is number of minutes the component consumes
 
 The final weighting is subtracted by log_10(x)*1000, where x is the number of sections the course has
 
 If a component section has multiple classes, weight each one, then find the mean
 
 If a class is taught by Quazi Rahman, it gets added a 5000 weighting
 */
void weight_course(vector<Value>::iterator coursePtr);

#endif /* weighting_cpp */
