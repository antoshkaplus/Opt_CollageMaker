//
//  grid_approx.h
//  CollageMaker
//
//  Created by Anton Logunov on 7/4/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __GRID_APPROX__H_
#define __GRID_APPROX__H_

#include <valarray>
#include <memory>
#include <array>


#include "solver.hpp"


namespace collage_maker {

namespace grid_approx {

// i - who, (h, w) - what , (r, c) - where in target     
//using SourceScore = array<Grid<Grid<double>>, kSourceImageCount>;

struct SourceScore;


namespace algorithm {
    
    struct Base {
        virtual map<Index, Region> compose(const SourceScore& source_score, const Size& target_size) = 0;
        virtual int score() const = 0;
        virtual ~Base() {}
    };
}


struct Placement {
    Placement() {}
    Placement(const Position& p, const Size& s, Index i)  
    : position(p), size(s), source_index(i) {}
    
    bool isMutuallyOpposite(const Placement& pl) {
        return source_index == pl.source_index || Region(position, size).hasIntersection(Region(pl.position, pl.size));
    }
    
    Position position;
    Size size;
    Index source_index;
};


struct SourceScore {
    SourceScore(const Size& target_size, const Size& max_source_size) 
    :   data_(data_element_count_, numeric_limits<double>::max()),
        target_size_(target_size),
        max_source_size_(max_source_size) {}
    
    Index score_id(const Position& p, const Size& s, Index i) const {
        return data_index(p, s, i);
    }
    
    Placement placement(Index id_) const {
        Placement pl;
        pl.position.row = id_/p_row_factor_0;
        id_ %= p_row_factor_0;
        pl.position.col = id_/p_col_factor_1;
        id_ %= p_col_factor_1;
        pl.size.row = id_/s_row_factor_2;
        id_ %= s_row_factor_2;
        pl.size.col = id_/s_col_factor_3;
        id_ %= s_col_factor_3;
        pl.source_index = id_;
        return pl;
    }
    
    Index score_id_end() const {
        return data_element_count_;
    }
    
    void set_score(const Position& p, const Size& s, Index i, double score) {
        data_[data_index(p, s, i)] = score;
    }
    
    void set_score(Index id_, double score) {
        data_[id_] = score;
    }
    
    double score(const Position& p, const Size& s, Index i) const {
        return data_[data_index(p, s, i)];
    }
    
    double score(Index id_) const {
        return data_[id_];
    }
    
    const Size& target_size() const {
        return target_size_;
    }
    
    const Size& max_source_size() const {
        return max_source_size_;
    }
    
    bool isInitialized(Index id_) const {
        return data_[id_] != numeric_limits<double>::max();
    }
    
private:

    Index data_index(const Position& p, const Size& s, Index i) const {
        return  p.row*p_row_factor_0 + p.col*p_col_factor_1
                + (s.row-1)*s_row_factor_2 + (s.col-1)*s_col_factor_3
                + i;
    }
    
    
    const Size target_size_;
    const Size max_source_size_;
    
    const Count data_element_count_ =   target_size_.cell_count()
                                        *max_source_size_.cell_count()
                                        *kSourceImageCount; 
    const Int p_row_factor_0{static_cast<Int>(data_element_count_/target_size_.row)};
    const Int p_col_factor_1{p_row_factor_0/target_size_.col};
    const Int s_row_factor_2{p_col_factor_1/max_source_size_.row};
    const Int s_col_factor_3{s_row_factor_2/max_source_size_.col};
    
    valarray<double> data_;
};




struct Base : collage_maker::Base {
    
    Base() : cell_size_(25) {}
    
    void initPseudoTarget() {
        Size s = target_->size();
        if (s.row%cell_size_ != 0) {
            s.row += - (Int)(s.row%cell_size_) + (Int)cell_size_;
        }
        if (s.col%cell_size_ != 0) {
            s.col += - (Int)(s.col%cell_size_) + (Int)cell_size_;
        }
        pseudo_target_ = scaleSmart(*target_, s);
        target_cell_count_.set(s.row/cell_size_, s.col/cell_size_);
    }
    
    void initSourceScore() {
        Int 
        T_H = target_cell_count_.row,
        T_W = target_cell_count_.col;
        Int 
        S_MAX_H = 4,
        S_MAX_W = 4;
        auto& source = *source_;
//        for (auto i = 0; i < kSourceImageCount; ++i) {
//            Size sz(source[i].row_count()/cell_size_, source[i].col_count()/cell_size_);
//            (*source_score_)[i].resize(sz);
//            for (auto h = 0; h < sz.row; ++h) {
//                for (auto w = 0; w < sz.col; ++w) {
//                    source_score_[i](h, w).resize(T_H-h, T_W-w);
//                }
//            }
//        }
        
        if (T_H*T_W > 12*9) {
            for (auto h = 0; h < S_MAX_H; ++h) {
                for (auto w = 0; w < S_MAX_W; ++w) {
                    Grid<Mat> target_bits(T_H-h, T_W-w);
                    Size sz((h+1)*cell_size_/5, (w+1)*cell_size_/5);
                    for (auto r = 0; r < T_H-h; ++r) {
                        for (auto c = 0; c < T_W-w; ++c) {
                            auto& tb = target_bits(r, c);
                            tb.set_size(sz);
                            for (auto rr = 0; rr < sz.row; ++rr) {
                                for (auto cc = 0; cc < sz.col; ++cc) {
                                    tb(rr, cc) = ant::linalg::sum(target_->submat(
                                            r*cell_size_ + rr*5, 
                                            c*cell_size_ + cc*5,
                                            5, 5))/25.;
                                }
                            }
                        }
                    }
//                    for (auto i = 0; i < kSourceImageCount; ++i) {
//                        if (h >= source_score_[i].row_count() || w >= source_score_[i].col_count()) continue;
//                        Mat m = scaleSmart(source[i], sz);
//                        for (auto r = 0; r < T_H-h; ++r) {
//                            for (auto c = 0; c < T_W-w; ++c) {
//                                source_score_[i](h, w)(r, c) = 
//                                    ::collage_maker::score(m, target_bits(r, c))/(5.*5.);
//                            }
//                        }
//                    }
                
                }
            }
            
        }
        else {
            for (auto h = 0; h < S_MAX_H; ++h) {
                for (auto w = 0; w < S_MAX_W; ++w) {
                    Grid<Mat> target_bits(T_H-h, T_W-w);
                    Size sz((h+1)*cell_size_, (w+1)*cell_size_);
                    for (auto r = 0; r < T_H-h; ++r) {
                        for (auto c = 0; c < T_W-w; ++c) {
                            Position pos(r*cell_size_, c*cell_size_);
                            target_bits(r, c).set_size({
                                static_cast<Int>((h+1)*cell_size_), 
                                static_cast<Int>((w+1)*cell_size_)});
                            target_bits(r, c) = target_->submat(pos, sz);
                        }
                    }
//                    for (auto i = 0; i < kSourceImageCount; ++i) {
//                        if (h >= source_score_[i].row_count() || w >= source_score_[i].col_count()) continue;
//                        Mat m = scaleSmart(source[i], sz);
//                        for (auto r = 0; r < T_H-h; ++r) {
//                            for (auto c = 0; c < T_W-w; ++c) {
//                                source_score_[i](h, w)(r, c) = (double)::collage_maker::score(m, target_bits(r, c))/(double)(cell_size_*cell_size_);;
//                            }
//                        }
//                    }
                    
                }
            }

        
        
        }
        
    }
    
    void compose() override {
//        initPseudoTarget();
//        initSourceScore();
//        map<Index, Region> result = algorithm_->compose(source_score_, target_cell_count_);
//        vector<Index> is;
//        vector<Region> rs;        
//        tie(is, rs) = ant::zip(result);
//        rs = scaleInnerRegions(rs, target_cell_count_, target_->size());
//        for (auto i = 0; i < result.size(); ++i) {
//            result[is[i]] = rs[i];
//        }
//        score_ = sqrt((double)::collage_maker::scoreSmart(result, source_, target_)/target_->element_count());
//        return result;
    }
    
    double score() const override {
        return score_;
    }
    
    void set_algorithm(unique_ptr<algorithm::Base> algorithm) {
        algorithm_ = move(algorithm);
    }
    
    virtual ~Base() {}
    
    double score_;
    unique_ptr<algorithm::Base> algorithm_;
    const size_t cell_size_;
    Mat pseudo_target_;
    Size target_cell_count_;
    unique_ptr<SourceScore> source_score_;
    vector<pair<Index, Region>> collage_;
};

}

}

#endif