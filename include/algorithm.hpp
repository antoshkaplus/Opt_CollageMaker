
//
//  algorithm.h
//  CollageMaker
//
//  Created by Anton Logunov on 7/4/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__


#include "grid_approx.h"

namespace collage_maker {

namespace grid_approx {

namespace algorithm {

struct Queue : Base {
    
    // will try to get n items per each size
    void initQueue(const grid_approx::SourceScore& source_score) {
        const Count kItemsPerPosition = 1500;
        vector<Placement> local_queue;
        local_queue.reserve(kSourceImageCount*4*4);
        source_queue_.clear();
        source_queue_.reserve(kItemsPerPosition*target_size_.cell_count());
        for (auto r = 0; r < target_size_.row; ++r) {
            for (auto c = 0; c < target_size_.col; ++c) {
                local_queue.clear();
                for (auto i = 0; i < kSourceImageCount; ++i) {
                    auto& ss = source_score[i]; 
                    for (auto h = 0; h < min<Int>(target_size_.row-r, ss.row_count()); ++h) {
                        for (auto w = 0; w < min<Int>(target_size_.col-c, ss.col_count()); ++w) {
                            local_queue.emplace_back(i, Position{r, c}, Size{h+1, w+1}, ss(h, w)(r, c));
                        }
                        
                    }
                }
                sort(local_queue.begin(), local_queue.end(), [](const Placement& p_0, const Placement& p_1) {
                    return p_0.score/p_0.s.cell_count() < p_1.score/p_1.s.cell_count();
                
                });
                source_queue_.insert(
                    source_queue_.end(), 
                    local_queue.begin(), 
                    local_queue.begin()+min(kItemsPerPosition, local_queue.size()));
            }
        }
        //auto& b_covering = best_coverage_;
    }
    
    
    // insex of source image, region
    virtual map<Index, Region> compose(const grid_approx::SourceScore& source_score, const Size& target_size) override {
        target_size_ = target_size;
        initQueue(source_score);
        map<Index, Region> best_result;
        double best_score = numeric_limits<double>::max();
        size_t best_gain = 0;
        
        auto gain_step = 1;
        Count count = target_size.cell_count();
        if (count <= 12*8) {
            gain_step = 4;
        } else if (count == 12*9) {
            gain_step = 4;
        } else if (count == 12*10) {
            gain_step = 4;
        } else if (count == 12*11) {
            gain_step = 4;
        } else if (count == 12*12) {
            gain_step = 4;
        }
        
        vector<Index> source_queue_indices(source_queue_.size());
        iota(source_queue_indices.begin(), source_queue_indices.end(), 0);
        vector<double> source_queue_values(source_queue_.size());
        //clock_t time = clock();
        for (auto gain = 90; gain <= 300; gain+=gain_step) {
            for (auto i = 0; i < source_queue_.size(); ++i) {
                auto& pl = source_queue_[i];
                source_queue_values[i] = pl.score/pl.s.cell_count() - gain*pl.s.cell_count();
            }
            sort(source_queue_indices.begin(), source_queue_indices.end(), [&](Index i_0, Index i_1) {
                return source_queue_values[i_0] < source_queue_values[i_1];
            });
            map<Index, Region> result;
            vector<bool> used(kSourceImageCount, false);
            Grid<char> covering(target_size);
            covering.fill(false);
            size_t covered_area = 0;
            double score = 0;
            for (auto& i : source_queue_indices) {
                auto& p = source_queue_[i];
                if (covered_area == target_size.cell_count()) break;
                if (used[p.i]) continue;
                // can hard code this one
                for (auto r = p.p.row; r < p.p.row + p.s.row; ++r) {
                    for (auto c = p.p.col; c < p.p.col + p.s.col; ++c) {
                        if (covering(r, c)) goto next;
                    }
                }
                for (auto r = p.p.row; r < p.p.row + p.s.row; ++r) {
                    for (auto c = p.p.col; c < p.p.col + p.s.col; ++c) {
                        covering(r, c) = true;
                    }
                }
                covered_area += p.s.cell_count();
                score += p.score;
                used[p.i] = true;
                result[p.i] = Region(p.p, p.s);
                next:;
            }
            if (covered_area == target_size.cell_count() && best_score > score) {
                //cerr << score << endl;
                //best_gain = gain;
                best_score = score;
                best_result = result;
            }
        }
        score_ = best_score;
        return best_result;
    }
    
    int score() const override {
        return score_;
    }
    
    virtual ~Queue() {}
    
    double score_;
    Size target_size_;
    vector<Placement> source_queue_;
    ant::linalg::Matrix<double> best_coverage_;
    
    static const Count kPlacementsPerCell = 1000;
};



struct BalanceQueue : Queue {
    
    // will try to get n items per each size
    void initQueue(const grid_approx::SourceScore& source_score, const Size target_size, const Size max_source_size) {
        //vector<Index> 
        
        
        using PrQueue = priority_queue<
                                Index, 
                                array<Index, kPlacementsPerCell>, 
                                function<bool(Index, Index)> >;
        Grid<unique_ptr<PrQueue>> placements;
        // returning max is good
        auto pl_comp = [&](Index i_0, Index i_1) {
            return source_score.score(i_0) < source_score.score(i_1);
        };
        for (auto r = 0; r < target_size.row; ++r) {
            for (auto c = 0; c < target_size.col; ++c) {
                placements(r, c).reset(new PrQueue(pl_comp)); 
            }
        }
        
        for (auto r = 0; r < target_size.row; ++r) {
            for (auto c = 0; c < target_size.col; ++c) {
                for (auto h = 1; h <= max_source_size.row; ++h) {
                    for (auto w = 1; w <= max_source_size.col; ++w) {
                        for (auto i = 0; i < kSourceImageCount; ++i) {
                            Index index = source_score.score_id({r, c}, {h, w}, i);
                            if (!source_score.isInitialized(index)) continue;
                            for (auto rr = r; rr < r+h; ++rr) {
                                for (auto cc = c; cc < c+w; ++cc) {
                                    if (placements(rr, cc)->size()
                                        < kPlacementsPerCell ||
                                        source_score.score(placements(rr, cc)->top())
                                        > source_score.score(index)) {
                                        placements(rr, cc)->pop();
                                        placements(rr, cc)->push(i);
                                    }
                                }
                            } 
                        } 
                    }
                }
            }
        }
        vector<Index> all_indices;
        all_indices.reserve(target_size_.cell_count()*kPlacementsPerCell);
        for (auto& pq : placements) {
            while (!pq->empty()) {
                all_indices.push_back(pq->top());
                pq->pop();
            }
        } 
        sort(all_indices.begin(), all_indices.end());
        source_placement_queue_.reserve(target_size_.cell_count()*kPlacementsPerCell);
        source_placement_queue_.push_back(all_indices[0]);
        for (auto i = 1; i < all_indices.size(); ++i) {
            if (all_indices[i-1] == all_indices[i]) continue;
            source_placement_queue_.push_back(all_indices[i]); 
        }
        source_placement_queue_.shrink_to_fit();
        sort(source_placement_queue_.begin(), source_placement_queue_.end(), pl_comp);
    }
    
    
    virtual map<Index, Region> compose(const grid_approx::SourceScore& source_score, const Size& target_size) override {
        target_size_ = target_size;
        //double score = ant::linalg::sum(best_coverage_)/(25*25*target_size.cell_count());
        //cerr << "perfect score: " << sqrt(score) << endl;
        initQueue(source_score, target_size, {4, 4});
        map<Index, Region> best_result;
        double best_score = numeric_limits<double>::max();
        
        vector<Index> source_queue_indices(source_queue_.size());
        iota(source_queue_indices.begin(), source_queue_indices.end(), 0);
        vector<double> source_queue_values(source_queue_.size());
        
        
        Count kSufferingCount = 10;
        
        // source_placement_queue_
        // source_values
        
        Count iteration_count = 100;
        for (auto iteration = 0; iteration < iteration_count; ++iteration) {
            // source_placement_queue_ should already be sorted
            
            vector<bool> used(kSourceImageCount, false);
            Grid<char> covering(target_size);
            covering.fill(false);
            
            Grid<double> cell_score(target_size);
            // indices used on current iteration
            vector<Index> queue_indices;
            Int covered_area = 0;
            double score = 0;
            for (auto i = 0; i < source_placement_queue_.size(); ++i) {
                auto pl = source_score.placement(source_placement_queue_[i]);
                // can use
                if (used[p.i]) continue;
                for (auto r = pl.position.row; r < pl.positon.row + pl.size.row; ++r) {
                    for (auto c = pl.position.col; c < pl.position.col + pl.size.col; ++c) {
                        if (covering(r, c)) goto next;
                    }
                }
                for (auto r = pl.position.row; r < pl.positon.row + pl.size.row; ++r) {
                    for (auto c = pl.position.col; c < pl.position.col + pl.size.col; ++c) {
                        covering(r, c) = true;
                        cell_score(r, c) = source_score.score(source_placement_queue_[i])/pl.size.cell_count();
                    }
                }
                queue_indices.push_back(i);
                covered_area += p.s.cell_count();
                score += p.score;
                used[p.i] = true;
                
                if (covered_area == target_size.cell_count()) break;
            }
            
            // now we got solution. time to weight it
        }
        
    
            
            for (auto i : queue_indices) {
                auto q_i = source_queue_indices[i];
                auto &q_pl = source_queue_[q_i];
                double cur_score = q_pl.score/q_pl.s.cell_count();
                const Count kNext = 10;
                Count cur_count = 0;
                double other_score = 0;
                for (auto i_next = i+1; i_next < source_queue_indices.size() && cur_count < kNext; ++i_next) {
                    if (q_pl.i != source_queue_[source_queue_indices[i_next]].i) continue;
                    auto& next_pl = source_queue_[source_queue_indices[i_next]];
                    double next_score = -next_pl.score;
                    for (auto r = next_pl.p.row; r < next_pl.p.row+next_pl.s.row; ++r) {
                        for (auto c = next_pl.p.col; c < next_pl.p.col+next_pl.s.col; ++c) {
                            next_score += cell_score(r, c);
                        }
                    }
                    if (next_score > 0) {
                        other_score += next_score;
                        cur_count += 1; 
                    }
                }
                if (cur_count == 0) continue;
                other_score +=  
            }
            
            
            
            if (covered_area == target_size.cell_count() && best_score > score) {
                best_score = score;
                best_result = result;
            }
        }
        score_ = best_score;
        return best_result;
    }
    
    vector<Index> source_placement_queue_;

};





struct TabuSearch : Base {
    
    // located in candidate list
    struct Move {
        Move() {}
        Move(Index i, const Size& s) : source_index(i), size(s) {}
        Index source_index;
        Size size;
    };
    
    struct MoveLocation {
        MoveLocation() {}
        MoveLocation(const Position& p, Index i) : position(p), index(i) {}
        Position position;
        Index index;
    };
    
    struct Solution {
        vector<MoveLocation> moves;
        double score;
    };
    
    // first argument is iteration in which tabu is down
    using TabuItem = pair<Count, MoveLocation>;
    struct TabuItemComp {
        bool operator()(const TabuItem& t_0,const TabuItem& t_1) {
            return t_0.first > t_1.first;
        }
    };
    
    double score(const MoveLocation& ml) {
        auto& m = move(ml);
        return (*source_score_)[m.source_index](m.size.row-1, m.size.col-1)(ml.position);
    } 
    
    double score(const Position& p, const Move& m) {
        return (*source_score_)[m.source_index](m.size.row-1, m.size.col-1)[p];
    } 
    
    const Move& move(const MoveLocation& ml) {
        return candidate_list_[ml.position][ml.index];
    }
    
    void initCandidateList() {
        auto& source_score = *source_score_;
        candidate_list_.resize(target_size_);
        candidate_tabu_.resize(target_size_);
        for (auto r = 0; r < target_size_.row; ++r) {
            for (auto c = 0; c < target_size_.col; ++c) {
                candidate_list_(r, c).reserve(kCandPerPosUsing);
            }
        }
        vector<Move> local_candidates(kCandPerPosExists);
        for (auto r = 0; r < target_size_.row; ++r) {
            for (auto c = 0; c < target_size_.col; ++c) {
                local_candidates.clear();
                for (Index i = 0; i < kSourceImageCount; ++i) {
                    auto& ss = source_score[i]; 
                    for (auto h = 0; h < min(target_size_.row-r, (Int)ss.row_count()); ++h) {
                        for (auto w = 0; w < min(target_size_.col-c, (Int)ss.col_count()); ++w) {
                            local_candidates.emplace_back(i, Size{h+1, w+1});
                        }
                    }
                }
                sort(local_candidates.begin(), local_candidates.end(), 
                    [=,&source_score](const Move& m_0, const Move& m_1) {
                        return source_score[m_0.source_index](m_0.size.row-1, m_0.size.col-1)(r, c)/(25*25*m_0.size.cell_count()) 
                                - 10*m_0.size.cell_count() < 
                                source_score[m_1.source_index](m_1.size.row-1, m_1.size.col-1)(r, c)/(25*25*m_1.size.cell_count())
                                - 10*m_1.size.cell_count();
                });
                auto& cand = candidate_list_(r, c); 
                cand.assign(local_candidates.begin(), 
                            local_candidates.begin()+min(kCandPerPosUsing, local_candidates.size()));
                candidate_tabu_(r, c).resize(cand.size(), false);
            }
        }
    }
    
    // i doesn't go inside
    Solution rebuildFrom(const Solution& sol, Index i) {
        Grid<char> covering(target_size_);
        array<char, kSourceImageCount> used;
        used.fill(false);
        covering.fill(false);
        Solution res;
        res.moves.assign(sol.moves.begin(), sol.moves.begin()+i);
        res.score = 0;
        for (auto j = 0; j < i; ++j) {
            auto& ml = sol.moves[j];
            auto& m = move(ml);
            used[m.source_index] = true;
            for (auto r = ml.position.row; r < ml.position.row+m.size.row; ++r) {
                for (auto c = ml.position.col; c < ml.position.col+m.size.col; ++c) {
                    covering(r, c) = true;
                }
            }
            res.score += score(ml);
        }
        Position pos = sol.moves[i].position;
        for (auto r = pos.row; r < target_size_.row; ++r) {
            for (auto c = r == pos.row ? pos.col : 0; c < target_size_.col; ++c) {
                if (covering(r, c)) continue;
                auto c_sz = 1;
                while (c + c_sz < target_size_.col && !covering(r, c + c_sz)) ++c_sz;
                auto& cand = candidate_list_(r, c);
                auto& tabu = candidate_tabu_(r, c);
                bool found = false;
                for (auto i = 0; i < cand.size(); ++i) {
                    auto& m = cand[i];
                    if (used[m.source_index] || tabu[i] || m.size.col > c_sz) continue;
                    for (auto r_0 = r; r_0 < r+m.size.row; ++r_0) {
                        for (auto c_0 = c; c_0 < c+m.size.col; ++c_0) {
                            covering(r_0, c_0) = true;
                        }
                    }
                    used[m.source_index] = true;
                    res.score += score(Position{r, c}, m);
                    res.moves.emplace_back(Position{r, c}, i);
                    c += m.size.col-1;
                    found = true;
                    break;
                }
                assert(found);
            }
        }
        return res;  
    }
    
    void searchNeighborhood() {
        Count n = neighborhood_distribution_(RNG);
        //uniform_int_distribution<Index> dd(0, current_solution_.moves.size()-1);
        vector<Index> vv(current_solution_.moves.size());
        iota(vv.begin(), vv.end(), 0);
        //vv.reverse();
        discrete_distribution<> dd(vv.rbegin(), vv.rend());
        
        Solution best_neighbor;
        best_neighbor.score = numeric_limits<double>::max();
        vector<MoveLocation> tabus(n);
        //if (best_solution_.score < 10000000) n = current_solution_.moves.size()*best_solution_.score/current_solution_.score;
        for (auto i = 0; i < n; ++i) {
             auto m_i = dd(RNG);
             auto ml = current_solution_.moves[m_i];
             tabus[i] = ml;
             tabu(ml);
             Solution s = rebuildFrom(current_solution_, m_i);
             unTabu(ml);
             if (s.score < best_neighbor.score) {
                 best_neighbor = std::move(s);
             }
        }
        for (auto& ml : tabus) {
            tabu(ml);
        }
        if (best_neighbor.score < best_solution_.score) {
            best_solution_ = best_neighbor;
        }
        current_solution_ = std::move(best_neighbor);
    }
    
    void tabu(const MoveLocation& ml) {
        auto n = tabu_iterations_distribution_(RNG);
        tabu_queue_.emplace(iteration_+n, ml);
        candidate_tabu_[ml.position][ml.index] = true;
    }
    
    void unTabu(const MoveLocation& ml) {
        candidate_tabu_[ml.position][ml.index] = false;
    }
    
    void updateTabus() {
        while (!tabu_queue_.empty() && tabu_queue_.top().first <= iteration_) {
            auto& ml = tabu_queue_.top().second;
            candidate_tabu_[ml.position][ml.index] = false;
            tabu_queue_.pop();
        }
    }
    
    virtual map<Index, Region> compose(const grid_approx::SourceScore& source_score, const Size& target_size) override {
        source_score_ = &source_score;
        target_size_ = target_size;
        
        initCandidateList();
        iteration_ = 0;
        clock_t t = clock();
        
        current_solution_.moves.emplace_back(Position{0, 0}, 0);
        best_solution_ = current_solution_ = rebuildFrom(current_solution_, 0);
        while ((clock() - t)/CLOCKS_PER_SEC < 7) {
            searchNeighborhood();
            updateTabus();
            ++iteration_;
        }      
        //cerr << "it: " << iteration_ << endl;
        map<Index, Region> result;
        for (auto& ml : best_solution_.moves) {
            auto& m = move(ml);
            result[m.source_index].set(ml.position, m.size);
        }
        return result;
    }
    
    int  score() const override {
        return 0;
    }
    
    
    Solution current_solution_;
    Solution best_solution_;
    const Count kMinTabuIterations = 100;
    const Count kMaxTabuIterations = 200;
    const Count kMinNeighborhood = 10;
    const Count kMaxNeighborhood = 30;
    const Count kCandPerPosExists = kSourceImageCount*5*5;
    const Count kCandPerPosUsing = 10000; 
    Size target_size_;
    Grid<vector<Move>> candidate_list_;
    Grid<vector<char>> candidate_tabu_;
    // where i find it?
    priority_queue<
        TabuItem, 
        vector<TabuItem>, 
        TabuItemComp> tabu_queue_{TabuItemComp()};   
    const SourceScore* source_score_; 
    uniform_int_distribution<Count> neighborhood_distribution_{kMinNeighborhood, kMaxNeighborhood};
    uniform_int_distribution<Count> tabu_iterations_distribution_{kMinTabuIterations, kMaxTabuIterations};
    Index iteration_{0};
};









}

}

}

#endif



