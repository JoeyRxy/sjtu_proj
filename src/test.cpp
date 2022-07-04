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
#include "sjtu/interp.hpp"

#include <gsl/gsl_interp2d.h>
#include <gsl/gsl_spline2d.h>

#include "cout_color.hpp"
#include "registry.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

using namespace std;
using namespace rxy;

unordered_map<int, unordered_map<int, list<rsrp_t>>> load_original_data(string const& file);

LocationMap load_loc_map();

RUN_OFF(knn) {
    cout << __color::bg_blu() << "--- knn ---" << __color::bg_def() << endl;
    // string file = "../data/cellinfo-campus_040312.txt";
    string file = "../data/new/train.txt";
    vector<int> pci_order = {117, 118, 314, 331, 997, 998};
    auto knn = get_knn(file, pci_order);
    int total = 0, cnt = 0;
    string test_file = "../data/new/test.txt";
    std::list<std::pair<int, std::vector<rsrp_t>>> test_data_aligned;
    if (load_data_aligned(test_file, test_data_aligned, pci_order)) {
        cout << "load test data success" << endl;
    } else {
        cout << "load test data failed" << endl;
        return;
    }

    auto loc_map = load_loc_map();
    double rmse = 0;
    for (auto&& [loc, data] : test_data_aligned) {
        total++;
        cout << "t = " << total << endl;
        int pred = knn.predict(data);
        if (pred == loc) {
            cout << __color::gre() << "\t" << loc_map.get_loc(loc)->point << __color::def() << endl;
            ++cnt;
        } else {
            double dist = minkowski(loc_map.get_loc(loc)->point, loc_map.get_loc(pred)->point);
            cout << "\t" << __color::gre() << loc_map.get_loc(loc)->point << __color::def()
                 << " vs. " << __color::red() << loc_map.get_loc(pred)->point << __color::def()
                 << ": " << dist << endl;
            rmse += dist * dist;
        }
    }
    cout << "accuracy: " << static_cast<double>(cnt) / total << endl;
    cout << "RMSE: " << sqrt(rmse / total) << endl;
}

inline void check_emission_prob(EmissionProb emission_prob) {
    map<Prob::value_type, int, greater<Prob::value_type>> prob_loc_map;
    for (auto&& [_, prob] : emission_prob) {
        prob_loc_map[*prob]++;
    }
    for (auto&& [prob, loc_ls] : prob_loc_map) {
        cout << prob << ": " << loc_ls << endl;
    }
}

inline void check_markov(MarkovPtr markov, std::list<LocationPtr> const & loc_ls) {
    for (auto it = loc_ls.begin(); it != loc_ls.end(); ++it) {
        cout << __color::bg_blu() << (*it)->point << __color::bg_def() << endl;
        for (auto jt = loc_ls.begin(); jt != loc_ls.end(); ++jt) {
            auto prob = *(*markov)(*it, *jt);
            if (prob > 0.01)
                cout << "\t-> " << (*jt)->point << ": " << prob << '\n';
        }
    }
}

RUN(procedure) {
    string file = "../data/new/train.txt";
    string sensor_file = "../data/new/test_sensor.txt";
    string test_file = "../data/new/test.txt";

    int top_k = 3000;

    vector<int> pci_order = {117, 118, 314, 331, 997, 998};
    // ---- location map ----
    auto loc_map = load_loc_map();
    // ------ sensation & markov ------
    auto markovs = get_markov(sensor_file, loc_map);
    // check_markov(markovs[0], loc_map.get_loc_set());
    int T = markovs.size() + 1;
    auto knn = get_knn(file, pci_order, top_k);
    std::list<std::pair<int, std::vector<rsrp_t>>> test_data_aligned;
    load_data_aligned(test_file, test_data_aligned, pci_order);

    for (auto & [loc, _] : test_data_aligned) {
        cout << loc_map.get_loc(loc)->point << endl;
    }

    cout << " =================== "<< endl;

    // ===== knn =====
    cout << __color::bg_blu() << "--- KNN ---" << __color::bg_def() << endl;
    int total = 0;
    int knn_cnt = 0;
    double knn_rmse = 0;
    for (auto&& [loc, data] : test_data_aligned) {
        total++;
        cout << "t = " << total << endl;
        int pred = knn.predict(data);
        if (pred == loc) {
            cout << __color::gre() << "\t" << loc_map.get_loc(loc)->point << __color::def() << endl;
            ++knn_cnt;
        } else {
            double dist = minkowski(loc_map.get_loc(loc)->point, loc_map.get_loc(pred)->point);
            cout << "\t" << __color::gre() << loc_map.get_loc(loc)->point << __color::def()
                 << " vs. " << __color::red() << loc_map.get_loc(pred)->point << __color::def()
                 << ": " << dist << endl;
            knn_rmse += dist * dist;
        }
    }
    
    cout << __color::bg_blu() << "--- HMM ---" << __color::bg_def() << endl;

    // ------ load data ------
    vector<EmissionProb> emission_probs;
    vector<LocationPtr> locations;
    cout << "get emission prob ..." << endl;
    get_emission_prob_using_knn(test_data_aligned, knn, loc_map, emission_probs, locations, T);
    cout << "GOT" << endl;
    // {
    //     int t = 0;
    //     for (auto & emit_prob : emission_probs) {
    //         cout << __color::bg_blu() << "T = " << ++t << __color::bg_def() << endl;
    //         check_emission_prob(emit_prob);
    //         cout << " ======================= " << endl;
    //     }
    // }
    // ------ init prob -------
    unordered_map<LocationPtr, Prob> init_probs;
    for (auto&& loc : loc_map.get_ext_list()) {
        // init_probs[loc] = emission_probs[0][loc];
        init_probs[loc] = Prob::ONE;
    }
    // ------ hmm ------
    cout << "viterbi ..." << endl;
    auto pred_locs = HMM{loc_map.get_ext_list()}.viterbi(markovs, init_probs, emission_probs);
    cout << "GOT" << endl;
    int cnt = 0;
    double rmse = 0;
    for (int t = 0; t < T; ++t) {
        cout << "t = " << (t + 1) << endl;
        auto locptr = locations[t];
        auto pred_locptr = pred_locs[t];
        if (locptr == nullptr) {
            cout << "locptr is null" << endl;
        }
        if (pred_locptr == nullptr) {
            cout << "pred_locptr is null" << endl;
        }
        if (locptr->id == 
            pred_locptr->id) {
            ++cnt;
            cout << __color::gre() << "\t" << pred_locs[t]->point << __color::def() << endl;
        } else {
            double dist = minkowski(locations[t]->point, pred_locs[t]->point);
            cout << "\t" << __color::gre() << locations[t]->point << __color::def() << " vs. "
                 << __color::red() << pred_locs[t]->point << __color::def() << ": "
                 << dist << endl;
            rmse += dist * dist;
        }
    }

    cout << "HMM's accuracy = " << (double)cnt / T << endl;
    cout << "HMM's RMSE: " << sqrt(rmse / T) << endl;
    cout << "KNN's accuracy: " << static_cast<double>(knn_cnt) / total << endl;
    cout << "KNN's RMSE: " << sqrt(knn_rmse / total) << endl;
}

RUN_OFF(_map) {
    string file = "../data/train.txt";
    auto loc_map = load_loc_map();
    unordered_map<int, unordered_map<int, list<rsrp_t>>> loc_pci_map;
    if (load_data(file, loc_pci_map)) {
        cout << "load data success" << endl;
    } else {
        cerr << "load data failed" << endl;
    }
    MaxAPosteri __map(loc_map, loc_pci_map);
    for (auto&& [loc, pci_rsrp_list] : loc_pci_map) {
        list<pair<int, rsrp_t>> rsrp_list;
        for (auto&& [pci, rsrps] : pci_rsrp_list) {
            for (auto&& rsrp : rsrps) {
                rsrp_list.emplace_back(pci, rsrp);
            }
        }
        auto loc_prob_map = __map(rsrp_list);
        LocationPtr max_loc = nullptr;
        Prob max_prob;
        for (auto&& [loc, prob] : loc_prob_map) {
            if (prob >= max_prob) {
                max_loc = loc;
                max_prob = prob;
            }
        }
        cout << "loc: " << loc << "\t" << max_loc->id << ";" << max_prob.prob << endl;
    }
}

RUN_OFF(interpolate2d) {
    // prepare data
    auto func = [](double x, double y) { return 100. * sin(x) * cos(y); };
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

RUN_OFF(interp_prob) {
    // prepare data
    unordered_map<Point, unordered_map<rsrp_t, Prob>> data;
    // (0, 0)
    auto & m1 = data[{0., 0.}];
    m1[-100] = 0.5;
    m1[-95] = 0.5;
    // (0, 1)
    auto & m2 = data[{0., 1.}];
    m2[-56] = 0.5;
    m2[-42] = 0.3;
    m2[-34] = 0.2;
    // (1, 0)
    auto & m3 = data[{1., 0.}];
    m3[-76] = 0.3;
    m3[-65] = 0.3;
    m3[-53] = 0.2;
    m3[-45] = 0.2;
    // (1, 1)
    auto & m4 = data[{1., 1.}];
    m4[-30] = 0.4;
    m4[-25] = 0.2;
    m4[-21] = 0.4;
    // (-1, 0)
    auto & m5 = data[{-1., 0.}];
    m5[-77] = 0.3;
    m5[-65] = 0.3;
    m5[-58] = 0.2;
    m5[-45] = 0.2;
    // (-1, 1)
    auto & m6 = data[{-1., 1.}];
    m6[-107] = 0.4;
    m6[-93] = 0.4;
    m6[-90] = 0.2;
    // (-1, -1)
    auto & m7 = data[{-1., -1.}];
    m7[-125] = 0.4;
    m7[-115] = 0.3;
    m7[-105] = 0.3;
    // (1, -1)
    auto & m8 = data[{1., -1.}];
    m8[-110] = 0.2;
    m8[-102] = 0.4;
    m8[-93] = 0.4;
    // (0, -1)
    auto & m9 = data[{0., -1.}];
    m9[-95] = 0.3;
    m9[-87] = 0.5;
    m9[-75] = 0.2;
    
    ProbInterp interp(data);
    
    unordered_map<Point, map<rsrp_t, Prob>> data_new;
    auto & t1 = data_new[{0.5, 0.5}];
    auto & t2 = data_new[{-0.5, 0.5}];
    auto & t3 = data_new[{0.5, -0.5}];
    auto & t4 = data_new[{-0.5, -0.5}];

    interp(data_new);
    
    cout << "-------------- After interpolation --------------" << endl;
    cout << "(0.5, 0.5): " << endl;
    double sum = 0;
    for (auto & [rsrp, prob] : t1) {
        double p = std::exp(prob.prob);
        cout << "\t" << rsrp << "\t" << p << endl;
        sum += p;
    }
    cout << "sum: " << sum << endl;
    cout << "(-0.5, 0.5): " << endl;
    sum = 0;
    for (auto & [rsrp, prob] : t2) {
        double p = std::exp(prob.prob);
        cout << "\t" << rsrp << "\t" << p << endl;
        sum += p;
    }
    cout << "sum: " << sum << endl;
    cout << "(0.5, -0.5): " << endl;
    sum = 0;
    for (auto & [rsrp, prob] : t3) {
        double p = std::exp(prob.prob);
        cout << "\t" << rsrp << "\t" << p << endl;
        sum += p;
    }
    cout << "sum: " << sum << endl;
    cout << "(-0.5, -0.5): " << endl;
    sum = 0;
    for (auto & [rsrp, prob] : t4) {
        double p = std::exp(prob.prob);
        cout << "\t" << rsrp << "\t" << p << endl;
        sum += p;
    }
    cout << "sum: " << sum << endl;
}

RUN_OFF(interpolation) {
    auto loc_map = load_loc_map();
    auto data_map = load_original_data("../data/train.txt");
    vector<int> pci_order = {117, 118, 314, 331, 997, 998};
    int pci = pci_order[0];
    std::unordered_map<Point, std::unordered_map<rsrp_t, Prob>> samples;
    vector<int> sm_set{1,2,3,4,7,8,9,10,13,14,15,16,19,20,21,22};
    for (auto loc : sm_set) {
        auto & rsrp_ls = data_map[loc][pci];
        unordered_map<rsrp_t, int> cnt_map;
        for (auto & rsrp : rsrp_ls) {
            cnt_map[rsrp]++;
        }
        unordered_map<rsrp_t, Prob> prob_map;
        prob_map.reserve(cnt_map.size());
        if (cnt_map.size() > 3) {
            vector<int> cnt_ls;
            cnt_ls.reserve(cnt_map.size());
            for (auto & [rsrp, cnt] : cnt_map) {
                cnt_ls.emplace_back(cnt);
            }
            sort(cnt_ls.begin(), cnt_ls.end(), greater<int>());
            while (cnt_ls.size() > 3) {
                cnt_ls.pop_back();
            }
            int cnt3 = cnt_ls[2];
            double S = accumulate(cnt_ls.begin(), cnt_ls.end(), 0);
            for (auto & [rsrp, cnt] : cnt_map) {
                if (cnt >= cnt3) {
                    prob_map.emplace(rsrp, Prob(cnt / S));
                }
            }
        } else {
            double S = accumulate(cnt_map.begin(), cnt_map.end(), 0, [](int a, auto & b) {
                return a + b.second;
            });
            for (auto & [rsrp, cnt] : cnt_map) {
                prob_map.emplace(rsrp, Prob(cnt / S));
            }
        }
        samples.emplace(loc_map.get_loc(loc)->point, std::move(prob_map));
    }
    ProbInterp interp(samples);
    std::unordered_map<Point, std::map<rsrp_t, Prob>> points_to_interp;
    points_to_interp.reserve(samples.size() * 2);
    double x0 = 0, x1 = 15, y0 = 0, y1 = 15;
    auto [x_step, y_step] = loc_map.step();
    for (double x = x0 + x_step / 2; x < x1; x += x_step) {
        for (double y = y0; y < y1; y += y_step) {
            points_to_interp.emplace(Point{x, y}, std::map<rsrp_t, Prob>{});
        }
    }
    for (double y = y0 + y_step / 2; y < y1; y += y_step) {
        for (double x = x0; x < x1; x += x_step) {
            points_to_interp.emplace(Point{x, y}, std::map<rsrp_t, Prob>{});
        }
    }
    for (double x = x0 + x_step / 2; x < x1; x += x_step) {
        for (double y = y0 + y_step / 2; y < y1; y += y_step) {
            points_to_interp.emplace(Point{x, y}, std::map<rsrp_t, Prob>{});
        }
    }
    interp(points_to_interp);
    for (auto & [point, interp_map] : points_to_interp) {
        cout << __color::bg_blu() << point << __color::bg_def() << endl;
        for (auto & [rsrp, prob] : interp_map) {
            cout << '\t' << rsrp << ": " << prob << endl;
        }
    }
   
}

RUN_OFF(boost_graph_test) {
    using namespace boost;
    using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        LocationPtr,
        boost::property<boost::edge_weight_t, double>>;
    std::size_t V = 5;
    graph_t g(V);

    g[0] = std::make_shared<Location>(1, Point{1,2});
    g[1] = std::make_shared<Location>(2, Point{3,4});
    g[2] = std::make_shared<Location>(3, Point{5,6});
    g[3] = std::make_shared<Location>(4, Point{7,8});
    g[4] = std::make_shared<Location>(5, Point{9,10});

    add_edge(0, 1, 1, g);
    add_edge(0, 2, 1.5, g);
    add_edge(0, 3, 2, g);
    add_edge(1, 4, 0.5, g);
    add_edge(2, 3, 2.5, g);
    add_edge(1, 3, 1.5, g);
    add_edge(1, 2, 2, g);
    add_edge(2, 4, 3, g);

    // std::vector<double> dist(V);

    // dijkstra_shortest_paths(g, 0, distance_map(dist.data()));

    // auto [begin, end] = vertices(g);
    // for (auto it = begin; it != end; ++it) {
    //     cout << *it << ": " << dist[*it] << endl;
    // }

    std::vector<std::vector<double>> dist_matrix(V, std::vector<double>(V, std::numeric_limits<double>::max()));
    johnson_all_pairs_shortest_paths(g, dist_matrix);

    for (size_t u = 0; u < V; ++u) {
        cout << g[u]->point << ":\n";
        for (size_t v = 0; v < V; ++v) {
            cout << '\t' << g[v]->point << ": " << dist_matrix[u][v] << '\n';
        }
        cout << endl;
    }
    
}
