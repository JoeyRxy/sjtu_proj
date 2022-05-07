#pragma once
#include <map>
#include <numbers>
#include "hmm/location.hpp"

namespace rxy {

class BarrierLoc : public Location {
private:
    // [0, 2 * pi), angles range which has blocked by the barrier
    // key: start angle, value: end angle
    std::map<double, double> blocked_angles_;
    
public:
    BarrierLoc(int id, Point const & p) : Location(id, p) {}

    void add_blocked_angle(double start, double end) {
        assert(start < end);
        assert(start >= 0 && start < 2 * std::numbers::pi);
        assert(end >= 0 && end < 2 * std::numbers::pi);
        // blocked_angles_.emplace(start, end);
        auto start_it = blocked_angles_.upper_bound(start);
        if (start_it == blocked_angles_.end()) {
            --start_it;
            if (start_it->second >= start) {
                start_it->second = std::max(start_it->second, end);
            } else {
                blocked_angles_.emplace_hint(blocked_angles_.end(), start, end);
            }
        } else if (start_it == blocked_angles_.begin()) {
            auto end_it = blocked_angles_.upper_bound(end);
            if (end_it == blocked_angles_.begin()) {
                blocked_angles_.emplace_hint(blocked_angles_.begin(), start, end);
            } else {
                end = std::max(end, prev(end_it)->second);
                blocked_angles_.erase(start_it, end_it);
                blocked_angles_.emplace_hint(blocked_angles_.begin(), start, end);
            }
        } else {
            auto end_it = blocked_angles_.upper_bound(end);
            end = std::max(end, prev(start_it)->second);
            blocked_angles_.erase(start_it, end_it);
            --start_it;
            if (start_it->second >= start) {
                start_it->second = end;
            } else {
                blocked_angles_.emplace_hint(++start_it, start, end);
            }
        }
    }

    bool is_blocked(double angle) const {
        assert(angle >= 0 && angle < 2 * std::numbers::pi);
        auto it = blocked_angles_.upper_bound(angle);
        if (it == blocked_angles_.begin()) {
            return false;
        }
        return (--it)->second > angle;
    }
    
};

using BarrierLocPtr = std::shared_ptr<BarrierLoc>;

// try cast LocationPtr to BarrierLocPtr
inline BarrierLocPtr barrier_loc(LocationPtr loc) {
    return std::dynamic_pointer_cast<BarrierLoc>(loc);
}

}
