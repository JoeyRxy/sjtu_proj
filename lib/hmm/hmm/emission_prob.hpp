#pragma once
#include <unordered_map>

#include "location.hpp"
#include "probability.hpp"

namespace rxy {

// A wrapper which wraps a markov transition probability function.
class EmissionProb {
   protected:
    using iterator = std::unordered_map<LocationPtr, Prob>::iterator;
    using const_iterator = std::unordered_map<LocationPtr, Prob>::const_iterator;

    std::unordered_map<LocationPtr, Prob> emission_probs;

   public:
    EmissionProb() = default;
    EmissionProb(std::unordered_map<LocationPtr, Prob> const &emission_probs)
        : emission_probs(emission_probs) {}
    EmissionProb(std::unordered_map<LocationPtr, Prob> &&emission_probs)
        : emission_probs(std::move(emission_probs)) {}

    iterator find(LocationPtr loc) { return emission_probs.find(loc); }
    
    iterator begin() { return emission_probs.begin(); }
    iterator end() { return emission_probs.end(); }
    const_iterator cbegin() const { return emission_probs.cbegin(); }
    const_iterator cend() const { return emission_probs.cend(); }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args &&...args) {
        return emission_probs.emplace(std::forward<Args>(args)...);
    }

    Prob &operator[](LocationPtr const &loc) { return emission_probs[loc]; }

    Prob const &operator[](LocationPtr const &loc) const { return emission_probs.at(loc); }

    virtual ~EmissionProb() = default;
};

}  // namespace rxy