//
//  solver.h
//  CollageMaker
//
//  Created by Anton Logunov on 6/28/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef SOLVER_H
#define SOLVER_H

#include <queue>

#include <ant>

namespace collage_maker {

using namespace std;

template<class T>
using Grid = ant::d2::grid::Grid<T>;
using MutMatView = ant::linalg::MutableMatrixView<int>;
using MatView = ant::linalg::MatrixView<int>;
using Mat = ant::linalg::Matrix<int>;
using Size = ant::d2::grid::Size;
using Index = ant::Index;
using Count = ant::Count;
using Region = ant::d2::grid::Region;
using Int = ant::Int;
using Position = ant::d2::grid::Position;
using ant::d2::grid::MaxEmptyRegions;

// place for statics
extern default_random_engine RNG;
const size_t kSourceImageCount = 200; 
const size_t kMaxTargetLength = 300;
const size_t kMaxSourceLength = 100;


struct CoordinateTransformation {
    
    static Region toTopLeftGoesRight(const Region& or_reg, const Size& or_size) {
        return or_reg;
    }
    
    // use this to create and use sources going in multiple directions
    static Region toTopLeftGoesDown(const Region& or_reg, const Size& or_size) {
        return Region(or_reg.col_begin(), or_reg.row_begin(), 
                      or_reg.col_count(), or_reg.row_count());
    }
    
    static Region toTopRightGoesLeft(const Region& or_reg, const Size& or_size) {
        return Region(or_reg.row_begin(), or_size.col - or_reg.col_end(),
                      or_reg.row_count(), or_reg.col_count());
    }
    
    static Region toTopRightGoesDown(const Region& or_reg, const Size& or_size) {
        return Region(or_size.col - or_reg.col_end(), or_reg.row_begin(),
                      or_reg.col_count(), or_reg.row_count());
    }
    
    static Region toBotLeftGoesUp(const Region& or_reg, const Size& or_size) {
        return Region(or_reg.col_begin(), or_size.row - or_reg.row_end(),
                      or_reg.col_count(), or_reg.row_count());
    }
    
    static Region toBotLeftGoesRight(const Region& or_reg, const Size& or_size) {
        return Region(or_size.row - or_reg.row_end() , or_reg.col_begin(),
                      or_reg.row_count(), or_reg.col_count());
    }
    
    static Region toBotRightGoesUp(const Region& or_reg, const Size& or_size) {
        return Region(or_size.col - or_reg.col_end(), or_size.row - or_reg.row_end(),
                      or_reg.col_count(), or_reg.row_count());
    }
    
    static Region toBotRightGoesLeft(const Region& or_reg, const Size& or_size) {
        return Region(or_size.row - or_reg.row_end(), or_size.col - or_reg.col_end(),
                      or_reg.row_count(), or_reg.col_count());
    }
    
    static Region fromTopLeftGoesRight(const Region& reg, const Size& size) {
        return reg;
    }
    
    static Region fromTopLeftGoesDown(const Region& reg, const Size& size) {
        return toTopLeftGoesDown(reg, size);
    }
    
    static Region fromTopRightGoesLeft(const Region& reg, const Size& size) {
        return toTopRightGoesLeft(reg, size);
    }
    
    static Region fromTopRightGoesDown(const Region& reg, const Size& size) {
        return toBotLeftGoesUp(reg, size);
    }
    
    static Region fromBotLeftGoesUp(const Region& reg, const Size& size) {
        return toTopRightGoesDown(reg, size);
    }
    
    static Region fromBotLeftGoesRight(const Region& reg, const Size& size) {
        return toBotLeftGoesRight(reg, size);
    }
    
    static Region fromBotRightGoesUp(const Region& reg, const Size& size) {
        return toBotRightGoesUp(reg, size);
    }
    
    static Region fromBotRightGoesLeft(const Region& reg, const Size& size) {
        return toBotRightGoesLeft(reg, size);
    }
};

    
Position scalePosition(const Position& original_p, const Size& origional_size, const Size& size);
int scaleSmart(const Position& original_p, const Size& original_s, const Mat& m);    
Mat scaleSmart(const Mat& source, const Size& size);    
Mat scaleSilly(const Mat& source, const Size& size);
// better create function wtf
Mat scalePrecise(const Mat& source, const Size& size);    

int score(const Mat& source, const Mat& target);
int score(const Mat& source, const MatView& target_view);
int score(const Mat& source, const Position& pos, const Mat& target);
int scoreSmart(const map<Index, Region>& regions, const array<Mat,kSourceImageCount>& source, const Mat& target);
int scoreSilly(const map<Index, Region>& regions, const array<Mat,kSourceImageCount>& source, const Mat& target);

vector<Region> scaleInnerRegions(vector<Region>& original_regions, Size original_size, Size size);

template<class ForwardIterator>
Mat readImage(ForwardIterator& start) {
    auto 
    h = *(start),
    w = *(++start);
    Mat m(h, w);
    for (auto r = 0; r < h; ++r) {
        for (auto c = 0; c < w; ++c) {
            m(r, c) = *(++start);
        }
    }
    ++start;
    return m;
} 

using SourceMats = array<Mat, kSourceImageCount>;

void initData(const vector<int>& data, SourceMats& source, Mat& target);
vector<int> formatCollage(const map<Index, Region>& regions);



struct Base {
    
    void init(shared_ptr<const Mat> target, shared_ptr<const SourceMats> source) {
        target_ = target;
        source_ = source;
    }
    
    virtual void compose() = 0;
    virtual double score() const = 0;
    virtual vector<pair<Index, Region>> collage() const = 0;
    
    shared_ptr<const Mat> target_;
    shared_ptr<const SourceMats> source_;

};



}

#endif
