#pragma once
#include <unordered_map>

#include "location.hpp"
#include "probability.hpp"

namespace rxy {

// A wrapper which wraps a markov emission probability function.
using EmissionProb = std::unordered_map<LocationPtr, Prob>;

}  // namespace rxy