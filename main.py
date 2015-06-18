__author__ = 'darrylmurray'

import json
from pprint import pprint
from math import log
from copy import deepcopy



# GLOBAL VARIABLES/INFORMATION
with open('/Users/darrylmurray/Desktop/data.json') as data_file:
    data = json.load(data_file)

deptsList = sorted(list(data['departments'].keys()))

chosenCourses = []

"""
Weight design:

* Each course's component starts off at a 5000 weighting
* Each course component's weight gets subtracted by the formula x^1.2 where x is the number of minutes from 8:30am
* A course component that goes into the 12:00-1:00pm block is subtracted 800 weighting points
* A course component gets 300/60 * x added to it's weighting, where x is number of minutes the component consumes
* The final weighting is subtracted by log_10(x)*1000, where x is the number of sections the course has
* If a component section has multiple classes, weight each one, then find the geometric mean
* If a class is taught by Quazi Rahman, it gets added a 5000 weighting
"""


# FUNCTIONS
def num_list(content, keys=["nil"]):
    count = 0
    for x in content:
        if keys == ["nil"]:
            print (str(count + 1) + ":\t\t"+ x)
        else:
            extras = ""
            for entries in keys:
                extras += (x[entries] + " ")
            print (str(count + 1) + ":\t\t" + extras)
        count += 1

def get_num(questionText, maximum, minimum=0):
    number = 0
    while True:
        selected = input("\n" + questionText)
        try:
            number = int(selected) - 1
            if number > maximum or number < minimum:
                raise ValueError
            break
        except ValueError:
            print("Invalid number")

    return number

def iroot(k, n):  # Finds the nth root of a number, where n is the number, and k is the root
    u, s = n, n+1
    while u < s:
        s = u
        t = (k-1) * s + n // pow(s, k-1)
        u = t // k
    return s

def geometric_mean(listOfWeightings):
    total = 1
    for weighting in listOfWeightings:
        total *= weighting
    return iroot(len(listOfWeightings), total)

def time_conversion(hour, minute):
    return (hour - 8) * 60 + (minute - 30)

def weight_time(startTime, endTime):
    weight = 0  # Since starting weight is determined by an upper function

    lunchSet = set(range(time_conversion(12, 00), time_conversion(1, 00)))
    classSet = set(range(startTime, endTime))
    if len(lunchSet.intersection(classSet)) > 0:
        weight -= 800
    weight -= pow(startTime, 1.2)
    weight += (endTime - startTime) * 5
    return weight

def weight_component(course, selector):
    count = len(course[selector])  # counts the number of sections in this course's component
    for section in course[selector]:  # for each section in this course component
        weightList = []
        for timeSlot in section['ti']:  # and for each time slot taken in this section's course component
            weightList.append(5000 + weight_time(time_conversion(timeSlot[2], timeSlot[3]),
                                                 time_conversion(timeSlot[4], timeSlot[5])))
            # calculate the weighting of each time slot
        sectionWeight = geometric_mean(weightList)  # the section's final weight:
                                                    # geometric mean of the weight of each of its time slots
        sectionWeight -= (log(count) * 1000)  # decrease the section weight by the number of sections
        if 'Rahman' in section['sups']:
            sectionWeight += 5000
        section.update({'sw': sectionWeight})  # tag each section with it's weight

def weight_course(course):
    # weight each lecture
    if 'c' in course:
        weight_component(course, 'c')

    # weight each tutorial
    if 'tu' in course:
        weight_component(course, 'tu')

    # weight each lab
    if 'l' in course:
        weight_component(course, 'l')

def generate_combinations(inputData):
    # Need to chop down inputData first
    newList = []
    existingList = []
    if len(inputData) > 1:
        for item in inputData[1:len(inputData)]:
            newList.append(item)
        existingList = generate_combinations(newList)

    # Take the returned list of combinations of previous courses, and add in new versions with this course in it
    newGenList = []
    counter = 0
    for version in inputData[0]:

        if len(existingList) == 0:
            newGenList.append([version])
        else:
            for item in existingList:
                newGenList.append(deepcopy(item))
            for existVer in existingList:
                newGenList[counter].append(version)
                counter += 1
    return newGenList

def remove_conflicts(inputData):
    badCandidates = []

    for cNum, candidate in enumerate(inputData):
        setsOfTimes = [{'mo':[],
                        'tu':[],
                        'we':[],
                        'th':[],
                        'fr':[]},
                       {'mo':[],
                        'tu':[],
                        'we':[],
                        'th':[],
                        'fr':[]}]   # the list has two dictionaries for term 1 and term 2, listing the
                                    # time consumptions for each day in that term

        # Generate a set of each of the classes' time consumptions
        for course in candidate:
            if 'c' in course:
                for times in course['c'][0]['ti']:
                    if times[0] == 1 or times[0] == 3:
                        setsOfTimes[0][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))
                    if times[0] == 2 or times[0] == 3:
                        setsOfTimes[1][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))
            if 'tu' in course:
                for times in course['tu'][0]['ti']:
                    if times[0] == 1 or times[0] == 3:
                        setsOfTimes[0][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))
                    if times[0] == 2 or times[0] == 3:
                        setsOfTimes[1][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))
            if 'l' in course:
                for times in course['l'][0]['ti']:
                    if times[0] == 1 or times[0] == 3:
                        setsOfTimes[0][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))
                    if times[0] == 2 or times[0] == 3:
                        setsOfTimes[1][times[1]].append(set(range(time_conversion(times[2], times[3] + 1),time_conversion(times[4], times[5] - 1),10)))

        # Detect set overlaps, note down
        for term in setsOfTimes:
            for day in term:
                for set1 in term[day]:
                    for set2 in term[day]:
                        if set1 != set2:
                            if len(set1.intersection(set2)) != 0:
                                badCandidates.append(cNum)

        # Delete bad candidates
        while len(badCandidates) > 0:
            inputData.remove(badCandidates.pop()) # Start from the end so that the index numbers stay valid

def modified_quickSort(alist):
   quickSortHelper(alist,0,len(alist)-1)

def quickSortHelper(alist,first,last):
   if first<last:

       splitpoint = partition(alist,first,last)

       quickSortHelper(alist,first,splitpoint-1)
       quickSortHelper(alist,splitpoint+1,last)


def partition(alist,first,last):
   pivotvalue = alist[first][len(alist[first])-1]['weight']

   leftmark = first+1
   rightmark = last

   done = False
   while not done:

       while leftmark <= rightmark and \
               alist[leftmark][len(alist[leftmark])-1]['weight'] <= pivotvalue:
           leftmark = leftmark + 1

       while alist[rightmark][len(alist[rightmark])-1]['weight'] >= pivotvalue and \
               rightmark >= leftmark:
           rightmark = rightmark -1

       if rightmark < leftmark:
           done = True
       else:
           temp = deepcopy(alist[leftmark])
           alist[leftmark] = deepcopy(alist[rightmark])
           alist[rightmark] = deepcopy(temp)

   temp = deepcopy(alist[first])
   alist[first] = deepcopy(alist[rightmark])
   alist[rightmark] = deepcopy(temp)


   return rightmark

def rank_list(inputList):
    for candidate in inputList:
        totalWeight = 0
        for course in candidate:
            cWeight = 0
            counter = 0
            if 'c' in course:
                cWeight += course['c'][0]['sw']
                counter += 1
            if 'tu' in course:
                cWeight += course['tu'][0]['sw']
                counter += 1
            if 'l' in course:
                cWeight += course['l'][0]['sw']
                counter += 1
            course.update({'cw': cWeight / counter})
            totalWeight += cWeight / counter
        candidate.append({'weight': totalWeight / len(candidate)})

    modified_quickSort(inputList)

    inputList.reverse()
    #Find aggregate weighting for each course, then each possibility
    # sort possibilities by weighting, high to low




# MAIN PROGRAM
def main():
    while True:
        num_list(deptsList)
        number = get_num("Choose a department by number (0 to exit): ", len(deptsList) - 1, -1)  # so that 0 is exit

        if number == -1:
            break

        courseList = data['courses'][data['departments'][deptsList[number]]]
        num_list(courseList, ["cod", "n"])
        number = get_num("Choose a course by number (0 to exit): ", len(courseList) - 1, -1)  # so that 0 is exit

        if number == -1:
            break

        chosenCourses.append(courseList[number])

    for course in chosenCourses:
        weight_course(course)

    # time to create an array of each of the possible course configurations
    # NOTE: THIS IS HORRIBLY STRUCTURED CODE! FIX!
    courseConfigurationsStepOne = []

    counter2 = 0
    for counter1, course in enumerate(chosenCourses):
        courseConfigurationsStepOne.append([])
        if 'c' in course and 'tu' in course and 'l' in course:
            for c1 in course['c']:
                for c2 in course['tu']:
                    for c3 in course['l']:
                        courseConfigurationsStepOne[counter1].append(deepcopy(course))
                        courseConfigurationsStepOne[counter1][counter2].update({'c': [c1]})
                        courseConfigurationsStepOne[counter1][counter2].update({'tu': [c2]})
                        courseConfigurationsStepOne[counter1][counter2].update({'l': [c3]})
                        counter2 += 1
            counter2 = 0
        elif 'c' in course and 'l' in course:
            for c1 in course['c']:
                for c2 in course['l']:
                    courseConfigurationsStepOne[counter1].append(deepcopy(course))
                    courseConfigurationsStepOne[counter1][counter2].update({'c': [c1]})
                    courseConfigurationsStepOne[counter1][counter2].update({'l': [c2]})
                    counter2 += 1
            counter2 = 0
        elif 'c' in course and 'tu' in course:
            for c1 in course['c']:
                for c2 in course['tu']:
                    courseConfigurationsStepOne[counter1].append(deepcopy(course))
                    courseConfigurationsStepOne[counter1][counter2].update({'c': [c1]})
                    courseConfigurationsStepOne[counter1][counter2].update({'tu': [c2]})
                    counter2 += 1
            counter2 = 0
        elif 'c' in course:
            for c1 in course['c']:
                courseConfigurationsStepOne[counter1].append(deepcopy(course))
                courseConfigurationsStepOne[counter1][counter2].update({'c': [c1]})
                counter2 += 1
            counter2 = 0
        elif 'tu' in course and 'l' in course:
            for c1 in course['tu']:
                for c2 in course['l']:
                    courseConfigurationsStepOne[counter1].append(deepcopy(course))
                    courseConfigurationsStepOne[counter1][counter2].update({'tu': [c1]})
                    courseConfigurationsStepOne[counter1][counter2].update({'l': [c2]})
                    counter2 += 1
            counter2 = 0
        elif 'tu' in course:
            for c1 in course['tu']:
                courseConfigurationsStepOne[counter1].append(deepcopy(course))
                courseConfigurationsStepOne[counter1][counter2].update({'tu': [c1]})
                counter2 += 1
            counter2 = 0
        elif 'l' in course:
            for c1 in course['l']:
                courseConfigurationsStepOne[counter1].append(deepcopy(course))
                courseConfigurationsStepOne[counter1][counter2].update({'l': [c1]})
                counter2 += 1
            counter2 = 0
        else:
            raise Exception("This class has no impact on the timetable! Is something wrong?")

    # END HORRIBLY STRUCTURED CODE

    configPossibilities = generate_combinations(courseConfigurationsStepOne)

    remove_conflicts(configPossibilities)

    rank_list(configPossibilities)

    pprint(configPossibilities)


main()
