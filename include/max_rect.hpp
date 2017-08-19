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


struct MaxRect : Composer {
    
    vector<Item> compose(const Mat& target, const SourceMats& source) override {
        vector<Item> result;
        
        vector<Index> unused_source(source.size());
        std::iota(unused_source.begin(), unused_source.end(), 0);
        
        ant::grid::MaxEmptyRegions empty_regions({0, 0}, target.size());
        
        while (!empty_regions.max_empty_regions().empty()) {
            assert(!unused_source.empty());
            Index i = uniform_int_distribution<>(0, (Int)empty_regions.max_empty_regions().size()-1)(RNG);
            const Region& r = empty_regions.max_empty_regions()[i]; 
            
            Index best_i = unused_source.size();
            double best_local_score = numeric_limits<double>::max(), local_score;
            Size best_local_size, local_size;
            for (Index i : unused_source) {
                if (r.row_count() < source[i].row_count() || r.col_count() < source[i].col_count()) {
                    local_size = {std::min(r.row_count(), (Int)source[i].row_count()), 
                                  std::min(r.col_count(), (Int)source[i].col_count())};
                    Mat m = scaleSilly_2(source[i], local_size);
                    local_score = 1. * score(m, r.position, target) / local_size.cell_count();
                } else {
                    local_size = source[i].size();
                    local_score = 1. * score(source[i], r.position, target) / local_size.cell_count();
                }   
                if (local_score < best_local_score) {
                    best_i = i;
                    best_local_size = local_size;
                    best_local_score = local_score;
                }
            }
            Region reg(r.position, best_local_size);
            
            remove(unused_source.begin(), unused_source.end(), best_i);
            unused_source.pop_back();
            
            empty_regions.FillRegion(reg);
            
            result.emplace_back(best_i, reg);
        }
        return result;
    }
};
    
    
}


#endif