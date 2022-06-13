#pragma once
#include <cmath>
#include <numbers>

#include "location.hpp"

namespace rxy {

struct Sensation {
    static constexpr Point::value_type pi = std::numbers::pi_v<Point::value_type>;
    Point _delta;

    Sensation(Point::value_type direction, Point::value_type speed, double dt)
        : _delta{speed * cos(direction) * dt, speed * sin(direction) * dt} {}

    Sensation(Point speed, double dt)
        : _delta(speed * dt) {}

    virtual ~Sensation() = default;

    virtual Point const& delta() const { return _delta; }
};

}  // namespace rxy