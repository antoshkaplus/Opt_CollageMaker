//
//  main.cpp
//  CollageMaker
//
//  Created by Anton Logunov on 6/25/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <array>

#include "utility.hpp"
#include "solver.hpp"
#include "max_rect.hpp"
#include "max_rect_2.hpp"

using namespace std;
using namespace collage_maker;


int main(int argc, const char * argv[]) {
    ifstream input("input.txt");
    auto v = readInput(input);
    MaxRect_2 solver;
    SourceMats source;
    Mat target;
    initData(v, source, target);
    auto items = solver.compose(target, source);
    ofstream output("output.txt");
    for (int i : formatCollage(items)) {
        output << i << " ";
    } 
    return 0;
}






