#include "loc_markov.hpp"
#include <configure.hpp>
#include "barrier_loc.hpp"

namespace rxy {

void LocMarkov::__init() {
    const Point& delta = sense->delta();
    Prob zero;
    for (auto && loc : loc_map.get_loc_set()) {
        Point new_point = loc->point + delta;
        if (!loc_map.check(new_point) || !loc_map.get_loc(new_point)) {
            for (auto && dest : loc_map.get_loc_set()) {
                _tran_prob[loc].emplace(dest, zero);
            }
            continue;
        }
        auto bloc = barrier_loc(loc);
        for (auto && dest : loc_map.get_loc_set()) {
            auto angle = (dest->point - loc->point).phi();
            if (bloc->is_blocked(angle)) {
                _tran_prob[loc].emplace(dest, zero);
            } else {
                _tran_prob[loc].emplace(dest, __tran_prob(new_point, dest->point, MINKOWSKI_P));
            }
        }
    }
}

}