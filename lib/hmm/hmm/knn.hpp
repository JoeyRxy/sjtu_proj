#pragma once
#include <cmath>
#include <functional>
#include <vector>
#include <stdexcept>

namespace rxy {

template <typename T>
requires std::is_arithmetic_v<T>
class KNN {
   private:
    int N_;
    int label_num;
    int const topk;
    std::function<double(const std::vector<T>&, const std::vector<T>&)> distance;
    std::vector<std::vector<T>> const* data;
    std::vector<int> const* labels;

   public:
    KNN(int topk, std::function<double(const std::vector<T>&, const std::vector<T>&)> const&
                      distance_function)
        : topk(topk), distance(distance_function) {}
    
    KNN(int topk, std::function<double(const std::vector<T>&, const std::vector<T>&)> &&
                      distance_function)
        : topk(topk), distance(std::move(distance_function)) {}
    

    ~KNN() = default;

    void train(std::vector<std::vector<T>> const& data, std::vector<int> const& labels) {
        N_ = data.size();
        assert(data.size() > 0);
        this->data = &data;
        assert(data.size() == labels.size());
        this->labels = &labels;
        label_num = *std::max_element(labels.begin(), labels.end()) + 1;
    }

    int predict(std::vector<T> const& X) {
        std::vector<std::pair<double, int>> distances;
        distances.reserve(N_);
        for (int i = 0; i < N_; ++i) {
            distances.emplace_back(distance(X, (*data)[i]), (*labels)[i]);
        }
        std::nth_element(distances.begin(), distances.begin() + topk, distances.end(),
                        [](std::pair<double, int> const& a, std::pair<double, int> const& b) {
                            return a.first < b.first;
                        });
        std::unordered_map<int, double> label_weight;
        for (int i = 0; i < topk; ++i) {
            if (distances[i].first == 0) {
                return distances[i].second;
            } else {
                label_weight[distances[i].second] += 1.0 / distances[i].first;
            }
        }
        double _max = std::numeric_limits<double>::min();
        int _max_label = -1;
        for (auto [label, weight] : label_weight) {
            if (weight > _max) {
                _max = weight;
                _max_label = label;
            }
        }
        if (_max_label == -1) {
            throw std::runtime_error("KNN::predict: no label found");
        }
        return _max_label;
    }

    std::vector<double> predict_prob(std::vector<T> const & X) {
        std::vector<std::pair<double, int>> distances;
        distances.reserve(data->size());
        for (int i = 0; i < X.size(); ++i) {
            distances.emplace_back(distance(X, (*data)[i]), (*labels)[i]);
        }

        std::nth_element(distances.begin(), distances.begin() + topk, distances.end(),
                        [](std::pair<double, int> const& a, std::pair<double, int> const& b) {
                            return a.first < b.first;
                        });

        std::unordered_map<int, double> label_weight;
        std::vector<double> label_prob(label_num, 0.0);
        double S = 0;
        for (int i = 0; i < topk; ++i) {
            if (distances[i].first == 0) {
                label_prob[distances[i].second] = 1.0;
                return label_prob;
            } else {
                double weight = 1.0 / distances[i].first;
                S += weight;
                label_weight[distances[i].second] += weight;
            }
        }
        for (auto [label, weight] : label_weight) {
            label_prob[label] = weight / S;
        }
        return label_prob;
    }

    static inline double distance_euclidean(std::vector<T> const& x, std::vector<T> const& y) {
        double sum = 0;
        for (int i = 0; i < x.size(); ++i) {
            sum += (x[i] - y[i]) * (x[i] - y[i]);
        }
        return sqrt(sum);
    }

    static inline double distance_minkowski(std::vector<T> const& x, std::vector<T> const& y,
                                            int p) {
        double sum = 0;
        for (int i = 0; i < x.size(); ++i) {
            sum += pow(x[i] - y[i], p);
        }
        return pow(sum, 1.0 / p);
    }

    static inline double distance_weighted_euc(std::vector<T> const& x, std::vector<T> const& y) {
        double sum = 0;
        T t;
        for (int i = 0; i < x.size(); ++i) {
            t = (x[i] - y[i]) * x[i];
            sum += t * t;
        }
        return sqrt(sum);
    }

    static inline double distance_inv_weighted_euc(std::vector<T> const& x, std::vector<T> const& y) {
        double sum = 0, t;
        if (std::is_integral_v<T>) {
            for (int i = 0; i < x.size(); ++i) {
                t = (x[i] - y[i]) / static_cast<double>(x[i]);
                sum += t * t;
            }
        } else {
            for (int i = 0; i < x.size(); ++i) {
                t = (x[i] - y[i]) / x[i];
                sum += t * t;
            }
        }
        
        return sqrt(sum);
    }
    
};

}  // namespace rxy