/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/ModelSeed.h>

#include <ranges>
#include <unordered_set>

namespace EFG::structure {
ModelSeed ModelBuilder::build(ModelBuilder &&builder) {
  auto get_neigh_ = [&](std::size_t from) {
    return builder.model_.binary_factors |
           std::views::filter([=](const auto &el) {
             return el.first.first == from || el.first.second == from;
           }) |
           std::views::transform([=](const auto &el) {
             if (el.first.first == from) {
               return el.first.second;
             } else {
               return el.first.first;
             }
           });
  };

  // BFS to check the graph is well connected (form a single island)
  std::unordered_set<std::size_t> visited;
  std::vector<std::size_t> level{0}, swap;
  while (!level.empty()) {
    swap.clear();
    for (auto prev : level) {
      visited.emplace(prev);
      auto rng = get_neigh_(prev) | std::views::filter([&](auto index) {
                   return !visited.contains(index);
                 });
      for (auto next : rng) {
        swap.push_back(next);
      }
    }
    std::swap(level, swap);
  }

  if (visited.size() != builder.model_.variables_sizes.size()) {
    throw Error{"Attempting to create a model with disconneted islands ... "
                "this is actually forbidden, just create multiple models!"};
  }

  return std::move(builder.model_);
}

std::size_t ModelBuilder::make_variable(categoric::VarStateSize size,
                                        VariableDescription desc) {
  if (size <= 1) {
    throw Error{"Variables cannot have null or a one size"};
  }
  auto res = model_.variables_sizes.size();
  model_.variables_sizes.push_back(size);
  if (!desc.label.empty()) {
    model_.named.emplace(std::move(desc.label), res);
  }
  if (desc.isPermanentEvidence) {
    model_.permanent_evidences.push_back(res);
  }
  return res;
}

std::size_t ModelBuilder::make_variable(categoric::VarStateSize variable_size) {
  return make_variable(variable_size, VariableDescription{});
}

void ModelBuilder::validateIndex(std::size_t var_index) {
  if (model_.variables_sizes.size() <= var_index) {
    throw Error("No variable with index: {} did you forget to "
                "create the variable before adding the factor?",
                var_index);
  }
}

void ModelBuilder::add_tunable_unary_factor(
    factor::UnaryFactorExponential &&to_add, std::size_t var_index) {
  add_unary_factor(std::forward<factor::UnaryFactorExponential>(to_add),
                   var_index);

  model_.tunability.order.emplace_back(
      Tunability::TunableUnaryFactor{var_index});
}

void ModelBuilder::add_tunable_binary_factor(
    factor::BinaryFactorExponential &&to_add, std::size_t var_a_index,
    std::size_t var_b_index) {
  auto factor_index = model_.binary_factors.size();

  add_binary_factor(std::forward<factor::BinaryFactorExponential>(to_add),
                    var_a_index, var_b_index);

  model_.tunability.order.emplace_back(
      Tunability::TunableBinaryFactor{var_a_index, var_b_index});
}
} // namespace EFG::structure
