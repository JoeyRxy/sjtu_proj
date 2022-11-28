#pragma once
#include <vector>
#include <unordered_map>
#include <list>
#include <configure.hpp>

std::unordered_map<int, std::vector<std::vector<double>>> analyze(std::unordered_map<int, std::list<std::vector<RSRP_TYPE>>> const &, int);
