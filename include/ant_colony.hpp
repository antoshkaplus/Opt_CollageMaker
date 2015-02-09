//
//  ant_colony_opt.h
//  CollageMaker
//
//  Created by Anton Logunov on 7/3/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __ANT_COLONY__H_
#define __ANT_COLONY__H_


#include "grid_approx.h"


namespace collage_maker {
    
namespace grid_approx {
    
namespace algorithm {


struct AntColony : Base {
    struct BaseMove {
        Index index;
        Size size;
    };
    
    struct Move : BaseMove {
        Move() {}
        Move(const Position& p, const BaseMove& b)
        : position(p), BaseMove(b) {}
        Position position;
    };
    
    struct Storage : vector<double> {
        Storage() : vector<double>(12*12*4*4*200) {}
        using vector<double>::operator[];
    };
    
    static const Index 
    kTopLeftGoesRight   = 0,
    kTopLeftGoesDown    = 1,
    kTopRightGoesLeft   = 2,
    kTopRightGoesDown   = 3,
    kBotLeftGoesRight   = 4,
    kBotLeftGoesUp      = 5,
    kBotRightGoesLeft   = 6,
    kBotRightGoesUp     = 7;
    
    static const Index
    kTopLeftCorner      = 0,
    kTopRightCorner     = 1,
    kBottomLeftCorner   = 2,
    kBottomRightCorner  = 3;
    
    static const Count kKindCount = 8;
    static const Count kCornerCount = 4;

    using CandidateList = Grid<vector<BaseMove>>;

    struct System {
        struct Candidate {
            Candidate() {}
            Candidate(const BaseMove& move, double score) 
            : move(move), score(score) {}
            
            bool operator<(const Candidate& c) const {
                return score/move.size.cell_count() < c.score/c.move.size.cell_count();
            }
            
            BaseMove move;
            double score;
        };
        
        static const size_t factor_0 = kSourceImageCount*4*4*12;
        static const size_t factor_1 = factor_0/12;
        static const size_t factor_2 = factor_1/4;
        static const size_t factor_3 = factor_2/4;
        
        struct Interface {
            Interface(System& system, const Size& target_size)
             :  system_(system), target_size_(target_size), 
                fnTo_(CoordinateTransformation::toTopLeftGoesRight), 
                fnFrom_(CoordinateTransformation::fromTopLeftGoesRight) {}
            Interface(System& system)
             : Interface(system, system.target_size_) {}
            
            virtual Index index(const Move& m) const {
                return factor_0*m.position.row + factor_1*m.position.col 
                        + factor_2*(m.size.row-1) + factor_3*(m.size.col-1) + m.index;
            }
            
            Move convertFrom(Move m) const { 
                Region t = fnFrom_(Region(m.position, m.size), target_size_);
                m.position = t.position;
                m.size = t.size;
                return m;
            }
            
            Move convertTo(Move m) const {
                Region t = fnTo_(Region(m.position, m.size), system_.target_size_);
                m.position = t.position;
                m.size = t.size;
                return m;
            }
            
            double& total(const Move& m) {
                return system_.total_[index(m)];
            }
            double& pheromone(const Move& m) {
                return system_.total_[index(m)];
            }
            double& score(const Move& m) {
                return system_.score_[index(m)];
            } 
            
            Size target_size() {
                return target_size_;
            }
            
            Region (*fnTo_)(const Region&, const Size&);
            Region (*fnFrom_)(const Region&, const Size&);
            Size target_size_;
            System& system_;
        };
        
        using TopLeftGoesRightInterface = Interface;
        
        struct InterfaceDescendant : Interface {
            using Interface::Interface;
            
            Index index(const Move& m) const override {
                Region reg = fnFrom_(Region(m.position, m.size), target_size_);
                return Interface::index(Move(reg.position, BaseMove{m.index, reg.size})); 
            }
        };
        
        struct TopLeftGoesDownInterface : InterfaceDescendant {
            TopLeftGoesDownInterface(System& system)
            : InterfaceDescendant(system, system.target_size_.swapped()) {
                fnTo_ = CoordinateTransformation::toTopLeftGoesDown;
                fnFrom_ = CoordinateTransformation::fromTopLeftGoesDown;
            }
        };
        
        struct TopRightGoesLeftInterface : InterfaceDescendant {
            TopRightGoesLeftInterface(System& system) 
            : InterfaceDescendant(system) {
                fnTo_ = CoordinateTransformation::toTopRightGoesLeft;
                fnFrom_ = CoordinateTransformation::fromTopRightGoesLeft;
            }
        };
        struct TopRightGoesDownInterface : InterfaceDescendant {
            TopRightGoesDownInterface(System& system) 
            : InterfaceDescendant(system, system.target_size_.swapped()) {
                fnTo_ = CoordinateTransformation::toTopRightGoesDown;
                fnFrom_ = CoordinateTransformation::fromTopRightGoesDown;
            }
        };
        struct BotLeftGoesRightInterface : InterfaceDescendant {
            BotLeftGoesRightInterface(System& system) 
            : InterfaceDescendant(system) {
                fnTo_ = CoordinateTransformation::toBotLeftGoesRight;
                fnFrom_ = CoordinateTransformation::fromBotLeftGoesRight;
            }
        };
        struct BotLeftGoesUpInterface : InterfaceDescendant {
            BotLeftGoesUpInterface(System& system) 
            : InterfaceDescendant(system, system.target_size_.swapped()) {
                fnTo_ = CoordinateTransformation::toBotLeftGoesUp;
                fnFrom_ = CoordinateTransformation::fromBotLeftGoesUp;
            }
        };
        struct BotRightGoesLeftInterface : InterfaceDescendant {
            BotRightGoesLeftInterface(System& system) 
            : InterfaceDescendant(system) {
                fnTo_ = CoordinateTransformation::toBotRightGoesLeft;
                fnFrom_ = CoordinateTransformation::fromBotRightGoesLeft;
            }
        
        };
        struct BotRightGoesUpInterface : InterfaceDescendant {
            BotRightGoesUpInterface(System& system)
            : InterfaceDescendant(system, system.target_size_.swapped()) {
                fnTo_ = CoordinateTransformation::toBotRightGoesUp;
                fnFrom_ = CoordinateTransformation::fromBotRightGoesUp;
            }
            
        };
        
        struct Ant {
            Ant(const CandidateList& candidate_list, Interface& interface) 
            :   candidate_list_(candidate_list),
                interface_(interface),
                target_size_(interface.target_size()) {
                
                covering_.resize(target_size_);
            }
            
            // should somehow handle return 
            Move assignNextMove() {
                while (covering_(next_position_)) {
                    if (next_position_.col == target_size_.col-1) {
                        ++next_position_.row;
                        next_position_.col = 0;
                    } else ++next_position_.col;
                }
                // should make restriciton on col count
                auto col_count = 1;
                while (next_position_.col + col_count < target_size_.col 
                       && !covering_(next_position_.row, next_position_.col + col_count)) ++col_count; 
                // search candidate list now
                Move best_move;
                double best_total = -1, total;
                if (distribution_(RNG) < best_move_rate_) {            
                    for (auto& move : candidate_list_(next_position_)) {
                        if (used_sources_[move.index]
                            || col_count < move.size.col) continue; 
                        total = interface_.total(Move(next_position_, move)); 
                        if (total > best_total) {
                            best_total = total;
                            static_cast<BaseMove&>(best_move) = move;
                        }
                    }
                } else {
                    // partial sum and shit
                    vector<const BaseMove*> moves;
                    vector<double> totals;
                    for (auto& move : candidate_list_(next_position_)) {
                        if (used_sources_[move.index] 
                            || col_count < move.size.col) continue;
                        moves.push_back(&move);
                        totals.push_back(interface_.total(Move(next_position_, move)));
                    }
                    discrete_distribution<> distr(totals.begin(), totals.end());
                    static_cast<BaseMove&>(best_move)  = *(moves[distr(RNG)]);
                }
                best_move.position = next_position_;
                next_move_ = best_move;
                return best_move;
            }
            
            bool didFinish() { 
                // need to know target area
                return covered_area_ == target_size_.cell_count();  
            }
            void performNextMove() {
                score_ += interface_.score(Move(next_position_, next_move_));
                Region reg(next_position_, next_move_.size);
                for (auto r = reg.row_begin(); r < reg.row_end(); ++r) {
                    for (auto c = reg.col_begin(); c < reg.col_end(); ++c) {
                        covering_(r, c) = true;
                    }
                }
                covered_area_ += reg.size.row*reg.size.col;
                collage_.push_back(Move(next_position_, next_move_));
                used_sources_[next_move_.index] = true;
                // next positon is unavailable
                next_position_.col = reg.col_end()-1;
            }
            
            vector<Move> collage() {
                vector<Move> collage;
                collage.reserve(collage_.size());
                for (auto m : collage_) {
                    collage.push_back(interface_.convertFrom(m));
                }
                return collage;
            }
            
            double score() {
                return score_;
            }
            
            void prepare() {
                covering_.fill(false);
                used_sources_.fill(false);
                next_position_.set(0, 0);
                collage_.clear();
                covered_area_ = 0;
                score_ = 0;
            }
            
            // wont be able to create in an array?
            // but possible copying because won't change
            double score_;
            // big total is good
            // should be sorted by total // no sorting apparantly
            Grid<char> covering_;
            array<bool, kSourceImageCount> used_sources_;
            
            //bool next_move_ready_{false};
            BaseMove next_move_;
            Position next_position_;
            
            uniform_real_distribution<> distribution_;
            vector<Move> collage_;
            double best_move_rate_{0.95};
            size_t covered_area_{0};
            
            Size target_size_;
            const CandidateList& candidate_list_;
            Interface& interface_;
        };

        System(const SourceScore& source_score, const Size& target_size) : target_size_(target_size) {
            // can't avoid this long assignment
            interface_[kTopLeftGoesRight] = move(unique_ptr<Interface>(new TopLeftGoesRightInterface(*this)));
            interface_[kTopLeftGoesDown] = move(unique_ptr<Interface>(new TopLeftGoesDownInterface(*this)));
            
            interface_[kTopRightGoesLeft] = move(unique_ptr<Interface>(new TopRightGoesLeftInterface(*this)));
            interface_[kTopRightGoesDown] = move(unique_ptr<Interface>(new TopRightGoesDownInterface(*this)));
            
            interface_[kBotLeftGoesRight] = move(unique_ptr<Interface>(new BotLeftGoesRightInterface(*this)));
            interface_[kBotLeftGoesUp] = move(unique_ptr<Interface>(new BotLeftGoesUpInterface(*this)));
            
            interface_[kBotRightGoesLeft] = move(unique_ptr<Interface>(new BotRightGoesLeftInterface(*this)));
            interface_[kBotRightGoesUp] = move(unique_ptr<Interface>(new BotRightGoesUpInterface(*this)));
            
            // all corners have right orientation
            vector<Index> corner_kinds = {kTopLeftGoesRight, kTopRightGoesLeft, kBotLeftGoesRight, kBotRightGoesLeft};
            array<Grid<vector<Candidate>>, kCornerCount> candidates;
            for (auto& cand : candidates) {
                cand.resize(target_size);
                for (auto r = 0; r < target_size.row; ++r) {
                    for (auto c = 0; c < target_size.col; ++c) {
                        cand(r, c).reserve(4*4*200);
                    }
                }
            }
            for (auto r = 0; r < target_size.row; ++r) {
                for (auto c = 0; c < target_size.col; ++c) {
                    for (Index i = 0; i < kSourceImageCount; ++i) {
                        auto& ss = source_score[i]; 
                        for (auto h = 0; h < min(target_size.row-r, (Int)ss.row_count()); ++h) {
                            for (auto w = 0; w < min(target_size.col-c, (Int)ss.col_count()); ++w) {
                                Region t, s(r, c, h+1, w+1);
                                // orientation helps a lot
                                Move m_t, m_s(s.position, BaseMove{i, {h+1, w+1}});
                                double sc = ss(h, w)(r, c);
                                for (auto k = 0; k < corner_kinds.size(); ++k) {
                                    m_t = interface_[corner_kinds[k]]->convertTo(m_s);
                                    candidates[k](m_t.position).emplace_back(BaseMove{i, m_t.size}, sc);
                                }
                            }
                        }
                    }
                }
            }
            // also we need to initialize score (other guys can wait)
            // and make so would be possible to iterate
            set<Index> candidate_id_set;
            vector<Move> candidate_moves;
            for (auto k = 0; k < corner_kinds.size(); ++k) {
                for (auto r = 0; r < candidates[k].row_count(); ++r) {
                    for (auto c = 0; c < candidates[k].col_count(); ++c) {
                        auto& cands = candidates[k](r, c);
                        auto n = min(kPositionCandidateCount, cands.size());
                        nth_element(cands.begin(), cands.end(), cands.begin()+n-1);
                        cands.resize(n);
                        // time to initialize score
                        for (auto i = 0; i < cands.size(); ++i) {
                            auto j = interface_[corner_kinds[k]]->index(Move({r, c}, cands[i].move));
                            if (candidate_id_set.insert(j).second) {
                                value_[j] = score_[j] = cands[i].score;
                                value_[j] /= cands[i].move.size.cell_count();
                                candidate_moves.push_back(
                                    interface_[corner_kinds[k]]->convertFrom(
                                        Move({r, c}, cands[i].move)));
                            }
                        }
                    } 
                }
            }
            candidate_id_.insert(candidate_id_.end(), candidate_id_set.begin(), candidate_id_set.end());
            // need to get move from id and pass for every candidate_list_
            for (auto k = 0; k < kKindCount; ++k) {
                candidate_list_[k].resize(interface_[k]->target_size());
                for (auto& m : candidate_moves) {
                    auto mm = interface_[k]->convertTo(m);
                    candidate_list_[k](mm.position).push_back(mm);
                } 
            }
        }
        
        void initPheromone(double d) {
            for (auto i : candidate_id_) {
                pheromone_[i] = d;
            }
        }
        
        void updateTotal() {
            for (auto i : candidate_id_) {
                total_[i] = pow(pheromone_[i], 1)*pow(value_[i], -3);
            }
        }
        
        void updateTotal(Index i) {
            total_[i] = pow(pheromone_[i], 1)*pow(value_[i], -3);
        }
        
        unique_ptr<Ant> makeAnt(Index kind) {
            return unique_ptr<Ant>(new Ant(candidate_list_[kind], *interface_[kind].get()));
        } 
        
        const Count kPositionCandidateCount = 400;
        array<CandidateList, kKindCount> candidate_list_; 
        array<unique_ptr<Interface>, kKindCount> interface_;
        // special index by move
        vector<Index> candidate_id_;
        Storage total_;
        Storage pheromone_;
        Storage value_;
        Storage score_;
        Size target_size_;
    };


    map<Index, Region> compose(const SourceScore& source_score, const Size& target_size) {
        System system(source_score, target_size);
        
        map<Index, Region> result;
        double best_score = numeric_limits<double>::max();
        
        // add interface and candidates from system... also need just interface... candidates can be gotten from interface
        vector<unique_ptr<System::Ant>> ants(kKindCount);
        for (auto i = 0; i < kKindCount; ++i) {
            ants[i] = system.makeAnt(i);
        }
        
        auto& a = *ants[0];
        a.prepare();
        while (!a.didFinish()) {
            a.assignNextMove();
            a.performNextMove();
        }
        double approximation = 1./(kSourceImageCount*kSourceImageCount*a.score());
        system.initPheromone(approximation);
        system.updateTotal();
        
        Count iteration = 0;
        clock_t t = clock();
        while ((clock() - t)/CLOCKS_PER_SEC < 7) {
            for (auto& a : ants) {
                a->prepare();
            }
            // change loop
            Count finished_ant_count = 0;
            while (finished_ant_count < ants.size()) {
                finished_ant_count = 0;
                for (auto& a : ants) {
                    if (a->didFinish()) {
                        ++finished_ant_count;
                        continue;
                    }
                    a->assignNextMove();
                    Move m(a->next_position_, a->next_move_);
                    double& phe = a->interface_.pheromone(m); 
                    phe = phe*(1-evaporation_rate) + evaporation_rate*approximation;
                    system.updateTotal(a->interface_.index(m));
                    a->performNextMove();
                }
            }
            // evaporation
            for (auto i : system.candidate_id_) {
                system.pheromone_[i] *= (1-evaporation_rate);
            }

            // now everyone updates portion of pheromon
            double best_ant_score = numeric_limits<double>::max();
            System::Ant* best_ant = nullptr;  
            for (auto& a : ants) {
                if (a->score() < best_score) {
                    best_score = a->score();
                    result.clear();
                    for (auto& m : a->collage()) {
                        result[m.index] = Region(m.position, m.size);
                    }
                }
                if (a->score() < best_ant_score) {
                    best_ant_score = a->score();
                    best_ant = a.get();
                }
                //goto finish;
                for (auto& m : a->collage_) {
                    a->interface_.pheromone(m) += 1./a->score();
                }
            }
            for (auto &m : best_ant->collage_) {
                best_ant->interface_.pheromone(m) += 1./best_ant_score;
            }
            system.updateTotal();
            ++iteration;
        }
        cerr << "it: " << iteration << endl; 
        finish:
        return result;
    }
    
    double evaporation_rate{0.1};
   // Count ant_count;
};


}

}

}

#endif