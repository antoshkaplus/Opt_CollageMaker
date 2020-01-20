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
#include "max_rect_3.hpp"
#include "max_rect_4.hpp"
#include "grid_approx.hpp"
#include "grid_layout.hpp"
#include "genetic.hpp"
#include "ant/core/core.hpp"

#ifdef WITHGPERFTOOLS
    #include "gperftools/profiler.h"
#endif


using namespace std;
using namespace collage_maker;


int main(int argc, const char * argv[]) {

#ifdef WITHGPERFTOOLS
    ProfilerRegisterThread();
    // for local profiler call Start/Stop with the name
    // in that case don't need to specify env variable CPUPROFILE
#endif

    auto pars = ant::command_line_options(argv, argc);
    string input_path = "input.txt";
    string output_path = "output.txt";
    if (pars.count("input") == 1 && pars.count("output") == 1) {  
        input_path = pars["input"];
        output_path = pars["output"];
    }
    ifstream input(input_path);
    ofstream output(output_path);
    
    auto v = readInput(input);
    MaxRect_4 solver;
    SourceMats source;
    Mat target;
    initData(v, source, target);
    
    GridApprox<25> approx;
    GridSourceScore res = approx.construct(target, source);
    GridLayout layout(1.5);
    auto places = layout.layout(res);
    vector<Item> items = approx.convert(places, target, source);
    
//    auto items = solver.compose(target, source);
    
    assert(isValid(items, target.size()));
    //cout << score<decltype(scaleSilly)>(items, source, target, scaleSmart) << endl;
    
    for (int i : formatCollage(items)) {
        output << i << " ";
    }

    return 0;
}






