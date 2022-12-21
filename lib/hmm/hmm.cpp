#include "hmm.hpp"
#include <algorithm>
#include <exception>
#include <execution>
#ifdef DEBUG
#include <iostream>
#endif

namespace rxy {
#ifdef DEBUG
static void pM(std::unordered_set<LocationPtr> const & loc_set, Markov const & markov, LocationPtr const & loc) {
    auto& tran_prob = markov.get_tran_prob().at(loc);
    for (auto & prv: loc_set) {
        auto prob = tran_prob.at(prv);
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
    if (T == 0) throw std::runtime_error("T == 0");
    if (markovs.size() != T - 1) throw std::runtime_error("markovs.size() != T - 1");
    
    std::unordered_map<LocationPtr, Prob> example;
    std::unordered_map<LocationPtr, LocationPtr> example2;

    example.reserve(N);
    example2.reserve(N);

    for(auto&& loc : *loc_set) {
        example[loc];
        example2[loc];
    }
    
    std::vector<std::unordered_map<LocationPtr, Prob>> dp(T, example);
    std::vector<std::unordered_map<LocationPtr, LocationPtr>> psi(T - 1, example2);

    auto init = [&init_prob, &emission_probs, &dp, this](size_t t) {
#ifdef DEBUG
        std::cout << "init(" << t << ')' << std::endl;
#endif
        bool all_zero = true;
        auto & cur = dp[t];
        for (auto && loc : *loc_set) {
            auto p = init_prob.at(loc) * emission_probs[t].at(loc);
            if (p != Prob::ZERO) all_zero = false;
            cur[loc] = p;
        }
        if (all_zero) {
            throw std::runtime_error("all zero for t = " + std::to_string(t));
        }
    };

    std::vector<LocationPtr> ret(T);
    auto recover = [&ret, &psi, &dp](int s, int t) {
        ret[t] = std::ranges::max_element(dp[t], [](auto const & lhs, auto const & rhs) { return lhs.second < rhs.second; })->first;
        for (int i = t - 1; i >= s; --i) {
            ret[i] = psi[i][ret[i + 1]];
            if (!ret[i]) {
#ifdef DEBUG
                std::cerr << i << " is null" << std::endl;
#endif
                ret[i] = std::ranges::max_element(dp[i], [](auto const & lhs, auto const & rhs) { return lhs.second < rhs.second; })->first;
            }
        }
    };
    
    // t = 0
    size_t start = 0;
    init(0);
    for (size_t t = 1; t < T; ++t) {
#ifdef DEBUG
        std::cout << "t = " << t << std::endl;
#endif
        auto const& markov = *markovs[t - 1];
        auto& et = emission_probs[t];
        auto& pt = psi[t - 1];
        auto& prv = dp[t - 1], &cur = dp[t];
        for (auto && loc : *loc_set) pt.emplace(loc, nullptr);
        // cur.clear();
        bool all_zero = true;
        std::for_each(std::execution::par_unseq, loc_set->begin(), loc_set->end(), 
            [this, &all_zero, &prv, &markov, &et, &cur, &pt](LocationPtr const & loc) {
            Prob max_prob;
            LocationPtr max_loc;
            auto & tran_prob = markov.get_tran_prob().at(loc);
            for (auto&& prev_loc : *loc_set) {
                // markov[prv_loc][loc]: can optimize for locality
                try {
                    Prob prob = prv[prev_loc] * tran_prob.at(prev_loc);
                    if (prob > max_prob) {
                        max_prob = prob;
                        max_loc = prev_loc;
                    }
                } catch (std::out_of_range &e) {
                    std::cerr << "fuck 1:" << loc->point << ";" << prev_loc->point << std::endl;
                    throw e;
                }
            }
            try {
                max_prob *= et.at(loc);
            } catch (std::out_of_range & e) {
                std::cerr << "fuck 2:" << loc->point << std::endl;
                throw e;
            }
            if (max_prob > 0) all_zero = false;
            cur[loc] = max_prob;
            pt[loc] = max_loc;
        });
        if (all_zero) {
            std::cout << t << ": re-init" << std::endl;
            // recover path
            recover(start, t - 1);
            // re-init
            start = t;
            init(start);
        }
    }
    recover(start, T - 1);
#ifdef DEBUG
    std::cout << "viterbi done" << std::endl;
#endif
    return ret;
}

}  // namespace rxy