//
//  max_rect.h
//  CollageMaker
//
//  Created by Anton Logunov on 7/4/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __MAX_RECT_H__
#define __MAX_RECT_H__

#include <cmath>

#include "solver.hpp"

namespace collage_maker {

//
// Scale (mat, size)
// Score (mat, mat)  
template<class Scale, class Score>
struct MaxRect : Composer {
    
    vector<Item> compose(shared_ptr<Mat> target_ptr, shared_ptr<SourceMats> source_ptr) override {
        auto& target = *target_ptr; 
        auto& source = *source_ptr;
        
        score_ = 0;
        Mat pseudo_target = scaleSmart(target, pseudo_size_);
        
        vector<Index> unused_source(source.size());
        std::iota(unused_source.begin(), unused_source.end(), 0);
        
        ant::grid::MaxEmptyRegions empty_regions(
                0, 0, (Int)pseudo_target.row_count(), (Int)pseudo_target.col_count());
        
        while (!empty_regions.max_empty_regions().empty()) {
            assert(!unused_source.empty());
            Index i = uniform_int_distribution<>(0, (Int)empty_regions.max_empty_regions().size()-1)(RNG);
            const Region& r = empty_regions.max_empty_regions()[i]; 
            
            Index best_i = unused_source.size();
            double best_local_score = numeric_limits<double>::max(), local_score;
            Size best_local_size(1, 1);
            // trying to insert unused_source into this region
            for (Index i : unused_source) {
                if (r.row_count() < source[i].row_count() || r.col_count() < source[i].col_count()) {
                    Int row_count = std::min(r.row_count(), (Int)source[i].row_count());
                    Int col_count = std::min(r.col_count(), (Int)source[i].col_count());
                    
                    Scale()
                } else {
                    MatView tv(pseudo_target, )
                    local_score = Score()
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
        tie(is, rs) = ant::Zip(result);
        rs = scaleInnerRegions(rs, pseudo_size_, target.size());
        for (auto i = 0; i < result.size(); ++i) {
            result[is[i]] = rs[i];
        }
        score_ = sqrt(::collage_maker::scoreSmart(result, source, target)/target.element_count());
    }
};
    
    
}


#endif