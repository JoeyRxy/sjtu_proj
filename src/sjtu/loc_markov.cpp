#include "loc_markov.hpp"
#include <configure.hpp>

#ifdef DEBUG
#include <iostream>
#endif

namespace rxy {

void LocMarkov::__init() {
    auto& delta = sense.delta();
    int N = loc_map.get_ext_dict().size();
    _tran_prob.reserve(N);

#ifdef DEBUG
    std::cout << "start Markov computing..." << std::endl;
#endif

    for (auto && loc : loc_map.get_ext_list()) {
        Point new_point = loc->point + delta;
        auto & Mi = _tran_prob[loc];
        Mi.reserve(N);
        LocationPtr new_loc;
        if (!loc_map.check(new_point) || !(new_loc = loc_map.get_ext_loc(new_point))) {
            for (auto && dest : loc_map.get_ext_list()) {
                Mi.emplace(dest, Prob::ZERO);
            }
            continue;
        }
        
        for (auto && dest : loc_map.get_ext_list()) {
            Mi.emplace(dest, log_nd_pdf(loc_map.distance(loc, new_loc)));
        }
    }

#ifdef DEBUG
    std::cout << "Markov trans prob DONE." << std::endl;
#endif

}

}