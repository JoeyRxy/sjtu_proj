#pragma once
#include "hmm/markov.hpp"
#include "location_map.hpp"

namespace rxy {

class LocMarkov : public Markov {
   private:
    LocationMap const& loc_map;
    std::unordered_map<LocationPtr, std::unordered_map<LocationPtr, Prob>> _tran_prob;

    // ====== helpers ====== //
    constexpr Prob __tran_prob(Point const& from, Point const& to, int p = 2) const {
       return log_nd_pdf(minkowski(from, to, p), ND_MU, loc_map.get_sigma());
    }

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
