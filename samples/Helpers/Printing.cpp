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
                         const EFG::factor::Function &distribution) {
  TabularStream<4> table;
  std::stringstream group_stream;
  group_stream << distribution.vars().getVariables();
  table.addLine(
      Line{group_stream.str(), "", "raw image value   ", "image value"});
  EFG::categoric::GroupRange range(distribution.vars());

  std::vector<std::vector<std::size_t>> combinations;
  std::vector<float> images, transformations;
  distribution.forEachCombination<false>(
      [&combinations, &images](const auto &comb, float img) {
        combinations.push_back(comb);
        images.push_back(img);
      });
  distribution.forEachCombination<true>(
      [&transformations](const auto &, float img) {
        transformations.push_back(img);
      });

  for (std::size_t k = 0; k < combinations.size(); ++k) {
    Line to_add;
    std::stringstream comb_stream;
    print_vector(comb_stream, combinations[k]);
    to_add.push_back(comb_stream.str());
    to_add.push_back(" -> ");
    to_add.push_back(std::to_string(images[k]));
    to_add.push_back(std::to_string(transformations[k]));
    table.addLine(to_add);
  }
  table.print(s);
  return s;
}
