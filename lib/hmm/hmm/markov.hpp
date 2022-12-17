#pragma once
#include "location.hpp"
#include "sensation.hpp"
#include <unordered_set>
#include "probability.hpp"

namespace rxy {

class Markov {
protected:
    Sensation const sense;
    std::unordered_map<LocationPtr, std::unordered_map<LocationPtr, Prob>> _tran_prob;

public:
    Markov(Sensation const& sense) : sense(sense) {}
    Markov(Sensation && sense) : sense(std::move(sense)) {}

    auto & get_tran_prob() const { return _tran_prob; }
};

using MarkovPtr = std::shared_ptr<Markov>;

}
