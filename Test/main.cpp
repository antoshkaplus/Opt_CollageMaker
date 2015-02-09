//
//  main.cpp
//  Test
//
//  Created by Anton Logunov on 6/27/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <iostream>

#include "utility.h"
#include "solver.h"


int main(int argc, const char * argv[]) {
    
    Region reg(5, 3, 2, 1);
    Size sz(8, 10);
    
    assert(reg == Transformation::fromTopLeftGoesDown(Transformation::toTopLeftGoesDown(reg, sz), sz.swapped()));
    assert(reg == Transformation::fromTopRightGoesLeft(Transformation::toTopRightGoesLeft(reg, sz), sz));
    assert(reg == Transformation::fromTopRightGoesDown(Transformation::toTopRightGoesDown(reg, sz), sz.swapped()));
    assert(reg == Transformation::fromBotLeftGoesRight(Transformation::toBotLeftGoesRight(reg, sz), sz));
    assert(reg == Transformation::fromBotLeftGoesUp(Transformation::toBotLeftGoesUp(reg, sz), sz.swapped()));
    assert(reg == Transformation::fromBotRightGoesLeft(Transformation::toBotRightGoesLeft(reg, sz), sz));
    assert(reg == Transformation::fromBotRightGoesUp(Transformation::toBotRightGoesUp(reg, sz), sz.swapped()));
    
    
    
    
    std::cout << "Hello, World!\n";
    return 0;
}
