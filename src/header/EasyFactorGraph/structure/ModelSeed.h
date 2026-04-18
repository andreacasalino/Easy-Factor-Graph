/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/structure/Definitions.h>

#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace EFG::structure {
using NamedVariables = std::unordered_map<std::string, std::size_t>;

struct PairHasher {
  PairHasher() = default;

  std::size_t operator()(const std::pair<std::size_t, std::size_t> &p) const {
    static std::hash<std::size_t> hasher;
    std::size_t res = hasher(p.first);
    res ^= hasher(p.second) + 0x9e3779b9 + (res << 6) + (res >> 2);
    return res;
  }
};

struct Tunability {
  struct TunableUnaryFactor {
    std::size_t var_index;
  };
  struct TunableBinaryFactor {
    std::size_t first_var_index;
    std::size_t second_var_index;
  };

  template <typename T> auto makeFilteredRange() const {
    return order | std::views::filter([](const auto &el) {
             return std::get_if<T>(&el);
           }) |
           std::views::transform(
               [](const auto &el) { return std::get<T>(el); });
  }

  std::vector<std::variant<TunableUnaryFactor, TunableBinaryFactor>> order;
};

struct ModelSeed {
  std::vector<categoric::VarStateSize> variables_sizes;

  std::unordered_map<std::size_t, FactorOrFactorExponential<1>> unary_factors;

  std::unordered_map<std::pair<std::size_t, std::size_t>,
                     FactorOrFactorExponential<2>, PairHasher>
      binary_factors;

  NamedVariables named;
  std::vector<std::size_t> permanent_evidences;
  Tunability tunability;
};

class ModelBuilder {
public:
  ModelBuilder() = default;

  ModelBuilder(const ModelBuilder &) = delete;
  ModelBuilder &operator=(const ModelBuilder &) = delete;

  static ModelSeed build(ModelBuilder &&builder);

  // returns the index of the generated variable
  std::size_t make_variable(categoric::VarStateSize variable_size);

  struct VariableDescription {
    bool isPermanentEvidence{false};
    std::string label{""};
  };
  // returns the index of the generated variable
  std::size_t make_variable(categoric::VarStateSize variable_size,
                            VariableDescription description);

  const auto &get_seed() const { return model_; }

  template <typename Trasform>
  void add_unary_factor(factor::FactorT<1, Trasform> &&to_add,
                        std::size_t var_index);

  void add_tunable_unary_factor(factor::UnaryFactorExponential &&to_add,
                                std::size_t var_index);

  template <typename Trasform>
  void add_binary_factor(factor::FactorT<2, Trasform> &&to_add,
                         std::size_t var_a_index, std::size_t var_b_index);

  void add_tunable_binary_factor(factor::BinaryFactorExponential &&to_add,
                                 std::size_t var_a_index,
                                 std::size_t var_b_index);

private:
  void validateIndex(std::size_t var_index);

  ModelSeed model_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Trasform>
void ModelBuilder::add_unary_factor(factor::FactorT<1, Trasform> &&to_add,
                                    std::size_t var_index) {
  validateIndex(var_index);

  bool inserted =
      model_.unary_factors
          .emplace(var_index,
                   std::forward<factor::FactorT<1, Trasform>>(to_add))
          .second;

  if (!inserted) {
    throw Error{"Multiple unary factor for variable with index {}", var_index};
  }
}

template <typename Trasform>
void ModelBuilder::add_binary_factor(factor::FactorT<2, Trasform> &&to_add,
                                     std::size_t var_a_index,
                                     std::size_t var_b_index) {
  if (var_a_index == var_b_index) {
    throw Error{"Binary factor cannot connect the same variable!"};
  }

  auto validate_ = [&](std::size_t var_index, std::size_t group_index) {
    validateIndex(var_index);
    auto expected_size = to_add.getVariables().sizes()[group_index];
    if (expected_size != model_.variables_sizes[var_index]) {
      throw Error{"Variable with index {} has a size different than {}",
                  var_index, expected_size};
    }
  };

  validate_(var_a_index, 0);
  validate_(var_b_index, 1);

  auto throw_alredy_connected_ = [&]() {
    throw Error{"Variables at index {} and {} were already connected!",
                var_a_index, var_b_index};
  };

  if (model_.binary_factors.contains(
          std::make_pair(var_b_index, var_a_index))) {
    throw_alredy_connected_();
  }

  bool inserted =
      model_.binary_factors
          .emplace(std::make_pair(var_a_index, var_b_index),
                   std::forward<factor::FactorT<2, Trasform>>(to_add))
          .second;

  if (!inserted) {
    throw_alredy_connected_();
  }
}
} // namespace EFG::structure
