//
//  json_manipulation.cpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#include "json_manipulation.hpp"


//FUNCTIONS
string quoteStripper(string input) {
    if (input.size() == 0)
        return input;
    if (input[input.size() - 1] == '\"')
        input.erase(input.end() - 1);
    if (input[0] == '\"')
        input.erase(input.begin());
    
    return input;
}

int list_object (const Json::Value& list, const bool withNumbers, const bool listKeys, vector<string> keys, const bool displayObjectKeysInsideArray) {
    
    // NOTE: listing the keys of an array of objects makes no sense.
    
    Value::iterator listIterator;
    vector <string> keyList;
    int counter = 0;
    
    if (list.isObject())
        keyList = list.getMemberNames();
    
    // Iterate through the contents of a JSON Value
    for (listIterator = list.begin(), counter = 0; listIterator != list.end(); counter++, listIterator++) {
        string extraText;
        
        //Add numbers, and pad with spaces as necessary
        if (withNumbers) {
            extraText = to_string(counter + 1) + ":";
            
            //Space out output
            while (extraText.length() < 7)
                extraText += " ";
        }
        
        if (listKeys == true && list.isObject()) //it's an object and listKeys is listed, iterator lists the keys
            cout << extraText << keyList[counter] << endl;
        else if (!keys.empty() && list.isObject()) { //its an object/map, the keys aren't to be listed, and there are specific key(s) that want to be listed, iterator lists key(s) contents on a single line
            bool found = false; //Indicates if a key was found
            
            //Search the key pointed to in the current loop for matches with the keys desired, indicate if search is successful
            for (auto i = keys.begin(); i != keys.end() && !found; i++) {
                if (keyList[counter] == *i)
                    found = true;
            }
            
            // if a match was found, write the key's contents to extra and send extra to output, else decrement counter so that the next object is logically numbered on the output
            if (found) {
                extraText += list.get(keys.back(), Json::Value()).asString() + " ";
                cout << extraText << endl;
            }
            else
                --counter;
        }
        else if (!keys.empty() && list.isArray() && list.size() > 0 && list[0].isObject() && displayObjectKeysInsideArray) { //lists keys in object inside array, iterator goes through objects - not keys
            for (auto key = keys.begin(); key != keys.end(); key++)
                extraText += list[counter].get(*key, Json::Value()).asString() + " ";
            cout << extraText << endl;
        }
        else //If all else fails, just spit the raw contents of the json value out to screen
            cout << extraText << *listIterator << endl;
    }
    
    return counter;
}

int list_and_get(const Json::Value& list, const bool withNumbers, string * const retVal, const bool listKeys, const vector<string> keys, const bool displayObjectKeysInsideArray) {
    
    list_object(list, withNumbers, listKeys, keys, displayObjectKeysInsideArray);
    
    if (retVal != NULL) {
        int ret = -1;
        cout << endl << "Select an option from the list above (press 0 to exit): ";
        cin >> ret;
        while (ret > list.size() || ret < 0) {
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
