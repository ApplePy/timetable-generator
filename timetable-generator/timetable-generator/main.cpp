//
//  main.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-19.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
//#include <list>
//#include <set>
#include <unordered_map>
#include <cmath>
#include <thread>
#include <mutex>
#include <assert.h>
#include "dist/json/json.h"

using namespace std;
using namespace Json;



// GLOBAL FUNCTIONS

mutex globalMutex;
const vector<string> components = {"c","tu", "l"}; //Holds the class components of a course



// FUNCTIONS

///Lists the contents of a Json::Value, returns the number of items listed.
int list_object (const Json::Value& list, const bool withNumbers = false, const bool listKeys = false, vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false) {
    
    // NOTE: listing the keys of an array of objects makes no sense.
    
    Value::iterator listIterator;
    vector <string> keyList;
    int counter = 0;

    if (list.isObject())
        keyList = list.getMemberNames();
    
    // Iterate through the contents of a JSON Value
    for (listIterator = list.begin(), counter = 0; listIterator != list.end(); counter++, listIterator++) {
        string extra;
        
        //Add numbers, and pad with spaces as necessary
        if (withNumbers) {
            extra = to_string(counter + 1) + ":";
            while (extra.length() < 7)
                extra += " ";
        }
        
        if (listKeys == true && list.isObject()) //it's an object and listKeys is listed, iterator lists the keys
            cout << extra << keyList[counter] << endl;
        else if (!keys.empty() && list.isObject()) { //its an object/map, the keys aren't to be listed, and there are specific key(s) that want to be listed, iterator lists key(s) contents on a single line
            bool found = false;
            
            //Search the keys in the object for matches with the keys desired, indicate if search is successful
            for (int i = 0; i < keyList.size() && !found; i++) {
                if (keyList[counter] == keys[i])
                    found = true;
            }
            
            // if a match was found, write the key's contents to extra and send extra to output, else decrement counter so that the next object is logically numbered on the output
            if (found) {
                extra += list.get(keys.back(), Json::Value()).asString() + " ";
                cout << extra << endl;
            }
            else
                --counter;
        }
        else if (!keys.empty() && list.isArray() && list.size() > 0 && list[0].isObject() && displayObjectKeysInsideArray) { //lists keys in object inside array, iterator goes through objects - not keys
            for (int i = 0; i < keys.size(); i++)
                extra += list[counter].get(keys[i], Json::Value()).asString() + " ";
            cout << extra << endl;
        }
        else //If all else fails, just spit the raw contents of the json value out to screen
            cout << extra << *listIterator << endl;
    }
    
    return counter;
}

/**Combines the listing of a Json::value with asking what option they want to choose, with the appropriate error checking.
 
 Returns: 0, 1, 2
 
 0 if successful.
 
 1 if the user signalled to exit without making a choice.
 
 2 if an unexpected error has occurred.
 */
int list_and_get(const Json::Value& list, const bool withNumbers = false, string * const retVal = NULL, const bool listKeys = false, const vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false) {
    
        list_object(list, withNumbers, listKeys, keys, displayObjectKeysInsideArray);
    
    if (retVal != NULL) {
        int ret = -1;
        cout << endl << "Select an option from the list above (press 0 to exit): ";
        cin >> ret;
        while (ret > list.size()) {
            cout << "Invalid option!" << endl;
            cout << "Select an option from the list above (press 0 to exit): ";
            cin >> ret;
        }
        
        //the user has signaled to exit
        if (ret == 0)
            return 1;
        
        //Increment the iterator to the desired position
        Value::iterator iterator = list.begin();
        for (int i = 0; i < ret - 1; i++)
            iterator++;
        
        //Figure out how to return the object appropriately.
        if (list.isObject())
            *retVal = iterator->asString();
        else if (list.isArray())
            *retVal = to_string(ret - 1);
        else
            return 2;
        
        return 0;
    }
    else
        return 0;
}

///Creates a vector containing the range of numbers between begin and end, inclusive, by a set interval between numbers
vector<int> range(const int begin, const int end, const int interval = 1) {
    vector<int> result;
    
    for (int i = begin; i <= end; i += interval) {
        result.push_back(i);
    }

    //sort(result.begin(), result.end()); //result should be sorted already by design
    return result;
}

/* UNUSED FUNCTIONS
double iroot(double k, double n) {  // Finds the nth root of a number, where n is the number, and k is the root
    double u (n), s (n+1);
    while (u < s) {
        s = u;
        double t = floor((k-1) * s + n / pow(s, k-1));
        u = floor(t / k);
    }
    return s;
}

double geometric_mean(const vector<double>& listOfWeightings) {
    double total = 1;
    for (auto weightIterator = listOfWeightings.begin(); weightIterator != listOfWeightings.end(); weightIterator++)
        total += *weightIterator;
    return iroot(listOfWeightings.size(), total);
}
 */

double mean(const vector<double>& listOfWeightings) {
    double total = 0;
    
    for (auto iter = listOfWeightings.begin(); iter != listOfWeightings.end(); iter++)
        total+= *iter;
    
    return (total/(listOfWeightings.end()-listOfWeightings.begin()));
}

///This function converts regular times into a minutes-from-8:30am integer
int time_conversion(const int hour, const int minute) {
    return (hour - 8) * 60 + (minute - 30);
}

///"weight_course" helper function
double weight_time(const int startTime, const int endTime) {
    double weight = 1;  // Since starting weight is determined by an upper function
    const vector<int> lunchSet = range(time_conversion(12, 01), time_conversion(12, 59)); //Times changed so that courses that start at 12 or 1 aren't incorrectly penalized
    const vector<int> classSet = range(startTime, endTime);
    vector<int> result ((lunchSet.size() > classSet.size()) ? lunchSet.size() : classSet.size()); //make result the size of the bigger set
    
    vector<int>::const_iterator end = set_intersection(lunchSet.begin(),lunchSet.end(), classSet.begin(), classSet.end(), result.begin());
    result.resize(end - result.begin()); //shink result vector to necessary size only
    
    if (result.size() > 0)
        weight -= 800;
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
        Value& times = (*sectIterator)["ti"];
        
        // ...and for each time slot taken in this section's course component...
        for (auto timeSlot = times.begin(); timeSlot != times.end(); timeSlot++) {
            // calculate the weighting of each time slot, and insert into the weightList
            weightList.push_back(5000 + weight_time(time_conversion((*timeSlot)[2].asInt(), (*timeSlot)[3].asInt()),time_conversion((*timeSlot)[4].asInt(), (*timeSlot)[5].asInt())));
        }
        auto sectionWeight = mean(weightList);  // the section's final weight: geometric mean of the weight of each of its time slots
        sectionWeight -= (log10(component.end() - component.begin()) * 1000);  // decrease the section weight by the number of sections
        
        //If the teacher of the section is Quazi, add 5000 to the weighting
        for (auto teacherIter = (*sectIterator)["sups"].begin(); teacherIter != (*sectIterator)["sups"].end(); teacherIter++) {
            if (teacherIter->asString() == string("Rahman")) {
                sectionWeight += 5000;
                break;
            }
        }
        
        //Write the calculated weight to the section
        globalMutex.lock();
        (*sectIterator)["sw"] = sectionWeight;  // tag each section with it's weight
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
    for (auto i = components.begin(); i != components.end(); i++)
        if (coursePtr->isMember(*i))
            weight_component(*coursePtr, *i);
}

///Joins all threads in a thread-containing vector.
void closeAllThreads (vector<thread>* globalThreadIndex) {
    assert(globalThreadIndex != NULL);  //QUICK DEBUG CODE
    
    for (auto iter = globalThreadIndex->end() - 1; !globalThreadIndex->empty(); iter--) {
        if (iter->joinable())
            iter->join();
        globalThreadIndex->pop_back();
    }
    globalThreadIndex->shrink_to_fit(); //resizes the thread array to no longer take up the space it previously consumed on the heap.
}

///Generates the permutations of classes, lectures, and tutorials in a given course.
void generateClassVariations(const vector<vector<string>::const_iterator>& componentIters, const unsigned int counter, vector<unsigned short>* inputArrayPosition, vector<Value>& output, const Value& input) {
    //If this is the end of the nested function structure, copy the input Value to a new spot in output; then using the position numbers from inputArrayPosition, overwrite the components indicated in componentIters by the entry in Input indicated by inputPosition, else just pass it farther down the chain.
    for (int i = 0; i < input[*componentIters[counter]].size(); i++) {
        inputArrayPosition->at(counter) = i;
        if (counter == componentIters.size() - 1) {
            output.push_back(input);
            for (int n = 0; n < componentIters.size(); n++) {
                Value temp (arrayValue);
                
                temp[0] = input[*componentIters[n]][inputArrayPosition->at(n)];
                output[output.size() - 1][*componentIters[n]] = temp;
                
                //NOTE: some inefficiency in here (copying to a temporary variable only to copy it again), something to change later
            }
        }
        else
            generateClassVariations(componentIters, counter + 1, inputArrayPosition, output, input);
    }
    
    //When this function is done, the inputArrayPosition vector that was dynamically allocated is no longer needed. Delete it to prevent a memory leak.
    if (counter == 0)
        delete inputArrayPosition;
}

///Sets up for the generateClassVariations function, used primarily for multithreading
void GCVSetup (vector<vector<Value>>::iterator configIter, vector<Value>::const_iterator classIterator) {
    vector<Value>& courseVariations = *(configIter);
    
    /* A high-level explanation for the mess below:
     The following code creates a variable length array that points to each of the key names listed in the "components" array. It then ensures that every pointer points to a valid key in the class object, discarding the invalid pointers. The data is then fed into a recursive function that works as a variable-depth nested for loop to append the course variations to the given array
     */
    vector<vector<string>::const_iterator> componentIters;
    
    //Initialize componentIters, ensuring they're valid
    for (auto iter = components.begin(); iter != components.end(); iter++) {
        if (classIterator->isMember(*iter)) {
            componentIters.push_back(iter);
        }
    }
    
    generateClassVariations(componentIters, 0, new vector<unsigned short>  (componentIters.size(), 0), courseVariations, *classIterator); // NOTE: DYNAMIC ALLOCATION HERE
}

/**
 Creates every possible combination out of the data from input
 
 NOTE: FIND A WAY TO MAKE THIS ALGORITHM MULTITHREADED-FRIENDLY AND NOT VULNERABLE TO REALLOCATION!
 */
void generateCombinations(vector<vector<Value>>* const input, vector<vector<Value*>>* const output, unsigned int counter = 0) {
    
    // ****** THIS FUNCTION IS VULNERABLE TO REALLOCATIONS ******
    
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
        auto endIter = output->end(); //holds the end pointer for the original output so that the copier knows where to stop reading its source material
        auto sz = output->size();
        
        // Iterate through the input container's list of course variations, and duplicate the output vector as necessary to append this class
        for (auto i = input->at(counter).begin(); i != input->at(counter).end(); i++) {
            for (auto n = output->begin(); n != endIter; n++) {
                output->push_back(*n);
            }
        }
        
        //Append classes
        unsigned int offset = 0;
        for (auto i = input->at(counter).begin(); i != input->at(counter).end(); i++) {
            for (auto n = output->begin() + offset; n != endIter + offset && n != output->end(); n++) {
                n->push_back(&*i);
            }
            offset += sz;
        }
    }
    
    
    
    /*
    vector<Value> newList;
    vector<Value> existingList;
    if (input->size() > 1) {
        for (item in inputData[1:len(inputData)])
            newList.append(item);
        existingList = generate_combinations(newList);
    }
    // Take the returned list of combinations of previous courses, and add in new versions with this course in it
    vector <Value> newGenList;
    unsigned int counter = 0;
    for (version in inputData[0]) {
        
        if (len(existingList) == 0) {
            newGenList.append([version]);
        }
        else {
            for (item in existingList) {
                newGenList.append(deepcopy(item));
            }
            for (existVer in existingList) {
                newGenList[counter].append(version);
                counter += 1;
            }
        }
    }
    return newGenList;
     */
}

///Takes a schedule array, and figures out if it contains a conflict
void testForConflict (const vector<Value*>* const input, vector<bool>::iterator resultOut) {
    // NOTE: For some reason the referencing of a bool value in a vector returns an iterator rather than an address... hence the bool iterator rather than 'bool* const'
    
    auto empty = vector<int>();
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
        for (auto iter = components.begin(); iter != components.end(); iter++) {
            if (course.isMember(*iter)) {
                componentIters.push_back(iter);
            }
        }
        // END NOTE
        
        //Iterate through the course's times, create the time set, compare it with setsOfTimes, and append if no intersection, quit otherwise
        for (auto componentPtr = componentIters.begin(); componentPtr != componentIters.end(); componentPtr++) {
            const string& component = **componentPtr;
            
            for (auto timePtr = course[component][0]["ti"].begin(); timePtr != course[component][0]["ti"].end(); timePtr++) {
                Value& time = *timePtr;
                assert(time.isArray()); // QUICK DEBUG CODE
                // NOTE: THE FOLLOWING SECTION WAS COPIED AND MODIFIED FROM ELSEWHERE! TRY TO ENCAPSULATE INTO A FUNCTION!
                auto proposedTime = range(time_conversion(time[2].asInt(), time[3].asInt()), time_conversion(time[4].asInt(), time[5].asInt()));
                vector<int> result [2];
                
                // NOTE: CODE COPIED AND PASTED! REFACTOR!
                switch (time[0].asInt()) {
                    case 3:
                    case 1: {
                        assert(false); //The line below has an invalid pointer issue, investigate.
                        vector<int>::const_iterator end = set_intersection(proposedTime.begin(),proposedTime.end(), setsOfTimes[0][time[1].asString()].begin(), setsOfTimes[0][time[1].asString()].end(), result[0].begin());
                        result[0].resize(end - result[0].begin()); //shink result vector to necessary size only
                        if ( time[0].asInt() == 1) //Allows for 3 to fall through
                            break;
                    }
                    case 2: {
                        vector<int>::const_iterator end = set_intersection(proposedTime.begin(),proposedTime.end(), setsOfTimes[1][time[1].asString()].begin(), setsOfTimes[1][time[1].asString()].end(), result[1].begin());
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
                            setsOfTimes[0][time[1].asString()].insert(setsOfTimes[0][time[1].asString()].end(), proposedTime.begin(), proposedTime.end()); //Concatenate the proposed time and the canonical
                            sort(setsOfTimes[0][time[1].asString()].begin(), setsOfTimes[0][time[1].asString()].end()); //Sort the canonical time so that set_intersection works properly
                            if ( time[0].asInt() == 1) //Allows for 3 to fall through
                                break;
                        case 2:
                            setsOfTimes[1][time[1].asString()].insert(setsOfTimes[1][time[1].asString()].end(), proposedTime.begin(), proposedTime.end()); //Concatenate the proposed time and the canonical
                            sort(setsOfTimes[1][time[1].asString()].begin(), setsOfTimes[1][time[1].asString()].end()); //Sort the canonical time so that set_intersection works properly
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





/* NOTES TO DO:
 CLEAN UP MAIN!
 OPTIMIZE VECTOR MEMORY USAGE!
 LOOK INTO USING THE HEAP WHERE APPROPRIATE!
 FIX CONST AND VECTORS/NESTED VECTORS!
 */







int main(int argc, const char * argv[]) {
    
    vector<thread> globalThreadIndex;
    
    // Make an HTTP request to http://www.timetablegenerator.com/sched.php?data="western" to get JSON
    
    fstream file ("/Users/darrylmurray/Desktop/timetable-generator/data.json", ios::in);
    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        
        Json::Value root;
        Json::CharReaderBuilder rbuilder;
        rbuilder["collectComments"] = false;
        std::string errs;
        bool ok = Json::parseFromStream(rbuilder, buffer, &root, &errs);
        
        if (!ok) {
            cout << "Failed to parse JSON string! Errors: " << errs << endl;
            return 2;
        }
        
        int intRet = 0;
        vector<Value> classes;
        
        while (intRet == 0) {
            string departmentToGet(""), courseToGet("");
            intRet = list_and_get(root["departments"], true, &departmentToGet, true);
            if (intRet == 1) break;
            intRet = list_and_get(root["courses"][departmentToGet], true, &courseToGet, false, vector<string>({"cod", "n"}), true);
            if (intRet == 1) break;
            classes.push_back(root["courses"][departmentToGet][stoi(courseToGet)]); //add selected course to vector
        }
        
        for (auto iterator = classes.begin(); iterator != classes.end(); iterator++)
            globalThreadIndex.push_back(thread(weight_course,iterator));
        
        closeAllThreads(&globalThreadIndex);

        //Now create array of possible configurations for each course, and reserve enough space for each
        vector<vector<Value>> courseConfigurations (classes.size(), vector<Value>());
        auto configIter = courseConfigurations.begin();
        for (int i = 0; i < courseConfigurations.size(); i++)
            courseConfigurations[i].reserve(100);
        
        
        //Iterate through each course, generating the variations
        for (vector<Value>::const_iterator iterator = classes.begin(); iterator != classes.end(); iterator++, configIter++) {
            globalThreadIndex.push_back(thread(GCVSetup,configIter, iterator));
        }
        
        closeAllThreads(&globalThreadIndex);
        
        //Shrink all the class allocations down to their real size
        for (auto i = courseConfigurations.begin(); i != courseConfigurations.end(); i++) {
            i->shrink_to_fit();
        }
        
        vector<vector<Value*>> output; // Consider changing this to a list instead to escape reallocation hell
        output.reserve(1000000000); //Reserve space for one trillion combinations... may need more
        generateCombinations(&courseConfigurations, &output); //Find a way to multithread this
        
        vector<bool> conflictVector(output.size(), false); //Holds the list of timetable variations that are known to have conflicts
        for (auto i = 0; i < output.size(); i++)
            //testForConflict(&(output[i]), &(conflictVector[i]));
            globalThreadIndex.push_back(thread(testForConflict, &(output[i]), &(conflictVector[i])));
        
        closeAllThreads(&globalThreadIndex);
        
        //Discard all the schedule variations with conflicts
        for (auto conflict = conflictVector.end() - 1; conflict != conflictVector.begin() - 1; conflict--) {
            if (*conflict)
                conflictVector.erase(conflict);
        }
        
        
        cout << output[0].size() << " test " << output.size() << endl;
    }
    else {
        cout << "The JSON file could not be read!" << endl;
        return 1;
    }

    return 0;
}