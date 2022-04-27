/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "Printing.h"

#include <functional>

std::ostream &operator<<(std::ostream &s,
                         const EFG::categoric::VariablesSoup &group) {
  s << group.front()->name();
  for (std::size_t k = 1; k < group.size(); ++k) {
    s << ' ' << group[k]->name();
  }
  return s;
}

std::ostream &operator<<(std::ostream &s, const EFG::categoric::Group &group) {
  const auto &vars = group.getVariables();
  s << '{' << vars.front()->name();
  for (std::size_t k = 1; k < vars.size(); ++k) {
    s << ", " << vars[k]->name();
  }
  s << '}';
  return s;
}

namespace {
template <typename T>
void print_vector(
    std::ostream &s, const std::vector<T> &subject,
    const std::function<void(std::ostream &, const T &)> &pred =
        [](std::ostream &s, const T &element) { s << element; }) {
  pred(s, subject.front());
  for (std::size_t k = 1; k < subject.size(); ++k) {
    s << ' ';
    pred(s, subject[k]);
  }
}
} // namespace

void print_range(std::ostream &s, const EFG::categoric::Group &group) {
  s << group.getVariables() << std::endl;
  EFG::categoric::GroupRange range(group);
  EFG::categoric::for_each_combination(
      range, [&s](const EFG::categoric::Combination &comb) {
        print_vector(s, comb.data());
        s << std::endl;
      });
}

std::ostream &operator<<(std::ostream &s,
                         const EFG::distribution::Distribution &distribution) {
  s << distribution.getVariables().getVariables() << "    raw    image"
    << std::endl;
  EFG::categoric::GroupRange range(distribution.getVariables());
  EFG::categoric::for_each_combination(
      range, [&s, &distribution](const EFG::categoric::Combination &comb) {
        print_vector(s, comb.data());
        auto map_it = distribution.getCombinationsMap().find(comb);
        s << "  ->  ";
        if (map_it == distribution.getCombinationsMap().end()) {
          s << " null";
        } else {
          s << ' ' << map_it->second;
        }
        s << ' ' << distribution.evaluate(comb);
        s << std::endl;
      });
  return s;
}
