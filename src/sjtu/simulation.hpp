#pragma once
#include <armadillo>
#include <cmath>
#include <list>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rxy {

static inline auto dbm(arma::mat const &r) {
    arma::mat z = -23 - 40 * arma::log10(r);
    z.transform([](double val) { return std::max(std::min(val, -44.), -140.); });
    return z;
}

class Simulator {
public:
    Simulator(double a, double b, double step,
              std::list<std::pair<double, double>> const &pci_locs)
        : a(a), b(b), step(step), m(a / step), n(b / step) {
        arma::vec X = arma::linspace(0, a, m);
        arma::mat Nx(m, n), Ny(m, n);
        Nx.each_col() = X;
        arma::rowvec Y = arma::linspace(0, b, n).st();
        Ny.each_row() = Y;

        res.reserve(pci_locs.size());

        for (auto &&[x, y] : pci_locs) {
            res.push_back(dbm(
                arma::sqrt(arma::pow(Nx - x, 2) + arma::pow(Ny - y, 2))));
        }
    }

    auto &get(int i) const { return res.at(i); }

    double get(int i, double x, double y) const {
        if (x >= a || x < 0 || y >= b || y < 0)
            throw std::out_of_range("index out of range: (" +
                                    std::to_string(x) + ", " +
                                    std::to_string(y) + ')');
        return res.at(i)(x / step, y / step);
    }

    arma::vec get(double x, double y) {
        if (x >= a || x < 0 || y >= b || y < 0)
            throw std::out_of_range("index out of range: (" +
                                    std::to_string(x) + ", " +
                                    std::to_string(y) + ')');
        arma::uword i = x / step, j = y / step;
        arma::vec rsrp(res.size());
        // for (auto && m : res) rsrp.push_back(m(i, j));
        for (size_t k = 0; k < res.size(); ++k) rsrp(k) = res[k](i, j);
        return rsrp;
    }

private:
    double a, b, step;
    int m, n;
    std::vector<arma::mat> res;
};

} // namespace rxy
