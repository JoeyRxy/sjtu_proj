#include "freq.h"
#include <exception>
#include <iterator>
#include <numeric>

using iter_t = std::list<std::vector<RSRP_TYPE>>::const_iterator;

std::vector<double> calc(iter_t begin, iter_t end, double lambda = 1.) {
    const int N = begin->size();
    std::vector<double> arr(98 * N);
    std::vector<std::vector<int>> cnt(N, std::vector<int>(97, 0));
    std::vector<int> S(N, 0);
    for (; begin != end; ++begin) {
        auto & rsrp = *begin;
        for (int i = 0; i < N; ++i) {
            auto v = static_cast<int>(rsrp[i]);
            if (v <= -140) {
                continue;
            }
            if (v > -44) v = -44;
            cnt[i][v + 140]++;
            S[i]++;
        }
    }
    auto s = static_cast<double>(std::accumulate(S.begin(), S.end(), 0));
    for (int i = 0; i < N; ++i) {
        int start = i * 98;
        double ss = S[i];
        arr[start] = ss / s * lambda;
        for (int v = 0; v <= 96; ++v) {
            arr[start + 1 + v] = cnt[i][v] / ss;
        }
    }
    return arr;
}

std::unordered_map<int, std::vector<std::vector<double>>> analyze(std::unordered_map<int, std::list<std::vector<RSRP_TYPE>>> const & m, int duration) {
    std::unordered_map<int, std::vector<std::vector<double>>> ans;
    for (auto && [loc, ls] : m) {
        auto & freq_ls =  ans[loc];
        if (duration > ls.size()) {
            throw std::exception("duration too large");
        }
        freq_ls.reserve(ls.size() - duration + 1);
        iter_t it = ls.cbegin(), jt = it;
        std::advance(jt, duration);
        while (jt != ls.cend()) {
            freq_ls.push_back(calc(it, jt));
            ++it, ++jt;
        }
    }
    return ans;
}
