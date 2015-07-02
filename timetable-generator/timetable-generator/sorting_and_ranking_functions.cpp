//
//  sorting_and_ranking_functions.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "sorting_and_ranking_functions.hpp"


//FUNCTIONS
long long partition(vector<vector<Value*>*>* alistPtr, unsigned long first, unsigned long last) {
    unsigned long pivotvalue = alistPtr->at(first)->at(alistPtr->at(first)->size() - 1)->asDouble();
    auto leftmark = first+1;
    auto rightmark = last;
    vector<vector<Value*>*>& alist = *alistPtr;
    
    bool done = false;
    while (!done) {
        
        while (leftmark <= rightmark && alist[leftmark]->at(alist[leftmark]->size()-1)->asDouble() <= pivotvalue)
            leftmark = leftmark + 1;
        
        while (alist[rightmark]->at(alist[rightmark]->size()-1)->asDouble() >= pivotvalue && rightmark >= leftmark)
            rightmark = rightmark -1;
        
        if (rightmark < leftmark)
            done = true;
        else {
            // Do some address swapping (faster than swapping vector contents)
            auto temp = alist[leftmark];
            alist[leftmark] = alist[rightmark];
            alist[rightmark] = temp;
            //alist[leftmark].swap(alist[rightmark]);
        }
    }
    
    // Do some address swapping (faster than swapping vector contents)
    auto temp = alist[leftmark];
    alist[leftmark] = alist[rightmark];
    alist[rightmark] = temp;
    
    
    return rightmark;
}

void quickSortHelper(vector<vector<Value*>*>* alist, unsigned long first, unsigned long last) {
    if (first<last) {
        
        auto splitpoint = partition(alist,first,last);
        
        quickSortHelper(alist,first,splitpoint-1);
        quickSortHelper(alist,splitpoint+1,last);
    }
}

void modified_quickSort(vector<vector<Value*>*>* alist) {
    quickSortHelper(alist, 0, alist->size()-1);
}

void rankTimetable (vector<vector<Value*>*>* const inputPtr) {
    for (auto candidatePtr = inputPtr->begin(); candidatePtr != inputPtr->end(); candidatePtr++) {
        vector<Value*>& candidate = **candidatePtr;
        
        double totalWeight = 0;
        for (auto coursePtr = candidate.begin(); coursePtr != candidate.end(); coursePtr++) {
            Value& course = **coursePtr;
            
            double cWeight = 0;
            unsigned int counter = 0;
            if (course.isMember("c")) {
                cWeight += course["c"][0]["sw"].asInt();
                counter += 1;
            }
            if (course.isMember("tu")) {
                cWeight += course["tu"][0]["sw"].asInt();
                counter += 1;
            }
            if (course.isMember("l")) {
                cWeight += course["l"][0]["sw"].asInt();
                counter += 1;
            }
            course["cw"] = cWeight / counter;
            totalWeight += cWeight / counter;
        }
        auto newDynamicAddress = new Value(totalWeight / candidate.size());
        globalMutex.lock();
        dynamicAllocations.push_back(newDynamicAddress);
        globalMutex.unlock();
        candidate.push_back(newDynamicAddress);
    }
    modified_quickSort(inputPtr);
    
    reverse(inputPtr->begin(), inputPtr->end());
    //Find aggregate weighting for each course, then each possibility
    //sort possibilities by weighting, high to low
    
}