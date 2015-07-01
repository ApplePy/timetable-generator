//
//  sorting_and_ranking_functions.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef sorting_and_ranking_functions_cpp
#define sorting_and_ranking_functions_cpp

#include <vector>
#include <list>
#include "misc_funcs.hpp"
#include "dist/json/json.h"

using std::vector;
using Json::Value;


//EXTERNAL VARAIBLES
extern std::mutex globalMutex;
extern std::list<void *> dynamicAllocations; //TOTAL KLUDGE! USED AS PAPERING OVER BAD DESIGN! FIX!


//DECLARATIONS
///"rankTimetable" helper function
long long partition(vector<vector<Value*>*>* alistPtr, unsigned long first, unsigned long last);

///"rankTimetable" helper function
void quickSortHelper(vector<vector<Value*>*>* alist, unsigned long first, unsigned long last);

///"rankTimetable" helper function
void modified_quickSort(vector<vector<Value*>*>* alist);

///Takes a an array of timetables and sorts it according to its ranking
void rankTimetable (vector<vector<Value*>*>* const inputPtr);

#endif /* sorting_and_ranking_functions_cpp */
