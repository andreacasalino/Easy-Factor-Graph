/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>

namespace EFG::categoric {
GroupRange::Data::Data(const std::vector<size_t> &s, bool eor)
    : sizes{s}, end_of_range{eor} {
  combination.resize(sizes.size());
  for (auto &v : combination) {
    v = 0;
  }
}

namespace {
std::vector<size_t> get_sizes(const Group &variables) {
  std::vector<size_t> result;
  result.reserve(variables.size());
  for (const auto &var : variables.getVariables()) {
    result.push_back(var->size());
  }
  return result;
}
} // namespace

GroupRange::GroupRange(const std::vector<std::size_t> &sizes) {
  data.emplace(sizes, false);
}

GroupRange::GroupRange(const Group &variables)
    : GroupRange{get_sizes(variables)} {}

GroupRange::GroupRange(const GroupRange &o) {
  if (o.data) {
    data.emplace(o.data->sizes, o.data->end_of_range);
    data->combination = o.data->combination;
  }
}

GroupRange &GroupRange::operator++() {
  if (!data.has_value()) {
    throw Error{"GroupRange not incrementable"};
  }
  auto &comb = data->combination;
  std::size_t k = comb.size() - 1;
  while (true) {
    if (++comb[k] == data->sizes[k]) {
      if (k == 0) {
        this->data->end_of_range = true;
        data.reset();
        return *this;
      } else {
        comb[k] = 0;
        --k;
      }
    } else
      break;
  }
  return *this;
}

bool operator==(const GroupRange &a, const GroupRange &b) {
  return a.isEqual(b);
}
bool operator!=(const GroupRange &a, const GroupRange &b) {
  return !a.isEqual(b);
}
} // namespace EFG::categoric
