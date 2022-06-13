#include "loc_markov.hpp"
#include <configure.hpp>
#include "barrier_loc.hpp"

namespace rxy {

void LocMarkov::__init() {
    Point delta = sense.delta();
    static Prob zero;
    for (auto && loc : loc_map.get_loc_set()) {
        Point new_point = loc->point + delta;
        auto & Mi = _tran_prob[loc];
        if (!loc_map.check(new_point) || !loc_map.get_loc(new_point)) {
            for (auto && dest : loc_map.get_loc_set()) {
                Mi.emplace(dest, zero);
            }
            continue;
        }
        auto bloc = barrier_loc(loc);
        if (bloc) {
            for (auto && dest : loc_map.get_loc_set()) {
                Point d = dest->point - loc->point;
                if (d.r() > 0 && bloc->is_blocked(d.phi())) {
                    Mi.emplace(dest, zero);
                } else {
                    Mi.emplace(dest, __tran_prob(new_point, dest->point, MINKOWSKI_P));
                }
            }
        } else {
            for (auto && dest : loc_map.get_loc_set()) {
                Mi.emplace(dest, __tran_prob(new_point, dest->point, MINKOWSKI_P));
            }
        }
    }
}

}