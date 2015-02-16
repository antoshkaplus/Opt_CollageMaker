//
//  max_rect_3.hpp
//  CollageMaker
//
//  Created by Anton Logunov on 2/16/15.
//
//

#ifndef CollageMaker_max_rect_3_hpp
#define CollageMaker_max_rect_3_hpp


#include <cmath>

#include "solver.hpp"

namespace collage_maker {
    
    struct MaxRect_3 : Composer {
    private:
        struct Insertion {
            Index source_index;
            double score;
            Size size;
            
            Insertion() : source_index(-1) {}
            
            Insertion(Index source_index, Size size, double score) 
            : source_index(source_index), size(size), score(score) {}
        };
        
        struct CompRegions {
            bool operator()(const Region& r_0, const Region& r_1) const {
                return r_0.row_begin() < r_1.row_begin() 
                || (r_0.row_begin() == r_1.row_begin() && r_0.col_begin() < r_1.col_begin()) 
                || (r_0.row_begin() == r_1.row_begin() && r_0.col_begin() == r_1.col_begin() && r_0.row_count() < r_1.row_count())
                || (r_0.row_begin() == r_1.row_begin() && r_0.col_begin() == r_1.col_begin() 
                    && r_0.row_count() == r_1.row_count() && r_0.col_count() < r_1.col_count());
            }
        };
        
        set<Index> unused_sources_;
        const Mat* target_;
        const SourceMats* source_;
        // if you build a queue for each item it will be even faster
        map<Region, Insertion, CompRegions> best_insertions_; 
        
    public:
        
        Insertion bestInsertion(const Region& r) {
            return best_insertions_[r];
        }
        
        void findBestInsertion(const Region& region) {
            auto& target = *target_;
            auto& source = *source_; 
            
            Index best_i;
            double best_local_score = numeric_limits<double>::max(), local_score;
            Size best_local_size, local_size;
            for (Index i : unused_sources_) {
                // need to try different sizes
                for (auto r_f : {0.3, 0.5, 0.7, 1.}) {
                    for (auto c_f : {0.3, 0.5, 0.7, 1.}) {
                        int s_r = r_f * source[i].row_count();
                        int s_c = c_f * source[i].col_count();
                        if (s_r == 0 || s_c == 0) continue;
                        local_size = {std::min(region.row_count(), s_r), 
                                      std::min(region.col_count(), s_c)};
                        Mat m = scaleSilly_2(source[i], local_size);
                        local_score = 1. * score(m, region.position, target) / local_size.cell_count();
                        
                        if (local_score < best_local_score) {
                            best_i = i;
                            best_local_size = local_size;
                            best_local_score = local_score;
                        }        
                    }
                }
            }
            best_insertions_[region] = {best_i, best_local_size, best_local_score};
        }
        
        bool isSourceUsed(Index i) {
            return unused_sources_.count(i) == 0;
        }
        
        vector<Item> compose(const Mat& target, const SourceMats& source) override {
            target_ = &target;
            source_ = &source;
            
            vector<Item> result;
            
            for (Index i = 0; i < source.size(); ++i) {
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
                Region fill_in = {best_region.position, best_insertion.size};
                
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
