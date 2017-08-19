//
//  genetic.hpp
//  CollageMaker
//
//  Created by Anton Logunov on 2/22/15.
//
//

#ifndef CollageMaker_genetic_hpp
#define CollageMaker_genetic_hpp


#include "grid_approx.hpp"
#include "solver.hpp"


namespace collage_maker {
    
struct Genetic {
    
    vector<Placement> layout(const GridSourceScore& source) {
        using Pair = pair<Region, GridSourceScore::Item>;
        
        vector<Placement> result;
        vector<Pair> places;  
        for (const auto& p : source) {
            places.push_back(p);
        }
        double factor = 1.5;
        sort(places.begin(), places.end(), [=](const Pair& p_0, const Pair& p_1) {
            return p_0.second.score/pow(p_0.first.cell_count(), factor) 
                    < p_1.second.score/pow(p_1.first.cell_count(), factor);
        });
        
        // create population array of vector<Pair> places
        
        // sum all scores in each sample to get which child is better
        
        // crossing : take two best results and connect one by one from one, then from another
         
        // mutation : 
        
    
    }

};


}

#endif
