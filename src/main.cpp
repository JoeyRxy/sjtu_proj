#include <fstream>
#include <iomanip>
#include <iostream>

#include "hmm/hmm.hpp"
#include "hmm/knn.hpp"
#include "sjtu/loc_markov.hpp"
#include "sjtu/location_map.hpp"
#include "sjtu/max_a_posteri.hpp"
#include "sjtu/util.hpp"
#include "sjtu/barrier_loc.hpp"

void test_map();
void test_interpolate();
void test_eigen();
void test_interpolate2d();

using namespace std;
using namespace rxy;

constexpr int T = 21;

Point upward = {0, 1. / 3.};
Point downward = {0, -1. / 3.};
Point leftward = {-1. / 3., 0};
Point rightward = {1. / 3., 0};

auto load_original_data(string const& file) {
    unordered_map<int, unordered_map<int, list<rsrp_t>>> loc_pci_map;
    if (load_data(file, loc_pci_map)) {
        cout << "load data success" << endl;
    } else {
        cerr << "load data failed" << endl;
    }
    return loc_pci_map;
}

auto load_loc_map() {
    LocationMap loc_map(4, 5, {0, 0}, {4, 5});
    loc_map.add_loc(make_shared<BarrierLoc>(1, Point{0, 0}));
    loc_map.add_loc(make_shared<BarrierLoc>(2, Point{0, 1}));
    loc_map.add_loc(make_shared<BarrierLoc>(3, Point{0, 2}));
    loc_map.add_loc(make_shared<BarrierLoc>(4, Point{1, 2}));
    loc_map.add_loc(make_shared<BarrierLoc>(5, Point{1, 1}));
    loc_map.add_loc(make_shared<BarrierLoc>(6, Point{1, 0}));
    loc_map.add_loc(make_shared<BarrierLoc>(7, Point{2, 0}));
    loc_map.add_loc(make_shared<BarrierLoc>(8, Point{2, 1}));
    loc_map.add_loc(make_shared<BarrierLoc>(9, Point{2, 2}));
    loc_map.add_loc(make_shared<BarrierLoc>(10, Point{3, 2}));
    loc_map.add_loc(make_shared<BarrierLoc>(11, Point{3, 1}));
    loc_map.add_loc(make_shared<BarrierLoc>(12, Point{2, 3}));
    loc_map.add_loc(make_shared<BarrierLoc>(13, Point{2, 4}));
    loc_map.add_loc(make_shared<BarrierLoc>(14, Point{1, 4}));
    loc_map.add_loc(make_shared<BarrierLoc>(15, Point{1, 3}));
    return loc_map;
}

unordered_map<LocationPtr, int> get_number_for_loc(unordered_map<int, unordered_map<int, list<rsrp_t>>> const & loc_pci_map, LocationMap const & loc_map, int & S) {
    unordered_map<LocationPtr, int> cnt_map;
    S = 0;
    for (auto [loc, pci_rsrp_map] : loc_pci_map) {
        int& cnt = cnt_map[loc_map.get_loc(loc)];
        for (auto [_, rsrp_list] : pci_rsrp_map) {
            cnt += rsrp_list.size();
        }
        S += cnt;
    }
    return cnt_map;
}

auto get_markov(LocationMap const& loc_map) {
    vector<Sensation> sensations;
    sensations.reserve(T - 1);
    sensations.emplace_back(upward);// 1
    sensations.emplace_back(upward);// 1
    sensations.emplace_back(upward);// 1
    sensations.emplace_back(upward);// 2
    sensations.emplace_back(upward);// 2
    sensations.emplace_back(upward);// 2
    sensations.emplace_back(rightward);// 3
    sensations.emplace_back(rightward);// 3
    sensations.emplace_back(rightward);// 3
    sensations.emplace_back(rightward);// 4
    sensations.emplace_back(rightward);// 4
    sensations.emplace_back(rightward);// 4
    sensations.emplace_back(rightward);// 9
    sensations.emplace_back(rightward);// 9
    sensations.emplace_back(rightward);// 9
    sensations.emplace_back(downward);// 8
    sensations.emplace_back(downward);// 8
    sensations.emplace_back(downward);// 8
    sensations.emplace_back(rightward);// 11
    sensations.emplace_back(rightward);// 11
    // sensations.emplace_back(rightward);// 11
    vector<MarkovPtr> markovs;
    markovs.reserve(T - 1);
    for (auto&& sen : sensations) {
        markovs.emplace_back(make_shared<LocMarkov>(loc_map, sen));
    }
    return markovs;
}

/**
 * @return EmissionProb list
 * */
bool get_emission_prob_using_map(string const& file, LocationMap const& loc_map,
                                MaxAPosteri const& max_a_posteri,
                                vector<EmissionProb>& emission_probs,
                                vector<LocationPtr>& locations, int T = -1) {
    ifstream ifs(file);
    Parser parser(ifs);
    if (parser.parse()) {
        if (T != -1) {
            emission_probs.reserve(T);
            locations.reserve(T);
        }
        for (auto&& cell_info : parser.get()) {
            list<pair<int, rsrp_t>> pci_rsrp_list;
            for (auto [pci, info] : cell_info.pci_info_list) {
                pci_rsrp_list.emplace_back(pci, info->rsrp);
            }
            locations.emplace_back(loc_map.get_loc(cell_info.loc));
            emission_probs.emplace_back(max_a_posteri(pci_rsrp_list));
        }
    } else {
        cerr << "parse failed" << endl;
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}

int procedure() {
    string file = "../data/cellinfo-campus_040312.txt";
    auto loc_pci_map = load_original_data(file);
    // ---- test location map ----
    auto loc_map = load_loc_map();
    // ------ Max A Posteriori ------
    MaxAPosteri __map(loc_map, loc_pci_map);
    // ------ sensation & markov ------
    auto markovs = get_markov(loc_map);
    // ------ emission prob ------
    // ------ load data ------
    string test_file = "../data/path2_040311.txt";
    vector<EmissionProb> emission_probs;
    vector<LocationPtr> locations;
    if (get_emission_prob_using_map(test_file, loc_map, __map, emission_probs, locations)) {
        cout << "load test data success" << endl;
    } else {
        cerr << "load test data failed" << endl;
        return -1;
    }
    // ------ init prob -------
    const int T = emission_probs.size();
    unordered_map<LocationPtr, Prob> init_probs;
    int S = 0;
    for (auto [loc, num] : get_number_for_loc(loc_pci_map, loc_map, S)) {
        init_probs.emplace(loc, Prob(num / static_cast<Prob::value_type>(S)));
    }
    // ------ hmm ------
    auto pred_locs = HMM{loc_map.get_loc_set()}(markovs, init_probs, emission_probs);
    for (int t = 0; t < T; ++t) {
        cout << "t = " << t << endl;
        cout << "\t" << locations[t]->point << " vs. " << pred_locs[t]->point << endl;
    }
    return 0;
}

int main(int argc, char const* argv[]) {
    // test_eigen();
    // test_interpolate();
    test_interpolate2d();
    return 0;
}
