/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <algorithm>

namespace EFG::distribution {
namespace {
std::vector<std::size_t>
get_indices(const categoric::VariablesSoup &distribution_group,
            const categoric::VariablesSoup &bigger_group) {
  if (distribution_group.size() >= bigger_group.size()) {
    throw Error{"Invalid group to build a CombinationFinder"};
  }
  std::vector<std::size_t> result;
  result.reserve(distribution_group.size());
  for (const auto &distribution_var : distribution_group) {
    auto bigger_group_it =
        std::find_if(bigger_group.begin(), bigger_group.end(),
                     [&distribution_var](const categoric::VariablePtr &var) {
                       return (var.get() == distribution_var.get());
                     });
    if (bigger_group_it == bigger_group.end()) {
      throw Error{distribution_var->name(),
                  " was not found in the bigger group"};
    }
    result.push_back(std::distance(bigger_group.begin(), bigger_group_it));
  }
  return result;
}
} // namespace

CombinationFinder::CombinationFinder(
    const EvaluatorPtr &evaluator,
    const CombinationRawValuesMapPtr &combinations_map,
    const categoric::VariablesSoup &distribution_group,
    const categoric::VariablesSoup &bigger_group)
    : indices_in_bigger_group(get_indices(distribution_group, bigger_group)) {
  this->evaluator = evaluator;
  this->combinations_map = combinations_map;
}

CombinationFinder::Result
CombinationFinder::find(const categoric::Combination &comb) const {
  if (comb.data().size() != indices_in_bigger_group.size()) {
    throw Error{"Invalid to combination"};
  }
  auto it = std::find_if(
      combinations_map->begin(), combinations_map->end(),
      [&comb, &indices = this->indices_in_bigger_group](
          const std::pair<categoric::Combination, float> &element) {
        for (std::size_t k = 0; k < indices.size(); ++k) {
          if (element.first.data()[k] != comb.data()[indices[k]]) {
            return false;
          }
        }
        return true;
      });
  if (it == combinations_map->end()) {
    return CombinationFinder::Result{it, evaluator->evaluate(0)};
  }
  return CombinationFinder::Result{it, evaluator->evaluate(it->second)};
}
} // namespace EFG::distribution
