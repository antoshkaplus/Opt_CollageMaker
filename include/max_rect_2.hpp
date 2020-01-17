//
//  max_rect_2.hpp
//  CollageMaker
//
//  Created by Anton Logunov on 2/15/15.
//
//

#ifndef CollageMaker_max_rect_2_hpp
#define CollageMaker_max_rect_2_hpp


#include <cmath>

#include "solver.hpp"

namespace collage_maker {
    
    struct MaxRect_2 : Composer {
    private:
        struct Insertion {
            Index source_index;
            double score;
        
            Insertion() : source_index(-1) {}
            
            Insertion(Index source_index, double score) 
            : source_index(source_index), score(score) {}
        };
        
        
        struct CompRegions {
            bool operator()(const Region& r_0, const Region& r_1) const {
                return r_0.row_begin() < r_1.row_begin() 
                || (r_0.row_begin() == r_1.row_begin() && r_0.col_begin() < r_1.col_begin());
            }
        };
        
        set<Index> unused_sources_;
        const Mat* target_;
        const SourceMats* source_;
        map<Region, Insertion, CompRegions> best_insertions_; 
        
    public:
        
        Insertion bestInsertion(const Region& r) {
            return best_insertions_[r];
        }
        
        Size findFillInSize(const Mat& source, Region target_region) {
            if (target_region.row_count() < source.row_count() || target_region.col_count() < source.col_count()) {
                return {std::min(target_region.row_count(), (Int)source.row_count()), 
                        std::min(target_region.col_count(), (Int)source.col_count())};
            } 
            return source.size();
        }
        
        void findBestInsertion(const Region& r) {
            auto& target = *target_;
            auto& source = *source_; 
            
            Index best_i;
            double best_local_score = numeric_limits<double>::max(), local_score;
            Size best_local_size, local_size;
            for (Index i : unused_sources_) {
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
            best_insertions_[r] = {best_i, best_local_score};
        }
        
        bool isSourceUsed(Index i) {
            return unused_sources_.count(i) == 0;
        }
        
        vector<Item> compose(const Mat& target, const SourceMats& source) override {
            target_ = &target;
            source_ = &source;
            
            vector<Item> result;
            
            for (size_t i = 0; i < source.size(); ++i) {
                unused_sources_.insert(i);
            }
            
            ant::grid::MaxEmptyRegions empty_regions({0, 0}, target.size());
            
            while (!empty_regions.max_empty_regions().empty()) {
                assert(!unused_sources_.empty());
                Region best_region;
                Insertion best_insertion;
                best_insertion.score = numeric_limits<double>::max();
                for (auto& r : empty_regions.max_empty_regions()) {
                    auto ins = bestInsertion(r);
                    if (ins.source_index == -1 || isSourceUsed(ins.source_index)) {
                        findBestInsertion(r);
                        ins = bestInsertion(r);
                    }
                    if (best_insertion.score > ins.score) {
                        best_region = r;
                        best_insertion = ins;
                    }
                }
                Region fill_in = {best_region.position, findFillInSize(source[best_insertion.source_index], best_region)};
                
                best_insertions_.erase(best_region);
                unused_sources_.erase(best_insertion.source_index);
                
                empty_regions.FillRegion(fill_in);
                result.emplace_back(best_insertion.source_index, fill_in);
            }
            return result;
        }
    };
    
    
}


#endif
