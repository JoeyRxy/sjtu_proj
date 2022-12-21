#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "registry.hpp"

#include "hmm/hmm.hpp"
#include "hmm/knn.hpp"
#include "sjtu/loc_markov.hpp"
#include "sjtu/location_map.hpp"
#include "sjtu/max_a_posteri.hpp"
#include "sjtu/util.hpp"
#include "sjtu/ext_loc.hpp"

#include "cout_color.hpp"

void test_map();
void test_interpolate();
void test_eigen();
void test_interpolate2d();

void test_interp_prob();

using namespace std;
using namespace rxy;

Point upward = {0, 2};
Point downward = {0, -2};
Point leftward = {-2, 0};
Point rightward = {2, 0};
Point stop = {0, 0};

unordered_map<int, unordered_map<int, list<RSRP_TYPE>>> load_original_data(string const& file) {
    unordered_map<int, unordered_map<int, list<RSRP_TYPE>>> loc_pci_map;
    if (load_data(file, loc_pci_map)) {
        cout << "load data success" << endl;
    } else {
        cerr << "load data failed" << endl;
    }
    return loc_pci_map;
}

LocationMap load_loc_map() {
    int m = 10, n = 15;
    LocationMap loc_map(m, n, {0, 0}, {20, 30});
    // auto [x_step, y_step] = loc_map.step();
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            loc_map.add_loc(i, j);
        }
    }
    // set obstacles
    // loc_map.remove_ext_rect({4, 12}, {16, 28});
    // loc_map.remove_ext_rect({4,4},{16,8});
    return loc_map;
}

/**
 * @return EmissionProb list
 * */
inline bool get_emission_prob_using_map(std::string const& file, LocationMap const& loc_map,
                                 MaxAPosteri const& max_a_posteri,
                                 std::vector<EmissionProb>& emission_probs,
                                 std::vector<LocationPtr>& locations, int T = -1) {
    std::ifstream ifs(file);
    Parser parser(ifs);
    if (parser.parse()) {
        if (T != -1) {
            emission_probs.reserve(T);
            locations.reserve(T);
        }
        for (auto&& cell_info : parser.get()) {
            std::list<std::pair<int, RSRP_TYPE>> pci_rsrp_list;
            for (auto&& [pci, info] : cell_info.pci_info_list) {
                pci_rsrp_list.emplace_back(pci, info->rsrp);
            }
            locations.emplace_back(loc_map.get_loc(cell_info.loc));
            emission_probs.emplace_back(max_a_posteri(pci_rsrp_list));
        }
    } else {
        std::cerr << "parse failed" << std::endl;
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}

int main(/* int argc, char const* argv[] **/) {
    RUN_ALL;
    return 0;
}
