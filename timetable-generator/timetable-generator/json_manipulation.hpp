//
//  json_manipulation.hpp
//  timetable-generator
//
//  Created by Darryl Murray on 2015-06-30.
//  Copyright © 2015 Darryl Murray. All rights reserved.
//

#ifndef json_manipulation_cpp
#define json_manipulation_cpp

#include <string>
#include <iostream>
#include <vector>
#include "dist/json/json.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::to_string;
using std::vector;
using Json::Value;


//DECLARATIONS
///Strips the leading and trailing quotation symbols from JsonObjects
string quoteStripper(string input);

///Lists the contents of a Json::Value, returns the number of items listed.
int list_object (const Json::Value& list, const bool withNumbers = false, const bool listKeys = false, vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false);

/**Combines the listing of a Json::value with asking what option they want to choose, with the appropriate error checking.
 
 Returns: 0, 1, 2
 
 0 if successful.
 
 1 if the user signalled to exit without making a choice.
 
 2 invalid JSON type
 */
int list_and_get(const Json::Value& list, const bool withNumbers = false, string * const retVal = NULL, const bool listKeys = false, const vector<string> keys = vector<string>(), const bool displayObjectKeysInsideArray = false);

#endif /* json_manipulation_cpp */
