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

Range::Range(const Group &variables) {
  data = std::make_unique<Data>(get_sizes(variables), );
}

Range::Range(const std::set<VariablePtr> &group) : combination(group.size()) {
  this->sizes.reserve(group.size());
  for (auto it = group.begin(); it != group.end(); ++it) {
    this->sizes.push_back((*it)->size());
  }
}

void Range::reset() {
  std::size_t *data = this->combination.data();
  for (std::size_t k = 0; k < this->sizes.size(); ++k) {
    data[k] = 0;
  }
  this->isAtEnd = false;
};

void Range::operator++() {
  std::size_t *data = this->combination.data();
  std::size_t k = this->combination.size() - 1;
  while (true) {
    ++data[k];
    if (data[k] == this->sizes[k]) {
      if (k == 0) {
        this->isAtEnd = true;
        break;
      } else {
        data[k] = 0;
        --k;
      }
    } else
      break;
  }
};
} // namespace EFG::categoric
