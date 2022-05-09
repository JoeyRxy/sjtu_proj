#pragma once
#include "emission_prob.hpp"
#include "location.hpp"
#include "markov.hpp"
#include "probability.hpp"
#include "sensation.hpp"
#include <list>

namespace rxy {

class HMM {
   private:
    std::list<LocationPtr> const *loc_set;
    const bool is_move_in;

   public:
    HMM() = default;
    HMM(std::list<LocationPtr> const &loc_set) : loc_set(&loc_set), is_move_in(false) {}
    HMM(std::list<LocationPtr> &&loc_set)
        : loc_set(new std::list<LocationPtr>(std::move(loc_set))), is_move_in(true) {}

    ~HMM() {
        if (is_move_in && loc_set != nullptr) {
            delete loc_set;
            loc_set = nullptr;
        }
    }

    std::vector<LocationPtr> const operator()(std::vector<MarkovPtr> const &markovs,
                                           std::unordered_map<LocationPtr, Prob> const &init,
                                           std::vector<EmissionProb> const &emission_probs) const;
};

}  // namespace rxy