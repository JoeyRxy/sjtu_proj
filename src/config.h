#pragma once
#include <boost/json.hpp>
#include <configure.hpp>
#include <exception>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace rxy {

class Config {
  friend Config const &GetConfig();

private:
  Config() {
    using namespace boost;
    std::ifstream in;
    in.open(CONF_PATH);
    if (!in)
      throw std::exception("can not open conf file");
    std::string jstr((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
    in.close();
    json::error_code ec;
    json::value jv = json::parse(jstr, ec);
    if (ec)
      throw std::exception("parse failed");
    if (!jv.is_object())
      throw std::exception("conf.json error format");
    json::object obj = jv.as_object();
    pci_order = json::value_to<std::vector<int>>(obj.at("pciOrder"));
    try {
      map_size = obj.at("mapSize").as_int64();
    } catch (std::out_of_range &) {
    }
    try {
      minkowski = obj.at("minkowski").as_int64();
    } catch (std::out_of_range &) {
    }
    try {
      enumer_limit = obj.at("enumerLimie").as_int64();
    } catch (std::out_of_range &) {
    }
    try {
      ext_rate = obj.at("extRate").as_int64();
    } catch (std::out_of_range &) {
    }
    try {
      ecc = obj.at("ecc").as_double();
    } catch (std::out_of_range &) {
    }
  }

public:
  std::vector<int> pci_order;
  int map_size = 100;
  int minkowski = 2;
  int enumer_limit = 1000000000;
  int ext_rate = 5;
  double ecc = 0.1;
};

inline Config const &GetConfig() {
  static Config conf;
  return conf;
}

} // namespace rxy
