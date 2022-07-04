#include "hmm.hpp"

namespace rxy {
#ifdef DEBUG

static void pM(std::unordered_set<LocationPtr> const & loc_set, Markov const & markov, LocationPtr const & loc) {
    for (auto & prv: loc_set) {
        auto prob = *markov(prv, loc);
        if (prob > 0) {
            std::cout << prv->point << " -> " << loc->point << ": " << prob << '\n';
        }
    }
    std::cout << "done" << std::endl;
}

static void pLP(std::unordered_map<LocationPtr, Prob> const & _m) {
    for (auto const & [loc, prob] : _m) {
        std::cout << loc->point << ": " << prob << std::endl;
    }
}

#endif

/**
 * @brief Viterbi algorithm for HMM in time sequence { 0, 1, 2, ... T - 1 }
 * Basic idea: dynamic programming. define dp(t, l) as the max probability of being in state(location) l at time t.
 * then, we have the relation formula between (t - 1, l') and (t, l):
 * dp(t, l) = max_{l'} dp(t - 1, l') * P(l' -> l) * P(l -> r)
 * where P(l -> r) is the transition probability from location l' to location l, and P(l -> r) is the emission probability of r at state(location) l.
 * @param markovs: a list of markov transition probability functions, corresponding to the time
 * sequence { 0, 1, 2, ... T - 2 } (Attention: the LAST time T - 1 is not used). markovs[t - 1] means the
 * markov transition probability function transited from time t - 1 to time t.
 * @param init_prob: the initial probability of each location at the first time step t = 1.
 * @param emission_probs: the rsrp emission probability P(X|L) corresponding to every location at
 * each time step t.
 * */
std::vector<LocationPtr> const HMM::viterbi(std::vector<MarkovPtr> const& markovs,
                                            std::unordered_map<LocationPtr, Prob> const& init_prob,
                                            std::vector<EmissionProb> const& emission_probs) const {
    // number of states (locations)
    auto N = loc_set->size();
    if (init_prob.size() != N) throw std::runtime_error("init_prob.size() != N");
    if (emission_probs[0].size() != N) throw std::runtime_error("emission_probs[0].size() != N");
    // T is the number of time steps
    auto T = emission_probs.size();
    if (T == 0 || !(markovs.size() == T - 1)) throw std::runtime_error("T == 0 OR markovs.size() != T - 1");
    std::unordered_map<LocationPtr, Prob> prv, cur;
    prv.reserve(N);
    cur.reserve(N);
    std::vector<std::unordered_map<LocationPtr, LocationPtr>> psi(T - 1);
    // t = 0
    bool all_zero = true;
    for (auto&& loc : *loc_set) {
        auto p = init_prob.at(loc) * emission_probs[0].at(loc);
        if (p != Prob::ZERO) {
            all_zero = false;
        }
        cur[loc] = p;
    }
    if (all_zero) {
        throw std::runtime_error("all zero for start");
    }
    std::vector<LocationPtr> ret(T);
    int start = 0;
    for (size_t t = 1; t < T; ++t) {
        auto const& markov = *markovs[t - 1];
        std::swap(prv, cur);
        auto& et = emission_probs[t];
        auto& pt = psi[t - 1];
        // cur.clear();
        for (auto&& loc : *loc_set) {
            Prob max_prob;
            LocationPtr max_loc = nullptr;
            for (auto&& prev_loc : *loc_set) {
                // markov[prv_loc][loc]: can optimize for locality
                Prob prob = prv[prev_loc] * markov(prev_loc, loc);
                if (prob > max_prob) {
                    max_prob = prob;
                    max_loc = prev_loc;
                }
            }
            if (max_loc) {
                max_prob *= et.at(loc);
                if (max_prob != Prob::ZERO) {
                    all_zero = false;
                }
                cur[loc] = max_prob;
                pt[loc] = max_loc;
            } else {
                max_prob = Prob::ZERO;
                max_loc = nullptr;
                for (auto && loc : *loc_set) {
                    if (prv[loc] > max_prob) {
                        max_prob = prv[loc];
                        max_loc = loc;
                    }
                }
                ret[t - 1] = max_loc;
                for (int _t = t - 2; _t >= start; --_t) {
                    ret[_t] = psi[_t][ret[_t + 1]];
                }
                start = t;
                for (auto && loc : *loc_set) {
                    auto p = init_prob.at(loc) * et.at(loc);
                    if (p != Prob::ZERO) {
                        all_zero = false;
                    }
                    cur[loc] = p;
                }
                if (all_zero) {
                    throw std::runtime_error("all zero for t = " + std::to_string(t));
                }
                break;
            }
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
    ret[T - 1] = max_loc;
    for (int t = T - 2; t >= start; --t) {
        ret[t] = psi[t][ret[t + 1]];
    }
    return ret;
}

}  // namespace rxy