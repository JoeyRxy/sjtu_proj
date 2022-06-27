#pragma once
#include <cmath>
#include <numbers>

#include "location.hpp"

namespace rxy {

struct Sensation {
    static constexpr Point::value_type pi = std::numbers::pi_v<Point::value_type>;
    Point _delta;

    Sensation() = default;

    Sensation(Point::value_type direction, Point::value_type speed, double dt)
        : _delta{speed * cos(direction) * dt, speed * sin(direction) * dt} {}

    Sensation(Point speed, double dt)
        : _delta(speed * dt) {}

    virtual ~Sensation() = default;

    virtual Point const& delta() const { return _delta; }
};

}  // namespace rxy

namespace std {
    // overload hash
    template <>
    struct hash<rxy::Sensation> {
        size_t operator()(rxy::Sensation const& s) const {
            return hash<rxy::Point>()(s._delta);
        }
    };

    template <>
    struct equal_to<rxy::Sensation> {
        bool operator()(rxy::Sensation const& lhs, rxy::Sensation const& rhs) const {
            return lhs._delta == rhs._delta;
        }
    };
    
}
