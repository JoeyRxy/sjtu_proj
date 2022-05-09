#pragma once

#include <configure.hpp>
#include <list>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <gsl/gsl_spline.h>

#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>

#include "line_parser/parser.h"

namespace rxy {

using rsrp_t = RSRP_TYPE;

inline bool load_data(
    std::string const& file,
    std::unordered_map<int, std::unordered_map<int, std::list<rsrp_t>>>& loc_pci_map) {
    std::ifstream in(file);
    if (in.fail()) {
        std::cerr << "Failed to open file" << std::endl;
        in.close();
        return false;
    }
    Parser parser(in);     // 构造时传入需要进行解析的文本
    if (parser.parse()) {  // parse()进行解析；返回一个bool值表示是否解析成功
        for (auto const& cellinfo : parser.get()) {  // parser.get() 获取所有的CellInfo
            auto& pci_rsrp_map = loc_pci_map[cellinfo.loc];
            for (auto&& [pci, rsrp] : cellinfo.pci_info_list) {
                pci_rsrp_map[pci].push_back(rsrp->rsrp);
            }
        }
    } else {
        in.close();
        return false;
    }
    in.close();
    return true;
}

/**
 * load the data which contains X aligned by specific pci_order
 * @param file: data_file path
 * @param loc_data_aligned: loc_data_aligned[loc] = [
 * [rsrp for pci1, rsrp for pci2, ...],
 * [rsrp for pci1, rsrp for pci2, ...],
 * ...
 * ]
 * @param pci_order: ordered pci list
 * */
inline bool load_data_aligned(
    std::string const& file,
    std::list<std::pair<int, std::vector<rsrp_t>>>& loc_data_aligned,
    std::vector<int> const& pci_order, rsrp_t default_rsrp = -140) {
    std::ifstream in(file);
    if (in.fail()) {
        std::cerr << "Failed to open file" << std::endl;
        in.close();
        return false;
    }
    std::unordered_map<int, int> idx_map;
    for (int i = 0; i < pci_order.size(); ++i) {
        idx_map[pci_order[i]] = i;
    }
    Parser parser(in);  // 构造时传入需要进行解析的文本
    if (parser.parse()) {
        for (auto const& cellinfo : parser.get()) {
            // auto& rsrp_aligned =
            //     loc_data_aligned[cellinfo.loc].emplace_back(pci_order.size(), default_rsrp);
            std::vector<rsrp_t> rsrp_aligned(pci_order.size(), default_rsrp);
            for (auto&& [pci, rsrp] : cellinfo.pci_info_list) {
                try {
                    rsrp_aligned[idx_map.at(pci)] = rsrp->rsrp;
                } catch (std::out_of_range&) {
                }
            }
            loc_data_aligned.emplace_back(cellinfo.loc, std::move(rsrp_aligned));
        }
    } else {
        in.close();
        return false;
    }
    in.close();
    return true;
}

namespace detail {

inline void __get_train_test_data_helper(std::vector<std::pair<int, std::vector<rsrp_t>>>&& X,
                                  std::vector<std::vector<rsrp_t>>& X_train,
                                  std::vector<int>& y_train,
                                  std::vector<std::vector<rsrp_t>>& X_test,
                                  std::vector<int>& y_test, double _split_ratio = 0.8) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(X.begin(), X.end(), g);
    std::size_t split_idx = X.size() * _split_ratio;
    X_train.reserve(split_idx);
    y_train.reserve(split_idx);
    X_test.reserve(X.size() - split_idx);
    y_test.reserve(X.size() - split_idx);
    for (std::size_t i = 0; i < split_idx; ++i) {
        X_train.emplace_back(std::move(X[i].second));
        y_train.emplace_back(X[i].first);
    }
    for (std::size_t i = split_idx; i < X.size(); ++i) {
        X_test.emplace_back(std::move(X[i].second));
        y_test.emplace_back(X[i].first);
    }
}

}  // namespace detail

inline auto get_train_test_data(
    std::list<std::pair<int, std::vector<rsrp_t>>>const& loc_data_aligned,
    double _split_ratio = 0.8) {
    std::vector<std::pair<int, std::vector<rsrp_t>>> X;
    X.reserve(loc_data_aligned.size());
    for (auto& p : loc_data_aligned) {
        X.emplace_back(p);
    }
    std::vector<std::vector<rsrp_t>> X_train;
    std::vector<int> y_train;
    std::vector<std::vector<rsrp_t>> X_test;
    std::vector<int> y_test;
    detail::__get_train_test_data_helper(std::move(X), X_train, y_train, X_test, y_test,
                                         _split_ratio);
    return std::make_tuple(std::move(X_train), std::move(y_train), std::move(X_test),
                           std::move(y_test));
}

inline auto get_train_test_data(
    std::list<std::pair<int, std::vector<rsrp_t>>>&& loc_data_aligned,
    double _split_ratio = 0.8) {
    std::vector<std::pair<int, std::vector<rsrp_t>>> X;
    X.reserve(loc_data_aligned.size());
    for (auto& p : loc_data_aligned) {
        X.emplace_back(std::move(p));
    }
    std::vector<std::vector<rsrp_t>> X_train;
    std::vector<int> y_train;
    std::vector<std::vector<rsrp_t>> X_test;
    std::vector<int> y_test;
    detail::__get_train_test_data_helper(std::move(X), X_train, y_train, X_test, y_test,
                                         _split_ratio);
    return std::make_tuple(std::move(X_train), std::move(y_train), std::move(X_test),
                           std::move(y_test));
}

inline std::set<int> get_pci_set(
    std::unordered_map<int, std::unordered_map<int, std::list<rsrp_t>>> const& loc_pci_map) {
    std::set<int> pci_set;
    for (auto&& [_, pci_rsrp_map] : loc_pci_map) {
        for (auto&& [pci, _] : pci_rsrp_map) {
            pci_set.insert(pci);
        }
    }
    return pci_set;
}

inline std::map<int, int> get_pci_map(
    std::unordered_map<int, std::unordered_map<int, std::list<rsrp_t>>> const& loc_pci_map) {
    std::map<int, int> pci_map;
    for (auto&& [_, pci_rsrp_map] : loc_pci_map) {
        for (auto&& [pci, _] : pci_rsrp_map) {
            pci_map[pci]++;
        }
    }
    return pci_map;
}

/**
 * compute mean and variance of rsrp_vector
 * */
template <typename InputIterator>
requires std::input_iterator<InputIterator> &&
    std::same_as<typename std::iterator_traits<InputIterator>::value_type, rsrp_t>
inline std::pair<double, double> get_mean_var(InputIterator begin, InputIterator end, int n = -1,
                                              bool compute_min = true) {
    if (n == -1) n = std::distance(begin, end);
    if (!(n > 0)) throw std::runtime_error("invalid argument");
    if (compute_min) {
        double sum = std::accumulate(begin, end, 0);
        double mean = sum / n;
        sum = std::accumulate(begin, end, 0, [mean](double sum, double rsrp) {
            return sum + (rsrp - mean) * (rsrp - mean);
        });
        double variance = sum / n;
        return {mean, variance};
    } else {
        double sum = std::accumulate(
            begin, end, 0, [](rsrp_t sum, rsrp_t rsrp) { return rsrp == -140 ? sum : sum + rsrp; });
        double mean = sum / n;
        sum = std::accumulate(begin, end, 0, [mean](double sum, double rsrp) {
            return rsrp == -140 ? sum : sum + (rsrp - mean) * (rsrp - mean);
        });
        double variance = sum / n;
        return {mean, variance};
    }
}

}  // namespace rxy
