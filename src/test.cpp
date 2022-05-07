#include <bits/stdc++.h>
#include "hmm/hmm.hpp"
#include "hmm/knn.hpp"
#include "sjtu/loc_markov.hpp"
#include "sjtu/location_map.hpp"
#include "sjtu/max_a_posteri.hpp"
#include "sjtu/util.hpp"

#include <Eigen/Dense>
#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>

using namespace std;
using namespace rxy;

int test_knn() {
    string file = "../data/cellinfo-campus_040312.txt";
    // string file = "../data/cellinfo-real.txt";
    std::unordered_map<int, std::list<std::vector<rsrp_t>>> loc_data_aligned;
    vector<int> pci_order = {908, 923, 936, 663, 100, 906, 457, 665, 223, 922, 748, 238};
    // vector<int> pci_order = {902, 324, 933, 934, 63, 807, 326, 143, 92, 365, 226, 662, 31, 942, 138, 140, 901, 325, 948, 945, 946, 364};
    sort(pci_order.begin(), pci_order.end());
    if (load_data_aligned(file, loc_data_aligned, pci_order)) {
        cout << "load data success" << endl;
    } else {
        cout << "load data failed" << endl;
        return -1;
    }
    auto [X_train, y_train, X_test, y_test] = get_train_test_data(move(loc_data_aligned), 0.8);
    KNN<rsrp_t> knn(10, KNN<rsrp_t>::distance_inv_weighted_euc);
    knn.train(X_train, y_train);
    int cnt = 0;
    for (int i = 0; i < X_test.size(); ++i) {
        int pred = knn.predict(X_test[i]);
        if (pred == y_test[i]) ++cnt;
        else cout << "pred: " << pred << "\ty: " << y_test[i] << endl;
    }
    cout << "accuracy: " << cnt * 1.0 / X_test.size() << endl;
    return 0;
}

LocationMap load_loc_map();

void test_map() {
    string file = "../data/cellinfo-campus_040312.txt";
    auto loc_map = load_loc_map();
    unordered_map<int, unordered_map<int, list<rsrp_t>>> loc_pci_map;
    if (load_data(file, loc_pci_map)) {
        cout << "load data success" << endl;
    } else {
        cerr << "load data failed" << endl;
    }
    MaxAPosteri __map(loc_map, loc_pci_map);
    for (auto [loc, pci_rsrp_list] : loc_pci_map) {
        list<pair<int, rsrp_t>> rsrp_list;
        for (auto [pci, rsrps] : pci_rsrp_list) {
            for (auto rsrp : rsrps) {
                rsrp_list.emplace_back(pci, rsrp);
            }
        }
        auto loc_prob_map = __map(rsrp_list);
        LocationPtr max_loc = nullptr;
        Prob max_prob;
        for (auto [loc, prob] : loc_prob_map) {
            if (prob >= max_prob) {
                max_loc = loc;
                max_prob = prob;
            }
        }
        cout << "loc: " << loc << "\t" << max_loc->id << ";" << max_prob.prob << endl;
    }
}

void test_eigen() {
    Eigen::MatrixXd A(2, 2);
    A << 1, 2, 3, 4;
    Eigen::MatrixXd B(2, 2);
    B << 5, 6, 7, 8;
    Eigen::MatrixXd C = A * B;
    cout << C << endl;
}

void test_interpolate() {
    vector<double> x(15);
    vector<double> y(x.size());
    for (int i = 0; i < x.size(); ++i) {
        x[i] = i;
        y[i] = sin(x[i]);
    }
    gsl_interp_accel *acc = gsl_interp_accel_alloc();
    gsl_spline *spline = gsl_spline_alloc(gsl_interp_cspline, x.size());
    gsl_spline_init(spline, x.data(), y.data(), x.size());
    double delta = 0.1;
    int N = (x.back() - x.front()) / delta;
    vector<double> x_new;
    x_new.reserve(N);
    vector<double> y_new;
    y_new.reserve(N);
    for (double xi = x.front(); xi <= x.back(); xi += delta) {
        x_new.push_back(xi);
        y_new.push_back(gsl_spline_eval(spline, xi, acc));
    }
    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);
    for (int i = 0; i < x_new.size(); ++i) {
        cout << setprecision(12) << x_new[i] << "\t" << y_new[i] << "\ttrue val: " << sin(x_new[i]) << endl;
    }
}

void test_interpolate2d() {
    // prepare data
    auto func = [](double x, double y) { return 100 * exp(-0.5 * (x * x + y * y)); };
    double start = -2, end = 2, delta = 0.5;
    int N = (end - start) / delta;
    Eigen::VectorXd x(N), y(N);
    x[0] = start;
    y[0] = start;
    for (int i = 1; i < N; ++i) {
        x[i] = x[i - 1] + delta;
        y[i] = y[i - 1] + delta;
    }
    Eigen::MatrixXd grid(x.size(), y.size());
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < y.size(); ++j) {
            grid(i, j) = func(x[i], y[j]);
        }
    }
    cout << "old grid:\n" << grid << endl;
    // gsl_spline2d * spline = gsl_spline2d_alloc(gsl_interp2d_bilinear, x.size(), y.size());
    gsl_spline2d * spline = gsl_spline2d_alloc(gsl_interp2d_bicubic, x.size(), y.size());
    gsl_interp_accel * acc_x = gsl_interp_accel_alloc();
    gsl_interp_accel * acc_y = gsl_interp_accel_alloc();
    // interpolate
    gsl_spline2d_init(spline, x.data(), y.data(), grid.data(), x.size(), y.size());
    double delta_x = 0.01;
    double delta_y = 0.01;
    int N_x = (x[x.size() - 1] - x[0]) / delta_x;
    int N_y = (y[y.size() - 1] - y[0]) / delta_y;
    Eigen::VectorXd x_new(N_x), y_new(N_y);
    x_new[0] = x[0], y_new[0] = y[0];
    Eigen::MatrixXd grid_new(N_x, N_y);
    for (int i = 1; i < N_x; ++i) {
        x_new[i] = x_new[i - 1] + delta_x;
    }
    for (int i = 1; i < N_y; ++i) {
        y_new[i] = y_new[i - 1] + delta_y;
    }
    for (int i = 0; i < N_x; ++i) {
        for (int j = 0; j < N_y; ++j) {
            grid_new(i, j) = gsl_spline2d_eval(spline, x_new[i], y_new[j], acc_x, acc_y);
        }
    }
    gsl_spline2d_free(spline);
    gsl_interp_accel_free(acc_x);
    gsl_interp_accel_free(acc_y);
    // cout << "new grid:\n" << grid_new << endl;
    double avg_diff = 0, max_diff = 0;
    for (int i = 0; i < N_x; ++i) {
        for (int j = 0; j < N_y; ++j) {
            double _x = x_new[i], _y = y_new[j], _z = func(_x, _y);
            double diff = abs(_z - grid_new(i, j));
            max_diff = max(max_diff, diff);
            avg_diff += diff;
            // cout << setprecision(10) << _x << ", " << _y << "\t" << _z 
            //     << "\t" << grid_new(i, j) << "\tdiff:" << diff << endl;
        }
    }
    cout << "avg diff: " << avg_diff / (N_x * N_y) << endl;
    cout << "max diff: " << max_diff << endl;
}
