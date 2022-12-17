#pragma once
#include <configure.hpp>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef DEBUG
#include <iostream>
#endif

#include <config.h>
#include "ext_loc.hpp"
#include "hmm/location.hpp"
#include "hmm/probability.hpp"

namespace rxy {

class LocationMap {
   private:
    int m, n;
    std::vector<std::vector<LocationPtr>> loc_map;
    std::vector<std::vector<LocationPtr>> ext_map;
    // loc_id -> (i, j)
    std::unordered_map<int, std::pair<int, int>> loc_dict;
    std::list<LocationPtr> loc_set;

    Point left_down, right_up;
    Point::value_type x_step, y_step;
    Point::value_type x_ratio, y_ratio;

    Point::value_type x_step_ext, y_step_ext;
    Point::value_type x_ratio_ext, y_ratio_ext;
    int m_ext, n_ext;
    std::unordered_map<LocationPtr, std::pair<int, int>> ext_dict;
    std::unordered_set<LocationPtr> ext_set;

    Prob::value_type sigma;

    mutable bool computed = false;
    mutable std::unordered_map<LocationPtr, std::unordered_map<LocationPtr, Point::value_type>>
        dist_map;

    void __init() {
        if (!(m > 1 && m <= GetConfig().map_size && n > 1 && n <= GetConfig().map_size))
            throw std::runtime_error("invalid argument");
        if (!(right_up.x() > left_down.x() && right_up.y() > left_down.y()))
            throw std::runtime_error("invalid argument");
        x_step = (right_up.x() - left_down.x()) / m;
        y_step = (right_up.y() - left_down.y()) / n;
        if (!(1 - std::min(x_step, y_step) / std::max(x_step, y_step) < GetConfig().ecc))
            throw std::runtime_error("invalid argument");
        x_ratio = 1.0 / x_step;
        y_ratio = 1.0 / y_step;
        sigma = ND_SIGMA * std::max(x_step, y_step);

        loc_dict.reserve(m * n);

        x_step_ext = x_step / GetConfig().ext_rate, y_step_ext = y_step / GetConfig().ext_rate;
        x_ratio_ext = 1.0 / x_step_ext, y_ratio_ext = 1.0 / y_step_ext;

        m_ext = m * GetConfig().ext_rate, n_ext = n * GetConfig().ext_rate;
        ext_map.resize(m_ext, std::vector<LocationPtr>(n_ext));
        ext_dict.reserve(m_ext * n_ext);

        dist_map.reserve(m_ext * n_ext);
    }

   public:
    bool check(int i, int j) const { return i >= 0 && i < m && j >= 0 && j < n; }

    bool check(LocationPtr loc) const { return loc != nullptr && check(loc->point); }

    bool check(Point const& point) const {
        return point.x() > left_down.x() && point.x() < right_up.x() && point.y() > left_down.y() &&
               point.y() < right_up.y();
    }

    LocationMap(int m, int n, Point const& left_down, Point const& right_up)
        : m(m),
          n(n),
          loc_map(m, std::vector<LocationPtr>(n)),
          left_down(left_down),
          right_up(right_up) {
        __init();
    }

    LocationMap(int m, int n, Point&& left_down, Point&& right_up)
        : m(m),
          n(n),
          loc_map(m, std::vector<LocationPtr>(n)),
          left_down(std::move(left_down)),
          right_up(std::move(right_up)) {
        __init();
    }

    LocationMap(const LocationMap&) = delete;
    LocationMap& operator=(const LocationMap&) = delete;

    LocationMap(LocationMap&&) = default;
    LocationMap& operator=(LocationMap&&) = default;

    constexpr auto get_sigma() const { return sigma; }

    bool add_loc(int i, int j, int id = -1) {
        if (!(check(i, j))) throw std::runtime_error("invalid argument");
        if (loc_map[i][j] == nullptr) {
            if (id != -1 && loc_dict.find(id) != loc_dict.end())
                throw std::runtime_error("duplicate id");
            if (id == -1) id = static_cast<int>(loc_set.size());
            loc_map[i][j] =
                std::make_shared<Location>(id, Point{left_down.x() + i * x_step + x_step / 2,
                                                     left_down.y() + j * y_step + y_step / 2});
            loc_dict[id] = std::make_pair(i, j);
            loc_set.emplace_back(loc_map[i][j]);
            int line_begin = GetConfig().ext_rate * i, line_end = GetConfig().ext_rate + line_begin;
            int col_begin = GetConfig().ext_rate * j, col_end = GetConfig().ext_rate + col_begin;
            for (int x = line_begin; x < line_end; ++x) {
                for (int y = col_begin; y < col_end; ++y) {
                    auto ptr = std::make_shared<ExtLocation>(
                        id, ext_set.size(),
                        Point{left_down.x() + x * x_step_ext + x_step_ext / 2,
                              left_down.y() + y * y_step_ext + y_step_ext / 2});
                    ext_set.emplace(ptr);
                    ext_map[x][y] = ptr;
                    ext_dict[ptr] = std::make_pair(x, y);
                }
            }
            return true;
        } else {
            return false;
        }
    }

    void tranverse(int i, int j, std::function<void(LocationPtr)> const& func) {
        if (!(check(i, j))) throw std::runtime_error("invalid argument");
        int line_begin = GetConfig().ext_rate * i, line_end = GetConfig().ext_rate + line_begin;
        int col_begin = GetConfig().ext_rate * j, col_end = GetConfig().ext_rate + col_begin;
        for (int k = line_begin; k < line_end; ++k) {
            for (int l = col_begin; l < col_end; ++l) {
                if (ext_map[k][l]) func(ext_map[k][l]);
            }
        }
    }

    std::pair<int, int> get_ext_index(Point const& p) const {
        return std::make_pair(
            std::min(static_cast<int>((p.x() - left_down.x() * x_ratio_ext)), m_ext - 1),
            std::min(static_cast<int>((p.y() - left_down.y() * y_ratio_ext)), n_ext - 1));
    }

    std::pair<int, int> get_ext_index(Point::value_type x, Point::value_type y) const {
        return std::make_pair(
            std::min(static_cast<int>((x - left_down.x() * x_ratio_ext)), m_ext - 1),
            std::min(static_cast<int>((y - left_down.y() * y_ratio_ext)), n_ext - 1));
    }

    void remove_ext_point(Point point) {
        check(point);
        auto [i, j] = get_ext_index(point);
        if (ext_map[i][j] != nullptr) {
            ext_dict.erase(ext_map[i][j]);
            ext_set.erase(ext_map[i][j]);
            ext_map[i][j] = nullptr;
        }
    }

    void remove_ext_line(Point start, Point end) {
        Point dist = end - start;
        auto R = dist.r();
        auto phi = dist.phi();
        Point::value_type dr = x_step_ext * std::cos(phi) + y_step_ext * std::sin(phi);
        Point::value_type x = start.x(), y = start.y();
        Point::value_type dx = dist.x() / R, dy = dist.y() / R;
        R -= EPSILON;
        for (Point::value_type r = 0; r < R; r += dr, x += dx, y += dy) {
            auto [i, j] = get_ext_index(x, y);
            if (ext_map[i][j] != nullptr) {
                ext_dict.erase(ext_map[i][j]);
                ext_set.erase(ext_map[i][j]);
                ext_map[i][j] = nullptr;
            }
        }
    }

    void remove_ext_rect(Point ld, Point ru) {
        auto [x, y] = get_ext_index(ld);
        auto [x_end, y_end] = get_ext_index(ru);
        if (x_end < m_ext) x_end++;
        if (y_end < n_ext) y_end++;
        for (; x < x_end; ++x) {
            for (int _y = y; _y < y_end; ++_y) {
                if (ext_map[x][_y] != nullptr) {
                    ext_dict.erase(ext_map[x][_y]);
                    ext_set.erase(ext_map[x][_y]);
                    ext_map[x][_y] = nullptr;
                }
            }
        }
    }

    auto& get_loc_dict() const { return loc_dict; }

    auto& get_loc_list() const { return loc_set; }

    auto& get_ext_dict() const { return ext_dict; }

    auto& get_ext_list() const { return ext_set; }

    auto& get_loc_idx(LocationPtr loc) const {
        if (!(check(loc))) throw std::runtime_error("invalid argument");
        return loc_dict.at(loc->id);
    }

    auto& get_loc(int i, int j) const { return loc_map.at(i).at(j); }

    auto& get_loc(int id) const {
        auto [i, j] = loc_dict.at(id);
        return loc_map.at(i).at(j);
    }

    auto get_loc(Point const& point) const {
        int i = static_cast<int>((point.x() - left_down.x()) * x_ratio);
        int j = static_cast<int>((point.y() - left_down.y()) * y_ratio);
        return loc_map.at(i).at(j);
    }

    auto get_ext_loc(Point const& point) const {
        int i = static_cast<int>((point.x() - left_down.x()) * x_ratio_ext);
        int j = static_cast<int>((point.y() - left_down.y()) * y_ratio_ext);
        return ext_map.at(i).at(j);
    }

    auto get_loc_idx(Point const& point) const {
        int x = static_cast<int>((point.x() - left_down.x()) * x_ratio);
        int y = static_cast<int>((point.y() - left_down.y()) * y_ratio);
        check(x, y);
        return std::make_pair(x, y);
    }

    auto get_map_row_size() const { return m; }

    auto get_map_col_size() const { return n; }

    auto x_range() const { return std::make_pair(left_down.x(), right_up.x()); }

    auto y_range() const { return std::make_pair(left_down.y(), right_up.y()); }

    auto step() const { return std::make_pair(x_step, y_step); }

    void compute_distance() const;

    Point::value_type distance(LocationPtr loc1, LocationPtr loc2) const {
        compute_distance();
        return dist_map.at(loc1).at(loc2);
    }

    Point::value_type distance(Point const& p1, Point const& p2) const {
        check(p1);
        check(p2);
        auto [i, j] = get_ext_index(p1);
        auto [x, y] = get_ext_index(p2);
        compute_distance();
        return dist_map.at(ext_map[i][j]).at(ext_map[x][y]);
    }
};

}  // namespace rxy