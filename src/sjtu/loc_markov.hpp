#pragma once
#include "hmm/markov.hpp"
#include "location_map.hpp"

namespace rxy {

class LocMarkov : public Markov {
   private:
    LocationMap const& loc_map; // need to ensure that loc_map is not out of scope or released before THIS instance
    std::unordered_map<LocationPtr, std::unordered_map<LocationPtr, Prob>> _tran_prob;

    void __init();

   public:
    LocMarkov(LocationMap const& loc_map, Sensation const& sense) : Markov(sense), loc_map(loc_map) {
        __init();
    }
    LocMarkov(LocationMap const& loc_map, Sensation &&sense) : Markov(std::move(sense)), loc_map(loc_map) {
        __init();
    }
    LocMarkov(LocMarkov const& loc_markov) = default;
    LocMarkov(LocMarkov&& loc_markov) = default;
    LocMarkov& operator=(LocMarkov const& loc_markov) = default;
    LocMarkov& operator=(LocMarkov&& loc_markov) = default;
    virtual ~LocMarkov() = default;
    virtual Prob operator()(LocationPtr prev, LocationPtr cur) const override {
        if (prev == nullptr || cur == nullptr) {
            return 0;
        }
        return _tran_prob.at(prev).at(cur);
    }
};

using LocMarkovPtr = std::shared_ptr<LocMarkov>;

}  // namespace rxy
