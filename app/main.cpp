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

//#include "grid_approx.hpp"
#include "utility.hpp"
#include "solver.hpp"
//#include "ant_colony.h"
//#include "algorithm.hpp"
#include "max_rect.hpp"

using namespace std;
using namespace collage_maker;

//#define GET_DATA
//#define DEBUG
//#define RELEASE



struct CollageMaker {
    vector<int> compose(vector<int>& data) {
        array<Mat, kSourceImageCount> source;
        Mat target;
        initData(data, source, target);
        
        
    }
};



int main(int argc, const char * argv[]) {
    CollageMaker m;
    vector<int> v;
    m.compose(v);
    
    
//#ifdef GET_DATA
//    ofstream out_input(root + "input.txt");
//    auto data = readInput(cin);
//    out_input << data.size() << " ";
//    for (int d : data) out_input << d << " ";
//    out_input.flush();
//#elif defined DEBUG
//    CollageMaker cm;
//    ifstream input(root + "input.txt");
//    auto data = readInput(input);
//    data = cm.compose(data);
//    for (int i = 0; i < data.size(); ++i) {
//        cout << data[i] << endl;
//    }
//    cout.flush();
//#elif defined RELEASE 
//    CollageMaker cm;
//    auto data = readInput(cin);
//    clock_t t = clock();
//    data = cm.compose(data);
//    //cerr << "time: " << (clock() - t)/CLOCKS_PER_SEC << endl;
//    for (int i = 0; i < data.size(); ++i) {
//        cout << data[i] << endl;
//    }
//    cout.flush();
//#endif
    return 0;
}






