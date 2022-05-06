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

std::ostream &operator<<(std::ostream &s, const std::vector<float> &values) {
  print_vector(s, values);
  return s;
}

std::ostream &operator<<(std::ostream &s,
                         const std::vector<std::size_t> &values) {
  print_vector(s, values);
  return s;
}

std::ostream &operator<<(std::ostream &s,
                         const EFG::categoric::Combination &comb) {
  print_vector(s, comb.data());
  return s;
}

namespace {
using Line = std::vector<std::string>;

template <std::size_t TableSize> class TabularStream {
public:
  TabularStream() {
    for (std::size_t k = 0; k < TableSize; ++k) {
      columns_sizes.push_back(0);
    }
  }

  void addLine(const Line &line) {
    if (line.size() != TableSize) {
      throw std::runtime_error{"Invalid line to print"};
    }
    lines.push_back(line);
    for (std::size_t col = 0; col < TableSize; ++col) {
      if (columns_sizes[col] < line[col].size()) {
        columns_sizes[col] = line[col].size();
      }
    }
  }

  void print(std::ostream &s) const {
    for (const auto &line : lines) {
      s << std::endl;
      for (std::size_t col = 0; col < TableSize; ++col) {
        std::string to_add = line[col];
        to_add.reserve(columns_sizes[col]);
        while (to_add.size() != columns_sizes[col]) {
          to_add.push_back(' ');
        }
        s << ' ' << to_add;
      }
    }
  }

private:
  std::vector<std::size_t> columns_sizes;
  std::vector<Line> lines;
};
} // namespace

#include <sstream>

std::ostream &operator<<(std::ostream &s,
                         const EFG::distribution::Distribution &distribution) {
  TabularStream<4> table;
  std::stringstream group_stream;
  group_stream << distribution.getGroup().getVariables();
  table.addLine(
      Line{group_stream.str(), "", "raw image value   ", "image value"});
  EFG::categoric::GroupRange range(distribution.getGroup());
  EFG::categoric::for_each_combination(
      range, [&table, &distribution](const EFG::categoric::Combination &comb) {
        Line to_add;
        std::stringstream comb_stream;
        print_vector(comb_stream, comb.data());
        to_add.push_back(comb_stream.str());
        to_add.push_back(" -> ");
        auto map_it = distribution.getCombinationsMap().find(comb);
        if (map_it == distribution.getCombinationsMap().end()) {
          to_add.push_back("null");
        } else {
          to_add.push_back(std::to_string(map_it->second));
        }
        to_add.push_back(std::to_string(distribution.evaluate(comb)));
        table.addLine(to_add);
      });
  table.print(s);
  return s;
}
