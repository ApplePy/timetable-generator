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
//#include <map>
#include <cmath>
#include <thread>
#include <mutex>
#include <assert.h>
#include "dist/json/json.h"

using namespace std;
using namespace Json;

mutex globalMutex;
vector<thread> globalThreadIndex;


//There's a problem where if the course code is missing an A or B, the placement of the name is shifted over one

int list_object (const Json::Value& list, const bool withNumbers = false, const bool listKeys = false, vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false) {
    
    /// Note, listing the keys of an array of objects makes no sense.
    
    Value::iterator listIterator;
    vector <string> keyList;
    if (list.isObject())
        keyList = list.getMemberNames();
    int counter = 0;
    
    
    for (listIterator = list.begin(), counter = 0; listIterator != list.end(); counter++, listIterator++) {
        
        string extra = "";
        
        if (withNumbers) {
            extra = to_string(counter + 1) + ":";
            while (extra.length() < 7)
                extra += " ";
        }
        
        if (listKeys == true && list.isObject()) //it's an object and listKeys is listed, iterator goes through keys
            cout << extra << keyList[counter] << endl;
        //else if (listKeys == false && displayObjectKeysInsideArray == false && keys.empty())
            //cout << extra << *listIterator << endl;
        else if (!keys.empty() && list.isObject()) { //iterator goes through keys
            bool found = false;
            
            for (int i = 0; i < keyList.size(); i++) {
                if (keyList[counter] == keys[i]) {
                    found = true;
                    break;
                }
            }
            
            // if a match was found, write to extra and output, else decrement counter so that the next object is logically numbered on the output
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
        else
            cout << extra << *listIterator << endl;
    }
    
    return counter;
}


int list_and_get(const Json::Value& list, const bool withNumbers = false, string * const retVal = NULL, const bool listKeys = false, const vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false) {
    
        list_object(list, withNumbers, listKeys, keys, displayObjectKeysInsideArray);
    
    if (retVal != NULL) {
        int ret = -1; //will overflow to the max int size
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
        
        Value::iterator iterator = list.begin();
        for (int i = 0; i < ret - 1; i++)
            iterator++;
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
    for (int i = begin; i <= end; i += interval){
        result.push_back(i);
    }
    //sort(result.begin(), result.end()); //result should be sorted already by design
    return result;
}

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

    const vector<int> lunchSet = range(time_conversion(12, 00), time_conversion(1, 00));
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
    Value& component = course[selector];
    assert(component.isArray());
    unsigned int count = component.end() - component.begin();  // counts the number of sections in this course's component
    Value::iterator sectIterator;
    
    
    for (sectIterator = component.begin(); sectIterator != component.end(); sectIterator++) {  // for each section in this course component
        vector<double> weightList;
        Value& times = (*sectIterator)["ti"];
        
        for (auto timeSlot = times.begin(); timeSlot != times.end(); timeSlot++) {  // and for each time slot taken in this section's course component
            weightList.push_back(5000 + weight_time(time_conversion((*timeSlot)[2].asInt(), (*timeSlot)[3].asInt()),time_conversion((*timeSlot)[4].asInt(), (*timeSlot)[5].asInt()))); // calculate the weighting of each time slot
        }
        auto sectionWeight = mean(weightList);  // the section's final weight: geometric mean of the weight of each of its time slots
        sectionWeight -= (log10(count) * 1000);  // decrease the section weight by the number of sections
        
        for (auto teacherIter = (*sectIterator)["sups"].begin(); teacherIter != (*sectIterator)["sups"].end(); teacherIter++) {
            if (teacherIter->asString() == string("Rahman")) {
                sectionWeight += 5000;
                break;
            }
        }
        
        
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
    Value& course = *coursePtr;
    assert(course.isObject());
    // weight each lecture
    if (course.isMember("c"))
        weight_component(course, "c");

    // weight each tutorial
    if (course.isMember("tu"))
        weight_component(course, "tu");

    // weight each lab
    if (course.isMember("l"))
        weight_component(course, "l");
}


int main(int argc, const char * argv[]) {
    
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
            cout << "Failed to parse JSON string! Aborting..." << endl;
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
        
        for (auto iter = globalThreadIndex.end() - 1; !globalThreadIndex.empty(); iter--) {
            if (iter->joinable())
                iter->join();
            globalThreadIndex.pop_back();
        }

        //Now create array of possible configurations for each course
        vector<vector<Value>> courseConfigurations;
        for (vector<Value>::const_iterator iterator = classes.begin(); iterator != classes.end(); iterator++) {
            Value workingClass = *iterator; //holds the copy of the class in question that gets edited
            vector<Value> courses;
            const vector<string> components = {"c","tu", "l"};
            for(auto iter1 = components.begin(); iter1 != components.end();) {
                //if the component pointed to by iter# does not exist in the course, increment counter and continue.
                for (auto iter2 = iter1 + 1; iter2 != components.end();) {
                    //if the component pointed to by iter# does not exist in the course, increment counter and continue.
                    for (auto iter3 = iter2 + 1; iter3 != components.end();){
                        //if the component pointed to by iter# does not exist in the course, increment counter and continue.
                        // NOTE: somewhere in here, there needs to be an iterator in the three 'for' loops that iterate through the classes' available section slots, once the for loops are assigned the appropriate section to search. At the end of each of the 'for' blocks should be the update code to write the selected section to the selected component.
                    }
                }
            }
        }
        
    }
    else {
        cout << "The JSON file could not be read!" << endl;
        return 1;
    }

    return 0;
}