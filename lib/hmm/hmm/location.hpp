#pragma once
#include <cmath>
#include <configure.hpp>
#include <functional>
#include <memory>
#include <iostream>

namespace rxy {

class Location;
using LocationPtr = std::shared_ptr<Location>;

class Point {
public:
    using value_type = POINT_TYPE;

protected:
    friend constexpr bool operator==(Point const& lhs, Point const& rhs);
    friend constexpr Point operator+(Point const& lhs, Point const& rhs);
    friend constexpr Point operator-(Point const& lhs, Point const& rhs);
    friend std::ostream& operator<<(std::ostream& os, Point const& p);

    value_type x_, y_;
    value_type r_, phi_;

    bool computed_ = false;

public:
    constexpr Point(value_type x, value_type y) : x_(x), y_(y) {
    }

    virtual ~Point() = default;

    constexpr Point& operator+=(Point const& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    constexpr Point& operator-=(Point const& rhs) {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    constexpr value_type x() const {
        return x_;
    }

    constexpr value_type y() const {
        return y_;
    }

    value_type r() {
        if (!computed_) {
            r_ = std::sqrt(x_ * x_ + y_ * y_);
            phi_ = std::atan2(y_, x_);
            computed_ = true;
        }
        return r_;
    }

    value_type phi() {
        if (!computed_) {
            r_ = std::sqrt(x_ * x_ + y_ * y_);
            phi_ = std::atan2(y_, x_);
            computed_ = true;
        }
        return phi_;
    }
    
};

using PointPtr = std::shared_ptr<Point>;

inline constexpr bool operator==(Point const& lhs, Point const& rhs) {
    return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
}

inline constexpr Point operator+(Point const& lhs, Point const& rhs) {
    return {lhs.x_ + rhs.x_, lhs.y_ + rhs.y_};
}

inline constexpr Point operator-(Point const& lhs, Point const& rhs) {
    return {lhs.x_ - rhs.x_, lhs.y_ - rhs.y_};
}

inline constexpr Point::value_type minkowski(Point const& lhs, Point const& rhs, int p = 2) {
    if (p == 1) return std::abs(lhs.x() - rhs.x()) + std::abs(lhs.y() - rhs.y());
    if (p == 2) return sqrt(pow(lhs.x() - rhs.x(), 2) + pow(lhs.y() - rhs.y(), 2));
    return std::sqrt(std::pow(std::abs(lhs.x() - rhs.x()), p) + std::pow(std::abs(lhs.y() - rhs.y()), p));
}

inline std::ostream& operator<<(std::ostream& os, Point const& p) {
    os << "(" << p.x_ << ", " << p.y_ << ")";
    return os;
}

struct Location {
    int id;
    Point point;

    constexpr Location(int id, Point::value_type x, Point::value_type y) : id(id), point(x, y) {}
    constexpr Location(int id, Point const& point) : id(id), point(point) {}
    constexpr Location(int id, Point&& point) : id(id), point(std::move(point)) {}
    virtual ~Location() = default;
};

inline constexpr Point::value_type distance(Location const& lhs, Location const& rhs, int p = 2) {
    return minkowski(lhs.point, rhs.point, p);
}

inline Point::value_type distance(LocationPtr lhs, LocationPtr rhs, int p = 2) {
    if (lhs == nullptr || rhs == nullptr) return -1;
    return minkowski(lhs->point, rhs->point, p);
}

inline constexpr bool operator==(Location const& lhs, Location const& rhs) {
    return lhs.id == rhs.id;
}

inline bool operator==(LocationPtr const& lhs, LocationPtr const& rhs) {
    if (lhs == nullptr && rhs == nullptr) return true;
    if (lhs == nullptr || rhs == nullptr) return false;
    return lhs->id == rhs->id;
}

}  // namespace rxy

namespace std {

template <>
struct hash<rxy::Point> {
    size_t operator()(rxy::Point const& p) const {
        return hash<rxy::Point::value_type>()(p.x()) ^ hash<rxy::Point::value_type>()(p.y());
    }
};

template <>
struct hash<rxy::PointPtr> {
    size_t operator()(rxy::PointPtr const& p) const {
        if (p == nullptr) return std::numeric_limits<size_t>::max();
        return hash<rxy::Point::value_type>()(p->x()) ^ hash<rxy::Point::value_type>()(p->y());
    }
};

template <>
struct hash<rxy::Location> {
    size_t operator()(rxy::Location const& loc) const { return hash<int>()(loc.id); }
};

template <>
struct hash<rxy::LocationPtr> {
    size_t operator()(rxy::LocationPtr const& loc) const {
        if (loc == nullptr) return numeric_limits<size_t>::max();
        return hash<int>()(loc->id);
    }
};

}  // namespace std