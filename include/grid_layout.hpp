//
//  grid_layout.hpp
//  CollageMaker
//
//  Created by Anton Logunov on 2/20/15.
//
//

#ifndef CollageMaker_grid_layout_hpp
#define CollageMaker_grid_layout_hpp

#include "grid_approx.hpp"

namespace collage_maker {

struct GridLayout {
private:
    const double cell_count_factor_;
    
public:
    
    GridLayout(double cell_count_factor)
    : cell_count_factor_(cell_count_factor) {}

    vector<Placement> layout(const GridSourceScore& source) {
        using Pair = pair<Region, GridSourceScore::Item>;
        
        vector<Placement> result;
        vector<Pair> places;  
        for (const auto& p : source) {
            places.push_back(p);
        }
        double factor = cell_count_factor_;
        sort(places.begin(), places.end(), [=](const Pair& p_0, const Pair& p_1) {
            return p_0.second.score/pow(p_0.first.cell_count(), factor) 
                < p_1.second.score/pow(p_1.first.cell_count(), factor);
        });
        int empty_cell_count = source.size().cell_count();
        while (empty_cell_count > 0) {
            assert(!places.empty());
            Region r = places[0].first;
            Index i = places[0].second.index;
            result.emplace_back(r, i);
            empty_cell_count -= r.cell_count();
            auto it = remove_if(places.begin(), places.end(), [&](const Pair p) {
                return i == p.second.index || p.first.hasIntersection(r); 
            });
            places.erase(it, places.end());
        }
        return result;
    }

};


}

#endif
