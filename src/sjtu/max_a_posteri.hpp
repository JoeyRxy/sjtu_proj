#pragma once
#include "hmm/probability.hpp"
#include "location_map.hpp"
#include "util.hpp"

namespace rxy {

class MaxAPosteri {
   private:
    // statistic data for each location's pci
    // Location -> (pci -> statistic data)
    std::unordered_map<LocationPtr, std::unordered_map<int, std::pair<double, double>>>
        loc_pci_stats;

   public:
    MaxAPosteri(
        LocationMap const& loc_map,
        std::unordered_map<int, std::unordered_map<int, std::list<rsrp_t>>> const& loc_pci_map) {
        for (auto&& [loc, pci_rsrp_map] : loc_pci_map) {
            auto& pci_stats = loc_pci_stats[loc_map.get_loc(loc)];
            for (auto&& [pci, rsrp_list] : pci_rsrp_map) {
                pci_stats[pci] = get_mean_var(rsrp_list.begin(), rsrp_list.end(), rsrp_list.size());
            }
        }
    }

    /**
     *
     * */
    std::unordered_map<LocationPtr, Prob> operator()(
        std::list<std::pair<int, rsrp_t>> const& X) const {
        std::unordered_map<LocationPtr, Prob> loc_prob;
        for (auto const& [loc, pci_rsrp_map] : loc_pci_stats) {
            Prob p(0, true);
            for (auto&& [pci, rsrp] : X) {
                try {
                    auto [mu, sigma] = pci_rsrp_map.at(pci);
                    p *= log_nd_pdf(rsrp, mu, sigma);
                } catch (std::out_of_range&) {
                    // pci not found
                }
            }
            loc_prob[loc] = p;
        }
        return loc_prob;
    }
};

}  // namespace rxy
