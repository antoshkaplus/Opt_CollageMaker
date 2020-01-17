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

    
struct Placement {
    Placement() {}
    Placement(const Position& p, const Size& s, Index i)  
    : position(p), size(s), source_index(i) {}
    Placement(const Region& r, Index i) 
    : position(r.position), size(r.size), source_index(i) {} 
    
    bool isMutuallyOpposite(const Placement& pl) {
        return source_index == pl.source_index || Region(position, size).hasIntersection(Region(pl.position, pl.size));
    }
    
    Position position;
    Size size;
    Index source_index;
};


struct GridSourceScore {
public:
    struct Item {
        // can use float
        double score;
        int index;
        
        bool operator==(const Item& item) {
            return score == item.score && index == item.index;
        }
    };

    class Iterator : std::iterator<std::input_iterator_tag, std::pair<Region, Item>> {
        Region current_region;
        const GridSourceScore& grid_;
        Index current_index;
        
        bool HasSize(const Region& region) {
            return grid_.items_[region.position].row_count() >= region.size.row 
                && grid_.items_[region.position].col_count() >= region.size.col;
        }
        
        bool HasPosition(const Region& region) {
            return grid_.items_.row_count() > region.position.row 
                && grid_.items_.col_count() > region.position.col;
        }
        
    public:
        Iterator(const GridSourceScore& grid, Region current_region, Index current_index) 
            : current_region(current_region), grid_(grid), current_index(current_index) {}
        
        pair<Region, Item> operator*() const { 
            return {current_region, grid_.items(current_region)[current_index]}; 
        }
        
        bool operator==(const Iterator& it) const {
            return current_index == it.current_index && (current_index == -1 || current_region == it.current_region); 
        }
        
        bool operator!=(const Iterator& it) const {
            return !(operator==(it));
        }
        
        Iterator& operator++() {
            if (current_index == -1) return *this;
            ++current_index;
            while (current_index != -1 && current_index == static_cast<decltype(current_index)>(grid_.items(current_region).size())) {
                current_index = 0;
                // need to take next current_region
                current_region.size.col += 1;
                if (HasSize(current_region)) {
                    continue;
                }
                current_region.size.col -= 1;
                
                current_region.size.row += 1;
                if (HasSize(current_region)) {
                    current_region.size.col = 1;
                    continue;
                }
                current_region.size.row -= 1;
                
                current_region.position.col += 1;
                if (HasPosition(current_region)) {
                    current_region.size.set(1, 1);
                    continue;
                }
                current_region.position.col -= 1;
                
                current_region.position.row += 1;
                if (HasPosition(current_region)) {
                    current_region.size.set(1, 1);
                    current_region.position.col = 0;
                    continue;
                }
                current_region.position.row -= 1;
                
                current_index = -1;
            }
            return *this;
        }
        // post iterator
        Iterator operator++(int) { 
            Iterator it(*this); 
            operator++(); 
            return it;
        }
    };
    friend class Iterator;

private:

    // position, size, Item
    Grid<Grid<vector<Item>>> items_;
 
public:
    
    GridSourceScore(Size target_size, Size max_source_size) {
        items_.resize(target_size);
        for (int r = 0; r < target_size.row; ++r) {
            for (int c = 0; c < target_size.col; ++c) {
                items_(r, c).resize(Size::min(max_source_size, target_size - Size{r, c}));
            }
        }
    }
    
    const Grid<Grid<vector<Item>>> items() const {
        return items_;
    }
    
    const vector<Item>& items(const Position& pos, const Size& size) const {
        return items_[pos](size.row - 1, size.col - 1);
    }
    
    const vector<Item>& items(const Region& region) const {
        return items(region.position, region.size);
    }
    
    void AddItem(const Position& pos, const Size& size, const Item& item) {
        items_[pos](size.row - 1, size.col - 1).push_back(item);
    }
    
    Iterator begin() const {
        if (items_.row_count() >= 1 && items_.col_count() >= 1 
            && items_(0, 0).row_count() >= 1 && items_(0, 0).col_count() >= 1 && items_(0, 0)(0, 0).size() >= 1) {
            return Iterator{*this, {0, 0, 1, 1}, 0};
        }
        return end();
    }
    
    Iterator end() const {
        return Iterator{*this, Region(), -1};
    }
    
    Size size() const {
        return items_.size();
    }
    
};


template<int kCellSize>
struct GridApprox {

    // to use this one we need to put inside target, cell size, sources,
    // should return new grid target SIZE, and sources data structure, with score 
    // for each size and location (placement)
    
    // target size inside
    GridSourceScore construct(const Mat& target, const SourceMats& source) {
        int t_h = ceil(1. * target.row_count() / kCellSize);
        int t_w = ceil(1. * target.col_count() / kCellSize);
        int s_max_h = 0;
        int s_max_w = 0;
        for (auto& s : source) {
            int h = s.row_count() / kCellSize;
            int w = s.col_count() / kCellSize;
            if (h > s_max_h) s_max_h = h;
            if (w > s_max_w) s_max_w = w;
        }
        GridSourceScore res({t_h, t_w}, {s_max_h, s_max_w});
        // should scale target first
        Mat t = scaleSmart(target, {t_h * kCellSize, t_w * kCellSize});
        for (size_t i = 0; i < source.size(); ++i) {
            int s_h = source[i].row_count() / kCellSize;
            int s_w = source[i].col_count() / kCellSize;
            for (int h = 1; h <= s_h; ++h) {
                for (int w = 1; w <= s_w; ++w) {
                    // also can compress some points *2 (use square of 2*2 as one)
                    // really bad for big matrices
                    // should consider to use stupid scale if 
                    //Mat s = scaleSilly_2(source[i], {h * kCellSize, w * kCellSize});
                    Mat s = scaleSmart(source[i], {h * kCellSize, w * kCellSize});
                    for (int r = 0; r < t_h - h + 1; ++r) {
                        for (int c = 0; c < t_w - w + 1; ++c) {
                            MatView tv(t, {r * kCellSize, c * kCellSize, h * kCellSize, w * kCellSize});
                            double score = collage_maker::score(s, tv);
                            res.AddItem({r, c}, {h, w}, {score, static_cast<int>(i)});
                        }
                    }
                }
            }
        }
        return res;
    }
    
    vector<Item> convert(const vector<Placement>& places, const Mat& target, const SourceMats& source) {
        vector<Region> rs;
        rs.reserve(places.size());
        for (auto p : places) {
            rs.emplace_back(p.position, p.size);
        };
        rs = scaleInnerRegions(rs, TargetGridSize(target.size()), target.size());
        vector<Item> items(rs.size());
        for (size_t i = 0; i < rs.size(); ++i) {
            items[i] = Item{places[i].source_index, rs[i]};
        }
        return items;
    }
    
private:
    Size TargetGridSize(Size sz) {
        return {static_cast<Int>(ceil(1. * sz.row / kCellSize)),
                static_cast<Int>(ceil(1. * sz.col / kCellSize))};
    }
};


}

#endif