//
//  main.cpp
//  Test
//
//  Created by Anton Logunov on 6/27/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <iostream>
#include <array>

#include "utility.hpp"
#include "solver.hpp"
//#include "max_rect.hpp"

#include "gtest/gtest.h"

using namespace collage_maker;

// version 1: 25127
// version 2: 4735
TEST(Scale, Silly) {
    Size s_original(100, 100);
    Size s_scale(50, 50);
    
    Mat m_0(s_original);
    Mat m_1(100, 100);
    
    Mat m(s_original);
    
    clock_t t_0 = clock();
    for (int i = 0; i < 1000; ++i) {    
        collage_maker::scaleSilly(m, s_scale);
    }
    cout << "usual: " << clock() - t_0 << endl;
    t_0 = clock();
    for (int i = 0; i < 1000; ++i) {    
        collage_maker::scaleSilly_2(m, s_scale);
    }
    cout << "improved: " << clock() - t_0 << endl;
}

TEST(MaxRect, ScaleFunction) {

}

// smart and precise scaling should be the same
// source images should be smaller, size 100*100 max
TEST(Scale, SmartPreciseComparison) {
    ifstream input("input.txt");
    vector<int> in = readInput(input);
    SourceMats source;
    Mat target;
    initData(in, source, target);
    default_random_engine rng;
    for (int i = 0; i < source.size(); ++i) {
        auto& s = source[i];
        uniform_int_distribution<> row_distr(20, s.row_count() + 1);
        uniform_int_distribution<> col_distr(20, s.col_count() + 1);
        Size newSize(row_distr(rng), col_distr(rng));
        Mat m_smart = scaleSmart(s, newSize);
        Mat m_precise = scalePrecise(s, newSize);
        ASSERT_TRUE(ant::linalg::any(m_smart - m_precise == 0));
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::FLAGS_gtest_filter = "MaxRect*";
    return RUN_ALL_TESTS();

//    assert(reg == Transformation::fromTopLeftGoesDown(Transformation::toTopLeftGoesDown(reg, sz), sz.swapped()));
//    assert(reg == Transformation::fromTopRightGoesLeft(Transformation::toTopRightGoesLeft(reg, sz), sz));
//    assert(reg == Transformation::fromTopRightGoesDown(Transformation::toTopRightGoesDown(reg, sz), sz.swapped()));
//    assert(reg == Transformation::fromBotLeftGoesRight(Transformation::toBotLeftGoesRight(reg, sz), sz));
//    assert(reg == Transformation::fromBotLeftGoesUp(Transformation::toBotLeftGoesUp(reg, sz), sz.swapped()));
//    assert(reg == Transformation::fromBotRightGoesLeft(Transformation::toBotRightGoesLeft(reg, sz), sz));
//    assert(reg == Transformation::fromBotRightGoesUp(Transformation::toBotRightGoesUp(reg, sz), sz.swapped()));
    return 0;
}
