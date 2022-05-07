#pragma once
#include <assert.h>

#include <unordered_map>
#include <vector>
#include <configure.hpp>

#include "hmm/location.hpp"

namespace rxy {

class LocationMap {
   private:
    // static constexpr int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
    // static constexpr int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

    int m, n;
    std::vector<std::vector<LocationPtr>> loc_map;
    std::unordered_map<int, std::pair<int, int>> loc_dict;
    std::unordered_set<LocationPtr> loc_set;

    Point left_down, right_up;
    Point::value_type x_step, y_step;
    Point::value_type x_ratio, y_ratio;

    Prob::value_type sigma;

    void __init() {
        assert(m > 1 && m <= MAP_SIZE && n > 1 && n <= MAP_SIZE);
        assert(right_up.x() > left_down.x() && right_up.y() > left_down.y());
        x_step = (right_up.x() - left_down.x()) / m;
        y_step = (right_up.y() - left_down.y()) / n;
        assert(1 - std::min(x_step, y_step) / std::max(x_step, y_step) < ECCENTRICITY);
        x_ratio = 1.0 / x_step;
        y_ratio = 1.0 / y_step;
        sigma = ND_SIGMA * std::max(x_step, y_step);
    }

   public:

    bool check(int i, int j) const {
        return i >= 0 && i < m && j >= 0 && j < n;
    }

    bool check(LocationPtr loc) const {
        return loc != nullptr && check(loc->point);
    }


    bool check(Point const & point) const {
        return point.x() > left_down.x() && point.x() < right_up.x() && point.y() > left_down.y() &&
               point.y() < right_up.y();
    }

    LocationMap(int m, int n, Point const& left_down, Point const& right_up)
        : m(m), n(n), loc_map(m, std::vector<LocationPtr>(n)), left_down(left_down), right_up(right_up) {
        __init();
    }

    LocationMap(int m, int n, Point&& left_down, Point&& right_up) 
        : m(m), n(n), loc_map(m, std::vector<LocationPtr>(n)), left_down(left_down), right_up(right_up) {
        __init();
    }

    LocationMap(const LocationMap&) = delete;
    LocationMap& operator=(const LocationMap&) = delete;

    LocationMap(LocationMap&&) = default;
    LocationMap& operator=(LocationMap&&) = default;

    constexpr auto get_sigma() const {
        return sigma;
    }

    bool add_loc(Point const & point) {
        int i = (point.x() - left_down.x()) * x_ratio;
        int j = (point.y() - left_down.y()) * y_ratio;
        assert(check(i, j));
        if (loc_map[i][j] == nullptr) {
            loc_map[i][j] = std::make_shared<Location>(loc_dict.size(), point.x(), point.y());
            loc_dict[loc_map[i][j]->id] = std::make_pair(i, j);
            loc_set.emplace(loc_map[i][j]);
            return true;
        } else {
            return false;
        }
    }

    bool add_loc(int i, int j) {
        assert(check(i, j));
        if (loc_map[i][j] == nullptr) {
            loc_map[i][j] = std::make_shared<Location>(loc_dict.size(), left_down.x() + i * x_step,
                                                       left_down.y() + j * y_step);
            loc_dict[loc_map[i][j]->id] = std::make_pair(i, j);
            loc_set.emplace(loc_map[i][j]);
            return true;
        } else {
            return false;
        }
    }

    void add_loc(LocationPtr loc) {
        assert(loc != nullptr);
        int x = static_cast<int>((loc->point.x() - left_down.x()) * x_ratio);
        int y = static_cast<int>((loc->point.y() - left_down.y()) * y_ratio);
        assert(check(x, y));
        loc_map[x][y] = loc;
        loc_dict.emplace(loc->id, std::make_pair(x, y));
        loc_set.emplace(loc);
    }

    auto& get_loc_dict() const { return loc_dict; }

    auto& get_loc_set() const { return loc_set; }

    auto& get_loc_idx(LocationPtr loc) const {
        assert(check(loc));
        return loc_dict.at(loc->id);
    }

    auto& get_loc(int i, int j) const {
        return loc_map.at(i).at(j);
    }

    auto get_loc(int id) const {
        auto [i, j] = loc_dict.at(id);
        return loc_map[i][j];
    }

    auto get_loc(Point const & point) const {
        int i = (point.x() - left_down.x()) * x_ratio;
        int j = (point.y() - left_down.y()) * y_ratio;
        return loc_map.at(i).at(j);
    }

    auto get_loc_idx(Point const& point) const {
        int x = static_cast<int>((point.x() - left_down.x()) * x_ratio);
        int y = static_cast<int>((point.y() - left_down.y()) * y_ratio);
        check(x, y);
        return std::make_pair(x, y);
    }

    auto get_map_row_size() const {
        return m;
    }

    auto get_map_col_size() const {
        return n;
    }

};

}  // namespace rxy