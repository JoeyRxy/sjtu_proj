#pragma once
#include <unordered_map>
#include <vector>
#include <list>
#include <configure.hpp>

#include "hmm/location.hpp"

namespace rxy {

class LocationMap {
   private:
    // static constexpr int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
    // static constexpr int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};

    int m, n;
    std::vector<std::vector<LocationPtr>> loc_map;
    // loc_id -> (i, j)
    std::unordered_map<int, std::pair<int, int>> loc_dict;
    std::list<LocationPtr> loc_set;

    Point left_down, right_up;
    Point::value_type x_step, y_step;
    Point::value_type x_ratio, y_ratio;

    Prob::value_type sigma;

    void __init() {
        if (!(m > 1 && m <= MAP_SIZE && n > 1 && n <= MAP_SIZE)) throw std::runtime_error("invalid argument");
        if (!(right_up.x() > left_down.x() && right_up.y() > left_down.y())) throw std::runtime_error("invalid argument");
        x_step = (right_up.x() - left_down.x()) / m;
        y_step = (right_up.y() - left_down.y()) / n;
        if (!(1 - std::min(x_step, y_step) / std::max(x_step, y_step) < ECCENTRICITY)) throw std::runtime_error("invalid argument");
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
        : m(m), n(n), loc_map(m, std::vector<LocationPtr>(n)), left_down(std::move(left_down)), right_up(std::move(right_up)) {
        __init();
    }

    LocationMap(const LocationMap&) = delete;
    LocationMap& operator=(const LocationMap&) = delete;

    LocationMap(LocationMap&&) = default;
    LocationMap& operator=(LocationMap&&) = default;

    constexpr auto get_sigma() const {
        return sigma;
    }

    template<typename T>
    requires std::derived_from<T, Location>
    bool add_loc(Point const & point) {
        int i = (point.x() - left_down.x()) * x_ratio;
        int j = (point.y() - left_down.y()) * y_ratio;
        if (!(check(i, j))) throw std::runtime_error("invalid argument");
        if (loc_map[i][j] == nullptr) {
            loc_map[i][j] = std::make_shared<T>(loc_dict.size(), point);
            loc_dict[loc_map[i][j]->id] = std::make_pair(i, j);
            loc_set.emplace_back(loc_map[i][j]);
            return true;
        } else {
            return false;
        }
    }

    template <typename T>
    requires std::derived_from<T, Location>
    bool add_loc(int i, int j) {
        if (!(check(i, j))) throw std::runtime_error("invalid argument");
        if (loc_map[i][j] == nullptr) {
            loc_map[i][j] = std::make_shared<T>(loc_dict.size(), Point{left_down.x() + i * x_step + x_step / 2,
                                                       left_down.y() + j * y_step + y_step / 2});
            loc_dict[loc_map[i][j]->id] = std::make_pair(i, j);
            loc_set.emplace_back(loc_map[i][j]);
            return true;
        } else {
            return false;
        }
    }

    void add_loc(LocationPtr loc) {
        if (!(loc != nullptr)) throw std::runtime_error("invalid argument");
        int x = static_cast<int>((loc->point.x() - left_down.x()) * x_ratio);
        int y = static_cast<int>((loc->point.y() - left_down.y()) * y_ratio);
        if (!(check(x, y))) throw std::runtime_error("invalid argument");
        loc_map[x][y] = loc;
        loc_dict.emplace(loc->id, std::make_pair(x, y));
        loc_set.emplace_back(loc);
    }

    auto& get_loc_dict() const { return loc_dict; }

    auto& get_loc_set() const { return loc_set; }

    auto& get_loc_idx(LocationPtr loc) const {
        if (!(check(loc))) throw std::runtime_error("invalid argument");
        return loc_dict.at(loc->id);
    }

    auto& get_loc(int i, int j) const {
        return loc_map.at(i).at(j);
    }

    auto& get_loc(int id) const {
        auto [i, j] = loc_dict.at(id);
        return loc_map.at(i).at(j);
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

    auto x_range() const {
        return std::make_pair(left_down.x(), right_up.x());
    }

    auto y_range() const {
        return std::make_pair(left_down.y(), right_up.y());
    }

    auto step() const {
        return std::make_pair(x_step, y_step);
    }

};

}  // namespace rxy