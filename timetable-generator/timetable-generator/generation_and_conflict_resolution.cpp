//
//  generation_and_conflict_resolution.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "generation_and_conflict_resolution.hpp"


//FUNCTIONS
void GCVSetup (vector<vector<Value>>::iterator outputVector, vector<Value>::const_iterator inputValue) {
    /* A high-level explanation for the mess below:
     The following code creates a variable length array that points to each of the key names listed in the "components" array. It then ensures that every pointer points to a valid key in the class object, discarding the invalid pointers. The data is then fed into a recursive function that works as a variable-depth nested for loop to append the course variations to the given array
     */
    
    vector<Value>& outputClassVariations = *(outputVector);
    vector<vector<string>::const_iterator> componentIters;
    
    //Initialize componentIters, ensuring they're valid
    for (auto iter = componentsList.cbegin(); iter != componentsList.cend(); iter++) {
        if (inputValue->isMember(*iter)) {
            componentIters.push_back(iter);
        }
    }
    
    vector<unsigned short> inputArrayPositions (componentIters.size(), 0);
    generateClassVariations(componentIters, 0, &inputArrayPositions, outputClassVariations, *inputValue);
}

void generateClassVariations(const vector<vector<string>::const_iterator>& componentIterators, const unsigned int counter, vector<unsigned short>* inputArrayPositions, vector<Value>& output, const Value& input) {
    //If this is the end of the nested function structure, copy the input Value to a new spot in output; then using the position numbers from inputArrayPosition, overwrite the components indicated in componentIters by the entry in Input indicated by inputPosition, else just pass it farther down the chain.
    for (int i = 0; i < input[*componentIterators[counter]].size(); i++) {
        inputArrayPositions->at(counter) = i;
        if (counter == componentIterators.size() - 1) {
            output.push_back(input);
            for (int n = 0; n < componentIterators.size(); n++) {
                Value temp (Json::arrayValue);
                
                temp[0] = input[*componentIterators[n]][inputArrayPositions->at(n)];
                output[output.size() - 1][*componentIterators[n]] = temp;
            }
        }
        else
            generateClassVariations(componentIterators, counter + 1, inputArrayPositions, output, input);
    }
}

void generateCombinations(vector<vector<Value>>* const input, vector<vector<Value*>>* const output, unsigned int counter) {
    
    // ****** THIS FUNCTION IS VULNERABLE TO REALLOCATIONS ****** ... NOT ANYMORE
    
    // If this is the end of the input vector, wrap in a vector and copy the contents directly into output
    if (counter == input->size() - 1) {
        for (auto iter = input->at(counter).begin(); iter != input->at(counter).end(); iter++) {
            output->push_back(vector<Value*>());
            (output->end() - 1)->push_back(&*iter);
        }
    }
    //If this is not the end, pass it farther down, duplicate the existing entries as necessary, then append the appropriate additions on
    else {
        generateCombinations(input, output, counter + 1);
        //auto endIter = output->end(); //holds the end pointer for the original output so that the copier knows where to stop reading its source material
        auto sz = output->size();
        
        // Iterate through the input container's list of course variations, and duplicate the output vector as necessary to append this class
        for (auto i = input->at(counter).begin(); i != input->at(counter).end(); i++) {
            for (auto n = 0; n < sz; n++) {
                output->push_back(output->at(n)); //the switch from iterators to index numbers escapes the reallocation dangers - but makes output incapable of being switched to a list
            }
        }
        
        //Append classes
        unsigned int offset = 0;
        for (auto i = input->at(counter).begin(); i != input->at(counter).end(); i++) {
            for (auto n = offset; n != sz + offset && n != output->size(); n++) {
                output->at(n).push_back(&*i);
            }
            offset += sz;
        }
    }
    
    //Total kludge to avoid having to fix the mistake in the algorithm
    for (auto n = --output->end(); n != --output->begin(); n--) {
        if (n->size() + counter != input->size())
            n = output->erase(n);
    }
    
}

void testForConflict (const vector<Value*>* const input, vector<bool>::iterator resultOut) {
    // NOTE: For some reason the referencing of a bool value in a vector returns an iterator rather than an address... hence the bool iterator rather than 'bool* const'
    
    // ****** THIS FUNCTION IS VULNERABLE TO REALLOCATIONS ******
    
    auto empty = vector<int>(2, 5);
    unordered_map<string, vector<int>> setsOfTimes [2] = {
        {
            {"mo", empty},
            {"tu", empty},
            {"we", empty},
            {"th", empty},
            {"fr", empty}
        }
    };
    
    //Iterate through the courses
    for (auto coursePtr = input->begin(); coursePtr != input->end(); coursePtr++) {
        
        // NOTE: THE FOLLOWING SECTION WAS COPIED FROM ELSEWHERE! TRY TO ENCAPSULATE INTO A FUNCTION!
        vector<vector<string>::const_iterator> componentIters;
        Value& course = **coursePtr;
        
        //Initialize componentIters, ensuring they're valid
        for (auto iter = componentsList.begin(); iter != componentsList.end(); iter++) {
            if (course.isMember(*iter)) {
                componentIters.push_back(iter);
            }
        }
        // END NOTE
        
        //Iterate through the course's times, create the time set, compare it with setsOfTimes, and append if no intersection, quit otherwise
        for (auto componentPtr = componentIters.begin(); componentPtr != componentIters.end(); componentPtr++) {
            const string& component = **componentPtr;
            
            for (auto timePtr = course[component][0]["ti"].begin(); timePtr != course[component][0]["ti"].end(); timePtr++) {
                const Value& time = *timePtr;
                assert(time.isArray()); // QUICK DEBUG CODE
                // NOTE: THE FOLLOWING SECTION WAS COPIED AND MODIFIED FROM ELSEWHERE! TRY TO ENCAPSULATE INTO A FUNCTION!
                auto proposedTime = range(time_conversion(time[2].asInt(), time[3].asInt()), time_conversion(time[4].asInt(), time[5].asInt()), 10); //Note: this will cause a collision if two classes are next to each other!
                assert (proposedTime.size() > 0);
                vector<int> result [2];
                result[0].reserve((time_conversion(23, 30)-time_conversion(8, 30))/10);
                result[1].reserve((time_conversion(23, 30)-time_conversion(8, 30))/10);
                
                // NOTE: CODE COPIED AND PASTED! REFACTOR!
                switch (time[0].asInt()) {
                    case 3:
                    case 1: {
                        //assert (false);
                        //globalMutex.lock();
                        //cout << quoteStripper(time[1].asString()) << endl;
                        //globalMutex.unlock();
                        vector<int>::const_iterator end = set_intersection(proposedTime.begin(), proposedTime.end(), setsOfTimes[0][quoteStripper(time[1].asString())].begin(), setsOfTimes[0][quoteStripper(time[1].asString())].end(), result[0].begin());
                        result[0].resize(end - result[0].begin()); //shink result vector to necessary size only
                        if ( time[0].asInt() == 1) //Allows for 3 to fall through
                            break;
                    }
                    case 2: {
                        vector<int>::const_iterator end = set_intersection(proposedTime.begin(),proposedTime.end(), setsOfTimes[1][quoteStripper(time[1].asString())].begin(), setsOfTimes[1][quoteStripper(time[1].asString())].end(), result[1].begin());
                        result[1].resize(end - result[1].begin()); //shink result vector to necessary size only
                        break;
                    }
                    default:
                        throw ("JSON structure corrupted! Course " + course["n"].asString() + " has a bad term condition!");
                        break;
                }
                
                //If conflict detected, signal conflict and quit function, else append to map
                if (result[0].size() > 0 || result[1].size() > 0) {
                    // END NOTE
                    *resultOut = true;
                    return;
                }
                else {
                    switch (time[0].asInt()) {
                        case 3:
                        case 1:
                            setsOfTimes[0][quoteStripper(time[1].asString())].insert(setsOfTimes[0][quoteStripper(time[1].asString())].end(), proposedTime.begin(), proposedTime.end()); //Concatenate the proposed time and the canonical
                            sort(setsOfTimes[0][quoteStripper(time[1].asString())].begin(), setsOfTimes[0][quoteStripper(time[1].asString())].end()); //Sort the canonical time so that set_intersection works properly
                            if ( time[0].asInt() == 1) //Allows for 3 to fall through
                                break;
                        case 2:
                            setsOfTimes[1][quoteStripper(time[1].asString())].insert(setsOfTimes[1][quoteStripper(time[1].asString())].end(), proposedTime.begin(), proposedTime.end()); //Concatenate the proposed time and the canonical
                            sort(setsOfTimes[1][quoteStripper(time[1].asString())].begin(), setsOfTimes[1][quoteStripper(time[1].asString())].end()); //Sort the canonical time so that set_intersection works properly
                            break;
                        default:
                            throw ("JSON structure corrupted! Course " + course["n"].asString() + " has a bad term condition!");
                            break;
                    }
                }
            }
        }
    }
}

void TFCSetup(const vector<vector<Value*>>* const input, vector<bool>::iterator resultOut, vector<vector<Value*>>::const_iterator start, vector<vector<Value*>>::const_iterator end) {
    
    for (auto i = start; i != end; i++, resultOut++) {
        testForConflict(&*i, resultOut);
    }
    
}