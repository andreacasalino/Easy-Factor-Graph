/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Combination.h>

#include <fstream>

namespace EFG::io {
std::ifstream make_stream(const std::string &file_name);

template <typename Predicate>
void for_each_line(std::ifstream &stream, const Predicate &pred) {
  std::string line;
  while (!stream.eof()) {
    line.clear();
    std::getline(stream, line);
    pred(line);
  }
}

categoric::Combination
parse_combination(const std::vector<std::string> &values);
} // namespace EFG::io