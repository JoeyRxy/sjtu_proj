#include "hmm.hpp"

#include <assert.h>

namespace rxy {

/**
 * @brief Viterbi algorithm for HMM in time sequence { 1, 2, ... T }
 * Basic idea: dynamic programming. define dp(t, l) as the max probability of being in state(location) l at time t.
 * then, we have the relation formula between (t - 1, l') and (t, l):
 * dp(t, l) = max_{l'} dp(t-1, l') * P(l' -> l) * P(l -> r)
 * where P(l -> r) is the transition probability from location l' to location l, and P(l -> r) is the emission probability of r at state(location) l.
 * @param markovs: a list of markov transition probability functions, corresponding to the time
 * sequence { 1, 2, ... T - 1 } (Attention: the LAST time T is not used). markovs[t - 1] means the
 * markov transition probability function transited from time t - 1 to time t.
 * @param init_prob: the initial probability of each location at the first time step t = 1.
 * @param emission_probs: the rsrp emission probability P(X|L) corresponding to every location at
 * each time step t.
 * */
std::vector<LocationPtr> const HMM::operator()(std::vector<MarkovPtr> const& markovs,
                                            std::unordered_map<LocationPtr, Prob> const& init_prob,
                                            std::vector<EmissionProb> const& emission_probs) const {
    // number of states (locations)
    int const N = loc_set->size();
    assert(init_prob.size() == N);
    // T is the number of time steps
    int T = emission_probs.size();
    assert(markovs.size() == T - 1);
    std::unordered_map<LocationPtr, Prob> prev, cur;
    std::vector<std::unordered_map<LocationPtr, LocationPtr>> psi(T - 1);
    // t = 0
    for (auto&& loc : *loc_set) {
        cur[loc] = init_prob.at(loc) * emission_probs[0][loc];
    }
    for (int t = 1; t < T; ++t) {
        auto const& markov = *markovs[t - 1];
        std::swap(prev, cur);
        cur.clear();
        for (auto&& loc : *loc_set) {
            Prob max_prob;
            LocationPtr max_loc = nullptr;
            for (auto&& prev_loc : *loc_set) {
                Prob prob = prev[prev_loc] * markov(prev_loc, loc);
                if (prob > max_prob) {
                    max_prob = prob;
                    max_loc = prev_loc;
                }
            }
            cur[loc] = max_prob * emission_probs[t][loc];
            psi[t - 1][loc] = max_loc;
        }
    }
    Prob max_prob;
    LocationPtr max_loc = nullptr;
    for (auto&& loc : *loc_set) {
        if (cur[loc] > max_prob) {
            max_prob = cur[loc];
            max_loc = loc;
        }
    }
    std::vector<LocationPtr> ret(T);
    ret[T - 1] = max_loc;
    for (int t = T - 2; t >= 0; --t) {
        ret[t] = psi[t][ret[t + 1]];
    }
    return ret;
}

}  // namespace rxy