//
//  weighting.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "weighting.hpp"

//FUNCTIONS
/**"weight_course" helper function
 
 Covers weighting overlap into lunch time, distance from 8:30am, and weights longer courses heavier since they're more difficult to move.
 */
double weight_time(const int startTime, const int endTime) {
    double weight = 1;  // Since starting weight is determined by an upper function
    const vector<int> lunchSet = range(time_conversion(12, 30), time_conversion(13, 30), 15); //Times changed so that courses that start at 12 or 1 aren't incorrectly penalized
    const vector<int> classSet = range(startTime, endTime, 15);
    vector<int> result ((lunchSet.size() > classSet.size()) ? lunchSet.size() : classSet.size(), 0); //make result the size of the bigger set
    
    vector<int>::const_iterator end = set_intersection(lunchSet.begin(),lunchSet.end(), classSet.begin(), classSet.end(), result.begin());
    result.resize(end - result.begin()); //shink result vector to necessary size only
    
    //if (result.size() > 0)
        //weight -= 1300;
    weight -= pow(startTime, 1.2);
    weight += (endTime - startTime) * 5;
    
    return weight;
}

///"weight_course" helper function
void weight_component(Value& course, const string selector) {
    assert(course[selector].isArray()); //QUICK DEBUG CODE
    
    Value& component = course[selector];
    
    //For each section in this course component...
    for (auto sectIterator = component.begin(); sectIterator != component.end(); sectIterator++) {
        vector<double> weightList;
        Value& section =  *sectIterator;
        Value& times = section["ti"];
        
        // ...and for each time slot taken in this section's course component...
        for (auto timeSlotPtr = times.begin(); timeSlotPtr != times.end(); timeSlotPtr++) {
            Value& timeSlot = *timeSlotPtr;
            auto beginTime = time_conversion(timeSlot[2].asInt(), timeSlot[3].asInt());
            auto endTime = time_conversion(timeSlot[4].asInt(), timeSlot[5].asInt());
            
            // calculate the weighting of each time slot, and insert into the weightList
            weightList.push_back(5000 + weight_time(beginTime, endTime));
        }
        
        auto sectionWeight = mean(weightList);  // the section's final weight: geometric mean of the weight of each of its time slots
        sectionWeight -= (log(component.size()) * 1000);  // decrease the section weight by the number of sections
        
        //If the teacher of the section is Quazi, add 5000 to the weighting
        for (auto teacherIter = section["sups"].begin(); teacherIter != section["sups"].end(); teacherIter++) {
            if (teacherIter->asString() == string("Rahman")) {
                sectionWeight += 5000;
                break;
            }
        }
        
        //Write the calculated weight to the section
        globalMutex.lock(); //This lock may be an overreaction, but being safe
        section["sw"] = sectionWeight;  // tag each section with it's weight
        globalMutex.unlock();
    }
}

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
void weight_course(vector<Value>::iterator coursePtr) {
    assert(coursePtr->isObject()); //QUICK DEBUG CODE
    
    // weight each lecture
    for (auto i = componentsList.begin(); i != componentsList.end(); i++)
        if (coursePtr->isMember(*i))
            weight_component(*coursePtr, *i);
}