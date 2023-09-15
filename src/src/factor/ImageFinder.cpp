/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/ImageFinder.h>

#include <optional>

namespace EFG::factor {
namespace {
std::optional<std::size_t>
find_var(const categoric::VariablesSoup &bigger_group,
         const categoric::VariablePtr &var) {
  for (std::size_t k = 0; k < bigger_group.size(); ++k) {
    if (bigger_group[k].get() == var.get()) {
      return k;
    }
  }
  return std::nullopt;
}

std::vector<std::size_t>
get_indices(const categoric::VariablesSoup &distribution_group,
            const categoric::VariablesSoup &bigger_group) {
  if (distribution_group.size() > bigger_group.size()) {
    throw Error{"Invalid bigger_group to build a CombinationFinder"};
  }
  std::vector<std::size_t> result;
  result.reserve(distribution_group.size());
  for (const auto &distribution_var : distribution_group) {
    auto index_in_bigger = find_var(bigger_group, distribution_var);
    if (index_in_bigger.has_value()) {
      result.push_back(index_in_bigger.value());
    } else {
      throw Error::make(distribution_var->name(),
                        " was not found in the bigger group");
    }
  }
  return result;
}
} // namespace

ImageFinder::ImageFinder(std::shared_ptr<const Function> function,
                         const categoric::VariablesSoup &bigger_group)
    : function_{function},
      indices_in_bigger_group(
          get_indices(function_->vars().getVariables(), bigger_group)) {}

std::vector<std::size_t> ImageFinder::extractSmallerCombination(
    const std::vector<std::size_t> &comb) const {
  std::vector<std::size_t> res;
  res.reserve(indices_in_bigger_group.size());
  for (auto index : indices_in_bigger_group) {
    res.push_back(comb[index]);
  }
  return res;
}
} // namespace EFG::factor
