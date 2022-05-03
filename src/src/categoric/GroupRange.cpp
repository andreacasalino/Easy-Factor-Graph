/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>

namespace EFG::categoric {
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

GroupRange::GroupRange(const Group &variables) {
  data = std::make_unique<Data>(Data{get_sizes(variables), nullptr, false});
  data->combination =
      std::make_unique<Combination>(variables.getVariables().size());
}

GroupRange::GroupRange(const GroupRange &o) {
  if (nullptr != o.data) {
    data = std::make_unique<Data>(
        Data{o.data->sizes, nullptr, o.data->end_of_range});
    data->combination = std::make_unique<Combination>(*o.data->combination);
  }
}

GroupRange &GroupRange::operator++() {
  if (nullptr == data) {
    throw Error{"GroupRange not incrementable"};
  }
  auto new_comb = this->data->combination->data();
  std::size_t k = new_comb.size() - 1;
  while (true) {
    ++new_comb[k];
    if (new_comb[k] == data->sizes[k]) {
      if (k == 0) {
        this->data->end_of_range = true;
        data.reset();
        return *this;
      } else {
        new_comb[k] = 0;
        --k;
      }
    } else
      break;
  }
  data->combination = std::make_unique<Combination>(std::move(new_comb));
  return *this;
}

bool operator==(const GroupRange &a, const GroupRange &b) {
  return a.isEqual(b);
}
bool operator!=(const GroupRange &a, const GroupRange &b) {
  return !a.isEqual(b);
}
} // namespace EFG::categoric
