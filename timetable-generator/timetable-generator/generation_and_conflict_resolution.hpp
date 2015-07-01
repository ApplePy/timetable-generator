//
//  generation_and_conflict_resolution.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef generation_and_conflict_resolution_cpp
#define generation_and_conflict_resolution_cpp

#include <vector>
#include <string>
#include <unordered_map>
#include "dist/json/json.h"
#include "json_manipulation.hpp"
#include "math_funcs.hpp"

#include <assert.h>

using std::vector;
using std::string;
using std::unordered_map;
using Json::Value;


//EXTERNAL VARIABLES
extern const std::vector<std::string> componentsList;


//DECLARATIONS
/**
 Sets up for the generateClassVariations function, used primarily for multithreading.
 
 Be sure to fully allocate the top-level vector before calling this function!
*/
void GCVSetup (vector<vector<Value>>::iterator configIter, vector<Value>::const_iterator classIterator);

///Generates the permutations of classes, lectures, and tutorials in a given course.
void generateClassVariations(const vector<vector<string>::const_iterator>& componentIters, const unsigned int counter, vector<unsigned short>* inputArrayPosition, vector<Value>& output, const Value& input);

/**
 Creates every possible combination out of the data from input
 
 NOTE: FIND A WAY TO MAKE THIS ALGORITHM MULTITHREADED-FRIENDLY AND NOT VULNERABLE TO REALLOCATION!
 */
void generateCombinations(vector<vector<Value>>* const input, vector<vector<Value*>>* const output, unsigned int counter = 0);

///Takes a schedule array, and figures out if it contains a conflict
void testForConflict (const vector<Value*>* const input, vector<bool>::iterator resultOut);

void TFCSetup(const vector<vector<Value*>>* const input, vector<bool>::iterator resultOut, vector<vector<Value*>>::const_iterator start, vector<vector<Value*>>::const_iterator end);

#endif /* generation_and_conflict_resolution_cpp */
