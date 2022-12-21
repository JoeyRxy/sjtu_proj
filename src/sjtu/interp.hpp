#pragma once
#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>

#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "config.h"
#include "hmm/location.hpp"
#include "hmm/probability.hpp"

namespace rxy {

class ProbInterp {
   private:
    std::unordered_map<Point, std::unordered_map<RSRP_TYPE, Prob>> const& loc_prob_map;
    std::unordered_map<Point, std::map<RSRP_TYPE, Prob>>* points_to_interp;
    std::map<Point::value_type, int> x_map, y_map;
    std::vector<Point::value_type> x, y, z;

    gsl_spline2d* spline;
    gsl_interp_accel *x_acc, *y_acc;

    void dfs(std::unordered_map<Point, std::unordered_map<RSRP_TYPE, Prob>>::const_iterator iter,
             Prob _prob) {
        if (iter == loc_prob_map.end()) {
            gsl_spline2d_init(spline, x.data(), y.data(), z.data(), x.size(), y.size());
            for (auto&& [point, prob_map] : *points_to_interp) {
                prob_map[gsl_spline2d_eval(spline, point.x(), point.y(), x_acc, y_acc)] += _prob;
            }
        } else {
            auto ne = next(iter);
            int x_idx = x_map[iter->first.x()];
            int y_idx = y_map[iter->first.y()];
            for (auto&& [rsrp, prob] : iter->second) {
                gsl_spline2d_set(spline, z.data(), x_idx, y_idx, rsrp);
                dfs(ne, _prob * prob);
            }
        }
    }

   public:
    ProbInterp(std::unordered_map<Point, std::unordered_map<RSRP_TYPE, Prob>> const& loc_prob_map,
               bool use_cubic = true)
        : loc_prob_map(loc_prob_map) {
        std::size_t sz = 1;
        for (auto&& [loc, prob_map] : loc_prob_map) {
            x_map.emplace(loc.x(), 0);
            y_map.emplace(loc.y(), 0);
            std::cout << prob_map.size() << std::endl;
            sz *= prob_map.size();
            if (sz == 0) {
                std::ostringstream oss;
                oss << "prob map of point " << loc << " is empty";
                throw std::runtime_error(oss.str());
            } else if (sz > GetConfig().enumer_limit) {
                throw std::runtime_error("prob map too large to compute cross product");
            }
        }
        std::cout << "interp size: " << sz << std::endl;
        if (loc_prob_map.size() != x_map.size() * y_map.size()) {
            throw std::runtime_error("loc_prob_map size != x_map.size * y_map.size");
        }
        x.reserve(x_map.size());
        y.reserve(y_map.size());
        for (auto&& [x_, idx] : x_map) {
            x.emplace_back(x_);
            idx = x.size() - 1;
        }
        for (auto&& [y_, idx] : y_map) {
            y.emplace_back(y_);
            idx = y.size() - 1;
        }
        z.resize(x.size() * y.size());

        if (use_cubic && x.size() >= gsl_interp2d_bicubic->min_size && y.size() >= gsl_interp2d_bicubic->min_size) {
            spline = gsl_spline2d_alloc(gsl_interp2d_bicubic, x.size(), y.size());
        } else {
            if (use_cubic) std::cerr << "cubic interp not used cause sample points' size of each axis must >= 4" << std::endl;
            spline = gsl_spline2d_alloc(gsl_interp2d_bilinear, x.size(), y.size());
        }
        x_acc = gsl_interp_accel_alloc();
        y_acc = gsl_interp_accel_alloc();
    }

    void operator()(std::unordered_map<Point, std::map<RSRP_TYPE, Prob>>& points_to_interp) {
        this->points_to_interp = &points_to_interp;
        dfs(loc_prob_map.begin(), 1.0);
    }

    ~ProbInterp() {
        gsl_spline2d_free(spline);
        gsl_interp_accel_free(x_acc);
        gsl_interp_accel_free(y_acc);
    }
};

}  // namespace rxy