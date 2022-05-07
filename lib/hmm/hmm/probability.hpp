#pragma once
#include <cmath>
#include <configure.hpp>

namespace rxy {

struct Prob {
    using value_type = PROB_TYPE;
    value_type prob;

    constexpr Prob() : prob(-std::numeric_limits<value_type>::infinity()) {}
    constexpr Prob(value_type prob, bool is_log = false) {
        if (is_log) {
            this->prob = prob;
        } else {
            this->prob = std::log1p(prob - 1);
        }
    }

    constexpr Prob& operator*=(Prob const &rhs) {
        this->prob += rhs.prob;
        return *this;
    }

    constexpr Prob& operator/=(Prob const &rhs) {
        this->prob -= rhs.prob;
        return *this;
    }

};

inline constexpr Prob operator*(Prob const &lhs, Prob const &rhs) { return Prob(lhs.prob + rhs.prob, true); }

inline constexpr Prob operator/(Prob const &lhs, Prob const &rhs) { return Prob(lhs.prob - rhs.prob, true); }

/**
 * overload all logic operators for Prob
 */
inline constexpr bool operator==(Prob const &lhs, Prob const &rhs) { return lhs.prob == rhs.prob; }
inline constexpr bool operator!=(Prob const &lhs, Prob const &rhs) { return lhs.prob != rhs.prob; }
inline constexpr bool operator<(Prob const &lhs, Prob const &rhs) { return lhs.prob < rhs.prob; }
inline constexpr bool operator<=(Prob const &lhs, Prob const &rhs) { return lhs.prob <= rhs.prob; }
inline constexpr bool operator>(Prob const &lhs, Prob const &rhs) { return lhs.prob > rhs.prob; }
inline constexpr bool operator>=(Prob const &lhs, Prob const &rhs) { return lhs.prob >= rhs.prob; }

static constexpr Prob::value_type INV_SQRT_PI = 1. / std::sqrt(2 * 3.141592653589793238462643383279502884);

inline constexpr Prob nd_pdf(Prob::value_type const x, Prob::value_type mu, Prob::value_type sigma) {
    if (sigma == 0) return x == mu;
    Prob::value_type t = INV_SQRT_PI / sigma;
    Prob::value_type a = 1. / sigma;
    Prob::value_type y = (x - mu) * a;
    Prob::value_type r = -0.5 * y * y;
    return t * std::exp(r);
}

static constexpr Prob::value_type INV_SQRT_PI_LOG = -0.5 * std::log(2 * 3.141592653589793238462643383279502884);

inline constexpr Prob log_nd_pdf(Prob::value_type const x, Prob::value_type const mu, Prob::value_type const sigma) {
    if (sigma == 0) return x == mu;
    Prob::value_type t = INV_SQRT_PI_LOG - std::log(sigma);
    Prob::value_type a = 1. / sigma;
    Prob::value_type y = (x - mu) * a;
    Prob::value_type r = t - 0.5 * y * y;
    return {r, true};
}

}  // namespace rxy