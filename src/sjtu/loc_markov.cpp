#include "loc_markov.hpp"
#include "hmm/location.hpp"
#include "hmm/probability.hpp"
#include <configure.hpp>
#include <execution>
#include <limits>
#include <stdexcept>

#ifdef DEBUG
#include <iostream>
#endif

namespace rxy {

void LocMarkov::__init() {
    auto &delta = sense.delta();
    auto N = loc_map.get_ext_dict().size();
    _tran_prob.reserve(N);

    auto const &ls = loc_map.get_ext_list();
    for (auto &&loc : ls) {
        auto &t = _tran_prob[loc];
        for (auto &&l : ls)
            t[l] = Prob::ZERO;
    }
    std::for_each(
        std::execution::par_unseq, ls.begin(), ls.end(),
        [this, &delta, &ls](LocationPtr loc) {
            Point new_point = loc->point + delta;
            LocationPtr new_loc;
            if (!loc_map.check(new_point) ||
                !(new_loc = loc_map.get_ext_loc(new_point))) {
                return;
            }

            for (auto &&dest : ls) {
                try {
                    auto dist = loc_map.distance(new_loc, dest);
                    if (dist != std::numeric_limits<double>::infinity() &&
                        dist < 1.5 * minkowski(loc->point, new_loc->point)) {
                        _tran_prob[dest][loc] = log_nd_pdf(dist);
                    }
                } catch (std::out_of_range &) {
                    continue;
                }
            }
        });

#ifdef DEBUG
    std::cout << "Markov trans prob DONE." << std::endl;
#endif
}

} // namespace rxy