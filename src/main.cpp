#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "hmm/hmm.hpp"
#include "hmm/knn.hpp"
#include "sjtu/barrier_loc.hpp"
#include "sjtu/loc_markov.hpp"
#include "sjtu/location_map.hpp"
#include "sjtu/max_a_posteri.hpp"
#include "sjtu/util.hpp"

void test_map();
void test_interpolate();
void test_eigen();
void test_interpolate2d();

using namespace std;
using namespace rxy;

namespace __color {
enum Code {
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_YELLOW = 43,
    BG_BLUE = 44,
    BG_DEFAULT = 49
};
class Modifier {
    Code code;

   public:
    constexpr Modifier(Code pCode) : code(pCode) {}
    friend std::ostream& operator<<(std::ostream& os, const Modifier& mod) {
        return os << "\033[" << mod.code << "m";
    }
};
constexpr Modifier gre() { return Modifier(FG_GREEN); }
constexpr Modifier red() { return Modifier(FG_RED); }
constexpr Modifier yel() { return Modifier(FG_YELLOW); }
constexpr Modifier blu() { return Modifier(FG_BLUE); }
constexpr Modifier def() { return Modifier(FG_DEFAULT); }
constexpr Modifier bg_red() { return Modifier(BG_RED); }
constexpr Modifier bg_gre() { return Modifier(BG_GREEN); }
constexpr Modifier bg_yel() { return Modifier(BG_YELLOW); }
constexpr Modifier bg_blu() { return Modifier(BG_BLUE); }
constexpr Modifier bg_def() { return Modifier(BG_DEFAULT); }
}  // namespace __color

Point upward = {0, 2};
Point downward = {0, -2};
Point leftward = {-2, 0};
Point rightward = {2, 0};
Point stop = {0, 0};

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
    LocationMap loc_map(4, 6, {-2.5, -2.5}, {17.5, 27.5});
    int step = 5;
    int id = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++, ++id) {
            loc_map.add_loc(make_shared<BarrierLoc>(id, Point{i * step, j * step}));
        }
    }
    return loc_map;
}

unordered_map<LocationPtr, int> get_number_for_loc(
    unordered_map<int, unordered_map<int, list<rsrp_t>>> const& loc_pci_map,
    LocationMap const& loc_map, int& S) {
    unordered_map<LocationPtr, int> cnt_map;
    S = 0;
    for (auto&& [loc, pci_rsrp_map] : loc_pci_map) {
        int& cnt = cnt_map[loc_map.get_loc(loc)];
        for (auto&& [_, rsrp_list] : pci_rsrp_map) {
            cnt += rsrp_list.size();
        }
        S += cnt;
    }
    return cnt_map;
}

auto get_markov(LocationMap const& loc_map) {
    vector<Sensation> sensations;
    sensations.emplace_back(upward);     // 8
    sensations.emplace_back(upward);     // 8
    sensations.emplace_back(upward);     // 8
    sensations.emplace_back(upward);     // 9
    sensations.emplace_back(upward);     // 9
    sensations.emplace_back(upward);     // 9
    sensations.emplace_back(upward);     // 10
    sensations.emplace_back(upward);     // 10
    sensations.emplace_back(upward);     // 11
    sensations.emplace_back(upward);     // 11
    sensations.emplace_back(upward);     // 12
    sensations.emplace_back(upward);     // 12
    sensations.emplace_back(rightward);  // 12
    sensations.emplace_back(rightward);  // 18
    sensations.emplace_back(rightward);  // 18
    sensations.emplace_back(rightward);  // 18
    sensations.emplace_back(rightward);  // 24
    sensations.emplace_back(rightward);  // 24
    sensations.emplace_back(downward);   // 24
    sensations.emplace_back(downward);   // 23
    sensations.emplace_back(downward);   // 23
    sensations.emplace_back(downward);   // 23
    sensations.emplace_back(downward);   // 22
    sensations.emplace_back(downward);   // 22
    sensations.emplace_back(downward);   // 21
    sensations.emplace_back(downward);   // 21
    sensations.emplace_back(downward);   // 20
    sensations.emplace_back(downward);   // 20
    // sensations.emplace_back(stop);      // 20
    vector<MarkovPtr> markovs;
    markovs.reserve(sensations.size());
    for (auto&& sen : sensations) {
        markovs.emplace_back(make_shared<LocMarkov>(loc_map, sen));
    }
    return markovs;
}

KNN<rsrp_t> get_knn(string const& file, vector<int> const& pci_order) {
    std::list<std::pair<int, std::vector<rsrp_t>>> loc_data_aligned;
    KNN<rsrp_t> knn(40, KNN<rsrp_t>::distance_inv_weighted_euc);
    if (load_data_aligned(file, loc_data_aligned, pci_order)) {
        cout << "load data success" << endl;
        auto data = get_train_test_data(loc_data_aligned, 1.);
        knn.train(move(get<0>(data)), move(get<1>(data)));
        return knn;
    } else {
        throw runtime_error("load data failed");
    }
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
            for (auto&& [pci, info] : cell_info.pci_info_list) {
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

void get_emission_prob_using_knn(
    std::list<std::pair<int, std::vector<rsrp_t>>> const& test_data_aligned, KNN<rsrp_t> const& knn,
    LocationMap const& loc_map, vector<EmissionProb>& emission_probs,
    vector<LocationPtr>& locations, int T = -1) {
    if (T != -1) {
        emission_probs.reserve(T);
        locations.reserve(T);
    }
    for (auto&& [loc, rsrp_aligned] : test_data_aligned) {
        locations.emplace_back(loc_map.get_loc(loc));
        vector<double> label_prob = knn.predict_prob(rsrp_aligned);
        unordered_map<LocationPtr, Prob> prob_map;
        for (auto&& _loc : loc_map.get_loc_set()) {
            prob_map[_loc] = label_prob[_loc->id];
        }
        emission_probs.emplace_back(std::move(prob_map));
    }
}

int test_knn() {
    cout << __color::bg_blu() << "--- knn ---" << __color::bg_def() << endl;
    // string file = "../data/cellinfo-campus_040312.txt";
    string file = "../data/train.txt";
    // string file = "../data/cellinfo-real.txt";
    std::list<std::pair<int, std::vector<rsrp_t>>> loc_data_aligned;
    // vector<int> pci_order = {908, 923, 936, 663, 100, 906, 457, 665, 223, 922, 748, 238};
    vector<int> pci_order = {117, 118, 314, 331, 997, 998};
    // vector<int> pci_order = {902, 324, 933, 934, 63, 807, 326, 143, 92, 365, 226, 662, 31, 942,
    // 138, 140, 901, 325, 948, 945, 946, 364};
    sort(pci_order.begin(), pci_order.end());
    if (load_data_aligned(file, loc_data_aligned, pci_order)) {
        cout << "load data success" << endl;
    } else {
        cout << "load data failed" << endl;
        return -1;
    }
    auto train_data = get_train_test_data(move(loc_data_aligned), 0.8);
    auto& X_train = get<0>(train_data);
    auto& y_train = get<1>(train_data);
    auto& X_test = get<2>(train_data);
    auto& y_test = get<3>(train_data);
    cout << "train data size: " << X_train.size() << endl;
    cout << "test data size: " << X_test.size() << endl;
    string test_file = "../data/test.txt";
    std::list<std::pair<int, std::vector<rsrp_t>>> test_data_aligned;
    if (load_data_aligned(test_file, test_data_aligned, pci_order)) {
        cout << "load test data success" << endl;
    } else {
        cout << "load test data failed" << endl;
        return -1;
    }
    KNN<rsrp_t> knn(40, KNN<rsrp_t>::distance_inv_weighted_euc);
    knn.train(X_train, y_train);
    int cnt = 0;
    auto loc_map = load_loc_map();
    auto tik = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < X_test.size(); ++i) {
        if (knn.predict(X_test[i]) == y_test[i]) {
            cnt++;
        }
    }
    auto tok = std::chrono::high_resolution_clock::now();
    cout << "accuracy: " << cnt * 1.0 / X_test.size() << "\n"
         << __color::bg_blu()
         << "duration: " << std::chrono::duration_cast<chrono::milliseconds>(tok - tik).count()
         << " ms" << __color::bg_def() << endl;
    int total = 0;
    cnt = 0;
    for (auto&& [loc, data] : test_data_aligned) {
        total++;
        cout << "t = " << total << endl;
        int pred = knn.predict(data);
        if (pred == loc) {
            cout << __color::gre() << "\t" << loc_map.get_loc(loc)->point << __color::def() << endl;
            ++cnt;
        } else {
            cout << "\t" << __color::gre() << loc_map.get_loc(loc)->point << __color::def()
                 << " vs. " << __color::red() << loc_map.get_loc(pred)->point << __color::def()
                 << ": " << minkowski(loc_map.get_loc(loc)->point, loc_map.get_loc(pred)->point) << endl;
        }
    }
    cout << "accuracy: " << static_cast<double>(cnt) / total << endl;
    return 0;
}

int procedure() {
    cout << __color::bg_blu() << "--- procedure ---" << __color::bg_def() << endl;
    vector<int> pci_order = {117, 118, 314, 331, 997, 998};
    string file = "../data/train.txt";
    // ---- location map ----
    auto loc_map = load_loc_map();
    // ------ sensation & markov ------
    auto markovs = get_markov(loc_map);
    int T = markovs.size() + 1;
    auto knn = get_knn(file, pci_order);
    // ------ load data ------
    string test_file = "../data/test.txt";
    vector<EmissionProb> emission_probs;
    vector<LocationPtr> locations;
    std::list<std::pair<int, std::vector<rsrp_t>>> test_data_aligned;
    load_data_aligned(test_file, test_data_aligned, pci_order);
    get_emission_prob_using_knn(test_data_aligned, knn, loc_map, emission_probs, locations, T);
    // ------ init prob -------
    unordered_map<LocationPtr, Prob> init_probs;
    for (auto&& loc : loc_map.get_loc_set()) {
        init_probs[loc] = emission_probs[0][loc];
        // init_probs[loc] = {0, true};
    }
    // ------ hmm ------
    auto pred_locs = HMM{loc_map.get_loc_set()}(markovs, init_probs, emission_probs);
    int cnt = 0;
    for (int t = 0; t < T; ++t) {
        cout << "t = " << (t + 1) << endl;
        if (locations[t]->id == pred_locs[t]->id) {
            ++cnt;
            cout << __color::gre() << "\t" << pred_locs[t]->point << __color::def() << endl;
        } else {
            cout << "\t" << __color::gre() << locations[t]->point << __color::def() << " vs. "
                 << __color::red() << pred_locs[t]->point << __color::def() << ": "
                 << minkowski(locations[t]->point, pred_locs[t]->point) << endl;
        }
    }
    cout << "accuracy = " << (double)cnt / T << endl;
    return 0;
}

int main(int argc, char const* argv[]) {
    // test_eigen();
    // test_interpolate();
    // test_interpolate2d();
    // auto loc_map = load_loc_map();
    // for (int j = 5; j >= 0; --j) {
    //     for (int i = 0; i < 4; ++i) {
    //         cout << __color::bg_blu() << loc_map.get_loc(i, j)->point << ' ' << __color::bg_def();
    //     }
    //     cout << endl;
    // }
    test_knn();
    // test_map();
    procedure();
    return 0;
}
