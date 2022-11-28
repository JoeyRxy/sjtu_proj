#pragma once
#include <cmath>
#include <configure.hpp>
#include <iostream>

namespace rxy {

struct Prob {
    static const Prob ZERO, ONE;
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

    constexpr Prob& operator+=(Prob const &rhs) {
        if (this->prob == -std::numeric_limits<value_type>::infinity()) {
            this->prob = rhs.prob;
        } else if (rhs.prob != -std::numeric_limits<value_type>::infinity()) {
            this->prob = std::log1p(std::exp(this->prob) + std::exp(rhs.prob) - 1);
        }
        return *this;
    }

    value_type operator*() const { return std::exp(prob); }

};

inline const Prob Prob::ZERO;
inline const Prob Prob::ONE(0, true);

inline Prob operator*(Prob const &lhs, Prob const &rhs) { return Prob(lhs.prob + rhs.prob, true); }

inline Prob operator/(Prob const &lhs, Prob const &rhs) { return Prob(lhs.prob - rhs.prob, true); }

inline Prob operator+(Prob const &lhs, Prob const &rhs) { 
    Prob ret(lhs);
    ret += rhs;
    return ret; 
}

inline std::ostream& operator<<(std::ostream& os, Prob const &prob) {
    os << std::exp(prob.prob);
    return os;
}

/**
 * overload all logic operators for Prob
 */
inline constexpr bool operator==(Prob const &lhs, Prob const &rhs) { return lhs.prob == rhs.prob; }
inline constexpr bool operator!=(Prob const &lhs, Prob const &rhs) { return lhs.prob != rhs.prob; }
inline constexpr bool operator<(Prob const &lhs, Prob const &rhs) { return lhs.prob < rhs.prob; }
inline constexpr bool operator<=(Prob const &lhs, Prob const &rhs) { return lhs.prob <= rhs.prob; }
inline constexpr bool operator>(Prob const &lhs, Prob const &rhs) { return lhs.prob > rhs.prob; }
inline constexpr bool operator>=(Prob const &lhs, Prob const &rhs) { return lhs.prob >= rhs.prob; }

static const Prob::value_type INV_SQRT_PI = 1. / std::sqrt(2 * 3.141592653589793238462643383279502884L);

inline constexpr Prob nd_pdf(Prob::value_type const x, Prob::value_type mu, Prob::value_type sigma) {
    if (sigma == 0) return x == mu;
    Prob::value_type t = INV_SQRT_PI / sigma;
    Prob::value_type a = 1. / sigma;
    Prob::value_type y = (x - mu) * a;
    Prob::value_type r = -0.5 * y * y;
    return t * std::exp(r);
}

static const Prob::value_type INV_SQRT_PI_LOG = -0.5 * std::log(2 * 3.141592653589793238462643383279502884L);

inline constexpr Prob log_nd_pdf(Prob::value_type const x, Prob::value_type const mu, Prob::value_type const sigma) {
    if (sigma == 0) return x == mu;
    Prob::value_type t = INV_SQRT_PI_LOG - std::log(sigma);
    Prob::value_type a = 1. / sigma;
    Prob::value_type y = (x - mu) * a;
    Prob::value_type r = t - 0.5 * y * y;
    return {r, true};
}

static const Prob::value_type INV_SQRT_PI_SIGMA_LOG = INV_SQRT_PI_LOG - std::log(ND_SIGMA);

inline Prob log_nd_pdf(Prob::value_type const x) {
    Prob::value_type y = (x - ND_MU) / ND_SIGMA;
    Prob::value_type r = INV_SQRT_PI_SIGMA_LOG - 0.5 * y * y;
    return {r, true};
}

}  // namespace rxy