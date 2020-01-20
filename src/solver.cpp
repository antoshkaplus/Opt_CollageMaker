//
//  solver.cpp
//  CollageMaker
//
//  Created by Anton Logunov on 7/2/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <array>

#include "solver.hpp"

namespace collage_maker {

default_random_engine RNG((unsigned)time(nullptr));



ant::grid::Position scalePosition(const ant::grid::Position& original_p, const Size& origional_size, const Size& size) {
    return Position(((double)original_p.row + 0.5)*(size.row)/(origional_size.row), 
                    ((double)original_p.col + 0.5)*(size.col)/(origional_size.col));
}


int scaleSmart(const Position& original_p, const Size& original_s, const Mat& m) {
    const double 
    x_0_or = original_p.col,
    y_0_or = original_p.row,
    x_1_or = x_0_or + 1,
    y_1_or = y_0_or + 1;
    Size 
    s_or = original_s,
    s = m.size();
    const double 
    x_0 = x_0_or*s.col/s_or.col,
    y_0 = y_0_or*s.row/s_or.row,
    x_1 = x_1_or*s.col/s_or.col,
    y_1 = y_1_or*s.row/s_or.row;
    
    const Index 
    x_0_c = ceil(x_0),
    y_0_c = ceil(y_0);
    const Index 
    x_0_f = floor(x_0),
    y_0_f = floor(y_0),
    x_1_f = floor(x_1),
    y_1_f = floor(y_1); 
    
    double weight;
    double total_color = 0;
    double total_weight = 0;

    // inner whole cells
    for (auto r = y_0_c; r < y_1_f; ++r) {
        for (auto c = x_0_c; c < x_1_f; ++c) {
            total_color += m(r, c);
            total_weight += 1.;
        }
    }
    
    // top left corner
    weight = (y_0_c - y_0)*(x_0_c - x_0);
    total_color += m(y_0_f, x_0_f)*weight;
    total_weight += weight;
    
    // left side
    for (auto r = y_0_c; r < y_1_f; ++r) {
        weight = 1.*(x_0_c - x_0);
        total_color += m(r, x_0_f)*weight;
        total_weight += weight;
    }
    
    if (x_1_f < m.col_count()) {
        // right side
        for (auto r = y_0_c; r < y_1_f; ++r) {
            weight = 1.*(x_1 - x_1_f);
            total_color += m(r, x_1_f)*weight;
            total_weight += weight;
        }
        // top right corner
        weight = (y_0_c - y_0)*(x_1 - x_1_f);
        total_color += m(y_0_f, x_1_f)*weight;
        total_weight += weight;
    }
    
    for (auto c = x_0_c; c < x_1_f; ++c) {
        // top
        weight = 1.*(y_0_c - y_0);
        total_color += m(y_0_f, c)*weight;
        total_weight += weight;
    }
    
    if (y_1_f < m.row_count()) {
        // bottom side
        for (auto c = x_0_c; c < x_1_f; ++c) {
            weight = 1.*(y_1 - y_1_f);
            total_color += m(y_1_f, c)*weight;
            total_weight += weight;
        }
        // bottom left 
        weight = (y_1 - y_1_f)*(x_0_c - x_0);
        total_color += m(y_1_f, x_0_f)*weight;
        total_weight += weight;
    }
    
    // bottom right
    if (y_1_f < m.row_count() && x_1_f < m.col_count()) {
        weight = (x_1 - x_1_f)*(y_1 - y_1_f);
        total_color += m(y_1_f, x_1_f)*weight;
        total_weight += weight;
    }
    return round(total_color/total_weight);
}

Mat scaleSmart(const Mat& source, const Size& size) {
    Mat result(size);
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            result(r, c) = scaleSmart({r, c}, size, source);
        }
    }
    return result;
}

Mat scaleSilly(const Mat& source, const Size& size) {
    Mat result(size);
    // try to use sse here
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            result(r, c) = source[scalePosition({r, c}, size, source.size())];
        }
    }
    return result;
}


Mat scaleSilly_2(const Mat& source, const Size& size) {
    Mat result(size);
    static vector<int> rs;
    static vector<int> cs;
    rs.resize(size.row);
    cs.resize(size.col);
    for (auto r = 0; r < size.row; ++r) {
        rs[r] = (r + 0.5)*(source.row_count())/(size.row);
    }
    for (auto c = 0; c < size.col; ++c) {
        cs[c] = (c + 0.5)*(source.col_count())/(size.col);
    }
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            result(r, c) = source(rs[r], cs[c]);
        }
    }
    return result;
}


Mat scalePrecise(const Mat& source, const Size& size) {
    Size original_size = source.size();
    Mat 
    res(size), 
    inter_res(original_size.row*size.row, 
              original_size.col*size.col);
    for (auto r = 0; r < original_size.row; ++r) {
        for (auto c = 0; c < original_size.col; ++c) {
            inter_res.submat(r*size.row, c*size.col, size.row, size.col) 
            = source(r, c);
        }
    }
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            res(r, c) = round(ant::linalg::sum(inter_res.submat(r*original_size.row, c*original_size.col, original_size.row, original_size.col))/(double)(original_size.row*original_size.col));
        }
    }
    return res;
}


int score(const Mat& source, const Mat& target) {
    return ant::linalg::sum((source - target)%(source - target));
}

// TODO takes 71% of execution
int score(const Mat& source, const MatView& target_view) {
    return ant::linalg::sum((source - target_view)%(source - target_view));
}


int score(const Mat& source, const Position& pos, const Mat& target) {
    return score(source, MatView(target, ant::linalg::Region(pos, source.size())));
}


int scoreSmart(const map<Index, Region>& regions, const array<Mat,kSourceImageCount>& source, const Mat& target) {
    double total_score = 0;
    for (auto& p : regions) {
        Mat t = scaleSmart(source[p.first], p.second.size);
        total_score += score(t, p.second.position, target);
    }
    return total_score;
}


int scoreSilly(const map<Index, Region>& regions, const array<Mat,kSourceImageCount>& source, const Mat& target) {
    double total_score = 0;
    for (auto& p : regions) {
        Mat t = scaleSilly(source[p.first], p.second.size);
        total_score += score(t, p.second.position, target);
    }
    return total_score;
}


vector<Region> scaleInnerRegions(vector<Region>& original_regions, Size original_size, Size size) {
    Mat original_mat(original_size);
    Mat mat(size);
    mat.fill(0);
    for (size_t i = 0; i < original_regions.size(); ++i) {
        assert(Region(0, 0, original_size.row, original_size.col).hasInside(original_regions[i]));
        original_mat[original_regions[i]] = i;
    }
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            Position p = scalePosition(Position(r, c), size, original_size); 
            assert(Region(0, 0, original_size.row, original_size.col).hasInside(p));
            mat(r, c) = original_mat[p];
        }
    }
    
    vector<Region> regions(original_regions.size());
    vector<bool> created(original_regions.size(), false);
    for (auto r = 0; r < size.row; ++r) {
        for (auto c = 0; c < size.col; ++c) {
            auto index = mat(r, c);
            if (created[index]) continue;
            auto row_count = 1;
            while (r + row_count < mat.row_count() && mat(r + row_count, c) == index) ++row_count;
            auto col_count = 1;
            while (c + col_count < mat.col_count() && mat(r, c + col_count) == index) ++col_count;
            regions[index].set(r, c, row_count, col_count);
            created[index] = true;
        }
    }
    for (size_t i = 0; i < regions.size(); ++i) {
        if (created[i]) continue;
        regions[i].size = Size{0, 0};
    }
    
    //out_collage << mat;
    return regions;
}


void initData(const vector<int>& data, array<Mat, kSourceImageCount>& source, Mat& target) {
    auto start = data.begin();
    target = readImage(start);
    for (size_t k = 0; k < kSourceImageCount; ++k) {
        source[k] = readImage(start);
    }
}


vector<int> formatCollage(const map<Index, Region>& regions) {
    vector<int> result(kSourceImageCount*4, -1);
    for (auto p : regions) {
        auto i = p.first;
        const auto& r = p.second;
        if (r.isEmpty()) continue;
        result[i*4]   = r.row_begin();
        result[i*4+1] = r.col_begin();
        result[i*4+2] = r.row_end()-1;
        result[i*4+3] = r.col_end()-1;
    }
    return result;
}

vector<int> formatCollage(const vector<Item>& regions) {
    vector<int> result(kSourceImageCount*4, -1);
    for (auto p : regions) {
        auto i = p.index;
        const auto& r = p.region;
        if (r.isEmpty()) continue;
        result[i*4]   = r.row_begin();
        result[i*4+1] = r.col_begin();
        result[i*4+2] = r.row_end()-1;
        result[i*4+3] = r.col_end()-1;
    }
    return result;
}

bool isValid(vector<Item> items, Size target_size) {
    for (size_t i = 0; i < items.size(); ++i) {
        for (size_t j = i+1; j < items.size(); ++j) {
            if (items[i].region.hasIntersection(items[j].region)) {
                return false;
            }
        }
    }
    Mat filling(target_size);
    filling.fill(false);
    for (auto i : items) {
        for (auto p : i.region) {
            filling[p] = true;
        }
    }
    for (int r = 0; r < filling.row_count(); ++r) {
        for (int c = 0; c < filling.col_count(); ++c) {
            if (!filling(r, c)) {
                return false;
            }
        }
    }
    return true;
}

bool isValid(vector<Region> regions, Size target_size) {
    for (size_t i = 0; i < regions.size(); ++i) {
        for (size_t j = i+1; j < regions.size(); ++j) {
            if (regions[i].hasIntersection(regions[j])) {
                return false;
            }
        }
    }
    Mat filling(target_size);
    filling.fill(false);
    for (auto r : regions) {
        for (auto p : r) {
            filling[p] = true;
        }
    }
    for (int r = 0; r < filling.row_count(); ++r) {
        for (int c = 0; c < filling.col_count(); ++c) {
            if (!filling(r, c)) {
                return false;
            }
        }
    }
    return true;
}


}
