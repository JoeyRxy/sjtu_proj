#pragma once
#include "location.hpp"
#include "sensation.hpp"
#include <unordered_set>
#include "probability.hpp"

namespace rxy {

class Markov {
protected:
    Sensation const* sense;
private:
    const bool is_move_in;
public:
    Markov(Sensation const& sense) : sense(&sense), is_move_in(false) {}
    Markov(Sensation && sense) : sense(new Sensation(std::move(sense))), is_move_in(true) {}
    virtual ~Markov() {
        if (is_move_in && sense != nullptr) {
            delete sense;
        }
    }

    virtual Prob operator()(LocationPtr prev, LocationPtr cur) const = 0;
};

using MarkovPtr = std::shared_ptr<Markov>;

}
