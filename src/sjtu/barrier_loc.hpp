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
    BarrierLoc(int id, Point && p) : Location(id, std::move(p)) {}

    void add_blocked_angle(double start, double end) {
        if (!(start < end)) throw std::runtime_error("invalid argument");
        if (!(start >= 0 && start < 2 * std::numbers::pi)) throw std::runtime_error("invalid argument");
        if (!(end >= 0 && end < 2 * std::numbers::pi)) throw std::runtime_error("invalid argument");
        if (!blocked_angles_.empty()) {
            auto lit = blocked_angles_.lower_bound(start);
            auto it = lit;
            bool is_first = true;
            if (lit != blocked_angles_.begin()) {
                is_first = false;
                --lit;
            }
            double second = -std::numeric_limits<double>::infinity();
            while (it != blocked_angles_.end() && it->first <= end) {
                second = it->second;
                it = blocked_angles_.erase(it);
            }
            end = std::max(end, second);
            if (!is_first) {
                if (lit->second >= start) {
                    lit->second = std::max(end, lit->second);
                    return;
                }
            }
        }
        blocked_angles_[start] = end;
    }

    void remove_blocked_angle(double left, double right) {
        if (!(left < right)) throw std::runtime_error("invalid argument");
        if (!(left >= 0 && left < 2 * std::numbers::pi)) throw std::runtime_error("invalid argument");
        if (!(right >= 0 && right < 2 * std::numbers::pi)) throw std::runtime_error("invalid argument");
        if (blocked_angles_.empty()) return;
        auto lit = blocked_angles_.lower_bound(left);
        auto it = lit;
        double second = -std::numeric_limits<double>::infinity();
        if (lit != blocked_angles_.begin()) {
            --lit;
            if (lit->second > left) {
                second = lit->second;
                lit->second = left;
            }
        }
        while (it != blocked_angles_.end() && it->first < right) {
            second = it->second;
            it = blocked_angles_.erase(it);
        }
        if (second > right) {
            blocked_angles_[right] = second;
        }
    }

    bool is_blocked(double angle) const {
        if (!(angle >= 0 && angle <= 2 * std::numbers::pi)) throw std::runtime_error("invalid argument");
        if (blocked_angles_.empty()) {
            return false;
        }
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
