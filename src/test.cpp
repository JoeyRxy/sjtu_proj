#include <bits/stdc++.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

#include "cout_color.hpp"
#include "hmm/hmm.hpp"
#include "hmm/knn.hpp"
#include "registry.hpp"
#include "sjtu/loc_markov.hpp"
#include "sjtu/location_map.hpp"
#include "sjtu/max_a_posteri.hpp"
#include "sjtu/util.hpp"

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

inline void check_markov(MarkovPtr markov, std::list<LocationPtr> const& loc_ls) {
    for (auto it = loc_ls.begin(); it != loc_ls.end(); ++it) {
        cout << __color::bg_blu() << (*it)->point << __color::bg_def() << endl;
        for (auto jt = loc_ls.begin(); jt != loc_ls.end(); ++jt) {
            auto prob = *(*markov)(*it, *jt);
            if (prob > 0.01) cout << "\t-> " << (*jt)->point << ": " << prob << '\n';
        }
    }
}

RUN_OFF(procedure) {
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

    for (auto& [loc, _] : test_data_aligned) {
        cout << loc_map.get_loc(loc)->point << endl;
    }

    cout << " =================== " << endl;

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
        if (locptr->id == pred_locptr->id) {
            ++cnt;
            cout << __color::gre() << "\t" << pred_locs[t]->point << __color::def() << endl;
        } else {
            double dist = minkowski(locations[t]->point, pred_locs[t]->point);
            cout << "\t" << __color::gre() << locations[t]->point << __color::def() << " vs. "
                 << __color::red() << pred_locs[t]->point << __color::def() << ": " << dist << endl;
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

RUN_OFF(boost_graph_test) {
    using namespace boost;
    using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, LocationPtr,
                                          boost::property<boost::edge_weight_t, double>>;
    std::size_t V = 5;
    graph_t g(V);

    g[0] = std::make_shared<Location>(1, Point{1, 2});
    g[1] = std::make_shared<Location>(2, Point{3, 4});
    g[2] = std::make_shared<Location>(3, Point{5, 6});
    g[3] = std::make_shared<Location>(4, Point{7, 8});
    g[4] = std::make_shared<Location>(5, Point{9, 10});

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

    std::vector<std::vector<double>> dist_matrix(
        V, std::vector<double>(V, std::numeric_limits<double>::max()));
    johnson_all_pairs_shortest_paths(g, dist_matrix);

    for (size_t u = 0; u < V; ++u) {
        cout << g[u]->point << ":\n";
        for (size_t v = 0; v < V; ++v) {
            cout << '\t' << g[v]->point << ": " << dist_matrix[u][v] << '\n';
        }
        cout << endl;
    }
}

#include <OpenXLSX.hpp>

RUN(OpenXLSX) {
    using namespace OpenXLSX;
    XLDocument doc;
    doc.open("../data/db/F1.xlsx");

    auto book = doc.workbook();
    auto sheetname = book.worksheetNames().front();
    cout << "sheetname: " << sheetname << endl;
    auto sheet = book.worksheet(sheetname);
    auto m = sheet.rowCount();
    auto n = sheet.columnCount();
    cout << "column count: " << n << endl;
    cout << "row count: " << m << endl;

    std::vector<decltype(n)> pci_idx_list;
    pci_idx_list.reserve(10);
    for (decltype(n) i = 1; i <= n; ++i) {
        if (sheet.cell(1, i).value().get<std::string>().starts_with("NR_PCI")) {
            pci_idx_list.push_back(i);
        }
    }

    std::list<std::vector<std::pair<int, rsrp_t>>> data_list;

    for (decltype(m) i = 2; i <= m; ++i) {
        auto& back = data_list.emplace_back();
        back.reserve(pci_idx_list.size());
        for (auto j : pci_idx_list) {
            int pci;
            try {
            pci = sheet.cell(i, j).value().get<int>();
            } catch (XLException const&) {
                continue;
            }
            cout << "<" << pci << ", ";
            rsrp_t rsrp = -140;
            try {
                rsrp = sheet.cell(i, j + 1).value().get<rsrp_t>();
            } catch (XLException const&) {
            }
            back.emplace_back(pci, rsrp);
            cout << rsrp << ">, ";
        }
        cout << endl;
    }

    // for (decltype(m) i = 2; i <= m; ++i){
    //     cout << '[';
    //     for (decltype(n) j = 2; j <= n; ++j) {
    //         XLCellReference cell_ref(i, "NR_PCI");
    //         cout << sheet.cell(cell_ref).value().get<int>() << ", ";

    //     }
    // }
}
