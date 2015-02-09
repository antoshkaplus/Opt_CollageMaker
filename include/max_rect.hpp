//
//  max_rect.h
//  CollageMaker
//
//  Created by Anton Logunov on 7/4/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __MAX_RECT_H__
#define __MAX_RECT_H__

#include "solver.h"

namespace collage_maker {
    
namespace max_rect {

struct Naive : collage_maker::Base {
    using collage_maker::Base::compose; 
    
    map<Index, Region> compose() override {
        map<Index, Region> result;
        score_ = 0;
        Mat pseudo_target = scaleSmart(target_, pseudo_size_);
        
        vector<Index> unused_source(source_.size());
        std::iota(unused_source.begin(), unused_source.end(), 0);
        
        ant::d2::grid::MaxEmptyRegions empty_regions(
                0, 0, (Int)pseudo_target.row_count(), (Int)pseudo_target.col_count());
        
        while (!empty_regions.max_empty_regions().empty() && !unused_source.empty()) {
            Index i = uniform_int_distribution<>(0, (Int)empty_regions.max_empty_regions().size()-1)(RNG);
            const Region& r = empty_regions.max_empty_regions()[i]; 
            
            Index best_i = unused_source.size();
            double best_local_score = numeric_limits<double>::max(), local_score;
            Size best_local_size(1, 1);
            for (Index i : unused_source) {
                Int row_count = min(r.row_count(), (Int)source_[i].row_count());
                Int col_count = min(r.col_count(), (Int)source_[i].col_count());
                
                if (row_count < source_[i].row_count() || col_count < source_[i].col_count()) {
                    Mat s = scaleSilly(source_[i], Size(row_count, col_count));
                    local_score = ::collage_maker::score(s, Position(r.row_begin(), r.col_begin()), pseudo_target);
                } else {
                    local_score = ::collage_maker::score(source_[i], Position(r.row_begin(), r.col_begin()), pseudo_target);
                }
                if (local_score/(row_count*col_count) < best_local_score/(best_local_size.row*best_local_size.col)) {
                    best_i = i;
                    best_local_score = local_score;
                    best_local_size.set(row_count, col_count);
                }
            }
            Region reg(r.position, best_local_size);
            score_ += best_local_score;
            
            remove(unused_source.begin(), unused_source.end(), best_i);
            unused_source.pop_back();
            
            empty_regions.insertRegion(reg);
            
            result[best_i] = reg;
        }
        score_ /= pseudo_target.element_count();
        score_ = sqrt(score_);
        
        vector<Index> is;
        vector<Region> rs;        
        tie(is, rs) = ant::zip(result);
        rs = scaleInnerRegions(rs, pseudo_size_, target_.size());
        for (auto i = 0; i < result.size(); ++i) {
            result[is[i]] = rs[i];
        }
        score_ = sqrt(::collage_maker::scoreSmart(result, source_, target_)/target_.element_count());
        return result;
    }
    
    double score() const override {
        return score_;
    }
    
    void set_pseudo_size(const Size pseudo_size) {
        pseudo_size_ = pseudo_size;
    }
    
    double score_;
    Size pseudo_size_;
};

}
    
}


#endif