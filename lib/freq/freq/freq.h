#pragma once
#include <vector>
#include <unordered_map>
#include <list>
#include <configure.hpp>

namespace rxy {

std::vector<double> calc(std::list<std::vector<RSRP_TYPE>>::const_iterator begin, std::list<std::vector<RSRP_TYPE>>::const_iterator end, double lambda = 1.);
std::unordered_map<int, std::vector<std::vector<double>>> analyze(std::unordered_map<int, std::list<std::vector<RSRP_TYPE>>> const &, int);

}

