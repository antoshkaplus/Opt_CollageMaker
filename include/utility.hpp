//
//  utility.h
//  CollageMaker
//
//  Created by Anton Logunov on 6/27/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef UTILITY_H
#define UTILITY_H

#include <fstream>

#include <ant>


using namespace std;

vector<int> readInput(istream& input) {
    int len;
    input >> len;
    vector<int> data(len);
    for (int i = 0; i < len; ++i) {
        input >> data[i];
    }
    return data; 
}


string root =   "/Users/antoshkaplus/Documents"
                "/Programming/Contests/TopCoder"
                "/CollageMaker/Scripts/";

ofstream out_collage(root + "collage.txt");
ofstream out_submat(root + "submat.txt");
//ofstream out_input(root + "input.txt");


#endif