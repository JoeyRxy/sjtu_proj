#pragma once
#include "location.hpp"
#include "sensation.hpp"
#include <unordered_set>
#include "probability.hpp"

namespace rxy {

class Markov {
protected:
    Sensation const sense;

public:
    Markov(Sensation const& sense) : sense(sense) {}
    Markov(Sensation && sense) : sense(std::move(sense)) {}

    virtual Prob operator()(LocationPtr prev, LocationPtr cur) const = 0;
};

using MarkovPtr = std::shared_ptr<Markov>;

}
