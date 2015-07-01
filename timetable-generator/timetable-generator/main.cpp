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
#include <list>
//#include <set>
#include <thread>
#include <mutex>
#include <assert.h>
#include "dist/json/json.h"
#include "timer.hpp"
#include "json_manipulation.hpp"
#include "weighting.hpp"
#include "misc_funcs.hpp"
#include "generation_and_conflict_resolution.hpp"
#include "sorting_and_ranking_functions.hpp"

using namespace std;
using namespace Json;



// GLOBAL VARIABLES
timer globalTimer;
mutex globalMutex;
const vector<string> componentsList = {"c","tu", "l"}; //Holds the class components of a course
list <void *> dynamicAllocations; //TOTAL KLUDGE! USED AS PAPERING OVER BAD DESIGN! FIX!



int main(int argc, const char * argv[]) {
    globalTimer.start();
    list<thread> mainThreadIndex;
    
    
    
    // Make an HTTP request to http://www.timetablegenerator.com/sched.php?data="western" to get JSON
    
    
    //Open file
    fstream file("/Users/darrylmurray/Desktop/timetable-generator/data.json", ios::in);
    assert(file.is_open());
    stringstream buffer;
    buffer << file.rdbuf();
    
    
    
    //Convert file to JSON
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    std::string errs;
    bool ok = Json::parseFromStream(rbuilder, buffer, &root, &errs);
    
    if (!ok) {
        cout << "Failed to parse JSON string! Errors: " << errs << endl;
        return 2;
    }

    
    //Get classes
    vector<Value> classes;
    while (true) {
        string departmentToGet(""), courseToGet("");
        
        //Get department from user
        if (list_and_get(root["departments"], true, &departmentToGet, true) == 1) {
            break;
        }
        
        //Get course from user
        if (list_and_get(root["courses"][departmentToGet], true, &courseToGet, false, vector<string>({"cod", "n"}), true) == 1) {
            break;
        }
        
        //Add selected course to vector
        classes.push_back(root["courses"][departmentToGet][stoi(courseToGet)]);
    }
    
    
    
    
    assert (globalTimer.lap() == 0);
    
    
    
    
    
    //Weight the courses selected
    for (auto iterator = classes.begin(); iterator != classes.end(); iterator++) {
        mainThreadIndex.push_back( thread(weight_course,iterator) );
    }
    closeAllThreads(&mainThreadIndex);
    
    
    
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    
    
    
    //Now create array of possible configurations for each course, and reserve enough space for each
    vector<vector<Value>> courseConfigurations (classes.size(), vector<Value>());
    auto configIter = courseConfigurations.begin();
    
    
    
    //Iterate through each course, passing in the output destination pointer and the input source data pointer, generating the variations
    for (auto iterator = classes.cbegin(); iterator != classes.cend(); iterator++, configIter++) {
        mainThreadIndex.push_back( thread(GCVSetup, configIter, iterator) );
    }
    closeAllThreads(&mainThreadIndex);
    
    
    
    
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    

    
    
    vector<vector<Value*>> output; // Consider changing this to a list instead to escape reallocation hell
    output.reserve(1000000000); //Reserve space for one trillion combinations... may need more
    generateCombinations(&courseConfigurations, &output); //Find a way to multithread this
    
 //******************************************
 ///NOTE REMEMBER TO ADD IN A RANKING FUNCTION THAT PUTS CLASSES CLOSER TOGETHER, AND PRIORITIZES CLASSES YOU WANT TO SHARE WITH FRIENDS
    //*****************************************
    
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    
    
    vector<bool> conflictVector(output.size(), false); //Holds the list of timetable variations that are known to have conflicts
    
    // NOTE: Threads finish faster than they can be made! Make a set number of threads instead, and allocate them chunks to handle instead... or make a task scheduler that dynamically sends threads new blocks to process as they finish their assigned ones.
    //globalThreadIndex.reserve(std::thread::hardware_concurrency());
    for (auto i = 0; i < std::thread::hardware_concurrency(); i++)
    //testForConflict(&(output[i]), &(conflictVector[i]));
    mainThreadIndex.push_back(thread(TFCSetup,&output, conflictVector.begin() + i * output.size()/std::thread::hardware_concurrency(), output.begin() + i * output.size()/std::thread::hardware_concurrency(), output.begin() + (i+1) * output.size()/std::thread::hardware_concurrency()));
    
    closeAllThreads(&mainThreadIndex);
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    //Discard all the schedule variations with conflicts by creating a new vector without the conflicts
    vector<vector<Value*>*> cleanedOutput; // Consider changing this to a list instead to escape reallocation hell
    output.reserve(output.size());
    for (long long i = 0; i < conflictVector.size(); i++) {
        if (!conflictVector[i]) {
            //output.erase(output.begin() + i);
            cleanedOutput.push_back(&output[i]); //this could be threaded where each of the threads generates a piece of cleanedOutput, then concatenates them all together at the end.
        }
    }
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    rankTimetable(&cleanedOutput);
    
    assert (globalTimer.lap() == 0);
    cerr << globalTimer.getLastDifference() << endl;
    
    for (auto i = 0; i < 1; i++) {
        for (auto j = cleanedOutput[i]->begin(); j != cleanedOutput[i]->end(); j++) {
            list_object(**j);
        }
    }
    
    for (auto i = dynamicAllocations.begin(); i != dynamicAllocations.end(); i++) {
        delete *i;
    }
    
    
    cerr << "Intial size: " << output.size() << endl << "Final size: " << cleanedOutput.size() << endl;
    assert (globalTimer.stop() == 0);
    cerr << globalTimer.getTotalTime() << endl;
    
    return 0;
}