/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

namespace EFG::distribution {
const UnaryFactor::DontFillDomainTag UnaryFactor::DONT_FILL_DOMAIN_TAG =
    UnaryFactor::DontFillDomainTag{};

UnaryFactor::UnaryFactor(const categoric::VariablePtr &var,
                         const DontFillDomainTag &)
    : distribution::Factor(categoric::Group{var}), variable(var) {}

UnaryFactor::UnaryFactor(const categoric::VariablePtr &var)
    : UnaryFactor(var, DONT_FILL_DOMAIN_TAG) {
  auto &map = getCombinationsMap_();
  for (std::size_t c = 0; c < var->size(); ++c) {
    map.emplace(std::vector<std::size_t>{c}, 1.f);
  }
}

UnaryFactor::UnaryFactor(
    const std::vector<const distribution::Distribution *> &factors)
    : UnaryFactor(factors.front()->getVariables().getVariables().front()) {
  for (const auto *factor : factors) {
    merge(*factor);
  }
  normalize();
}

void UnaryFactor::merge(const Distribution &to_merge) {
  {
    const auto &vars = to_merge.getVariables().getVariables();
    if (vars.size() != 1) {
      throw Error{"Invalid factor"};
    }
    if (vars.front().get() != this->variable.get()) {
      throw Error{"Invalid factor"};
    }
  }
  auto &map = getCombinationsMap_();
  const auto &to_merge_map = to_merge.getCombinationsMap();
  if (to_merge_map.size() == map.size()) {
    auto map_it = map.begin();
    for (auto to_merge_it = to_merge_map.begin();
         to_merge_it != to_merge_map.end(); ++to_merge_it, ++map_it) {
      map_it->second *= to_merge_it->second;
    }
  } else {
    for (auto &[comb, val] : map) {
      val *= to_merge.evaluate(comb);
    }
  }
}

void UnaryFactor::normalize() {
  auto &map = getCombinationsMap_();
  float coeff = map.begin()->second;
  for (const auto &[comb, val] : map) {
    if (coeff < val) {
      coeff = val;
    }
  }
  if (0 == coeff) {
    return;
  }
  coeff = 1.f / coeff;
  for (auto &[comb, val] : map) {
    val *= coeff;
  }
}

namespace {
categoric::VariablePtr get_other_var(const Distribution &binary_factor,
                                     const categoric::VariablePtr &var) {
  if (2 != binary_factor.getVariables().getVariables().size()) {
    throw Error{"invalid binary factor"};
  }
  const auto &vars = binary_factor.getVariables().getVariables();
  if (vars.front() == var) {
    return vars.back();
  }
  return vars.front();
}

void get_positions(const Distribution &binary_factor,
                   const categoric::VariablePtr &unary_factor_var,
                   std::size_t &unary_factor_var_pos,
                   std::size_t &other_var_pos) {
  unary_factor_var_pos = 0;
  other_var_pos = 1;
  if (binary_factor.getVariables().getVariables().back().get() ==
      unary_factor_var.get()) {
    std::swap(unary_factor_var_pos, other_var_pos);
  }
}
} // namespace

Evidence::Evidence(const Distribution &binary_factor,
                   const categoric::VariablePtr &evidence_var,
                   const std::size_t evidence)
    : UnaryFactor(get_other_var(binary_factor, evidence_var),
                  DONT_FILL_DOMAIN_TAG) {
  std::size_t pos_evidence;
  std::size_t pos_hidden;
  get_positions(binary_factor, getVariable(), pos_hidden, pos_evidence);
  auto &map = getCombinationsMap_();
  const auto &eval = binary_factor.getEvaluator();
  for (const auto &[comb, val] : binary_factor.getCombinationsMap()) {
    const auto &big_comb_data = comb.data();
    if (comb.data()[pos_evidence] == evidence) {
      categoric::Combination comb(
          std::vector<std::size_t>{big_comb_data[pos_hidden]});
      map.emplace(std::move(comb), eval.evaluate(val));
    }
  }
}

namespace {
void fill_message(
    const UnaryFactor &merged_unaries,
    const distribution::Distribution &binary_factor,
    distribution::CombinationRawValuesMap &recipient,
    const std::function<float(const std::vector<float> &)> &reduction) {
  std::size_t message_var_pos;
  std::size_t sender_var_pos;
  get_positions(binary_factor, merged_unaries.getVariable(), message_var_pos,
                sender_var_pos);
  std::vector<float> values;
  auto to_eliminate =
      binary_factor.getVariables().getVariables()[sender_var_pos];
  for (const auto &[comb, val] : merged_unaries.getCombinationsMap()) {
    values.clear();
    for (std::size_t sender_comb = 0; sender_comb < to_eliminate->size();
         ++sender_comb) {
      std::vector<std::size_t> binary_factor_comb;
      binary_factor_comb.resize(2);
      binary_factor_comb[sender_var_pos] = sender_comb;
      binary_factor_comb[message_var_pos] = comb.data().front();
      values.push_back(val * binary_factor.evaluate(categoric::Combination{
                                 std::move(binary_factor_comb)}));
    }
    recipient.emplace(comb, reduction(values));
  }
}
} // namespace

MessageSUM::MessageSUM(const UnaryFactor &merged_unaries,
                       const distribution::Distribution &binary_factor)
    : UnaryFactor(get_other_var(binary_factor, merged_unaries.getVariable()),
                  DONT_FILL_DOMAIN_TAG) {
  fill_message(merged_unaries, binary_factor, getCombinationsMap_(),
               [](const std::vector<float> &values) {
                 float result = 0;
                 for (const auto &val : values) {
                   result += val;
                 }
                 return result;
               });
}

MessageMAP::MessageMAP(const UnaryFactor &merged_unaries,
                       const distribution::Distribution &binary_factor)
    : UnaryFactor(get_other_var(binary_factor, merged_unaries.getVariable()),
                  DONT_FILL_DOMAIN_TAG) {
  fill_message(merged_unaries, binary_factor, getCombinationsMap_(),
               [](const std::vector<float> &values) {
                 float result = values.front();
                 for (const auto &val : values) {
                   if (val > result) {
                     result = val;
                   }
                 }
                 return result;
               });
}

Indicator::Indicator(const categoric::VariablePtr &var, const std::size_t value)
    : UnaryFactor(var) {
  if (value >= var->size()) {
    throw Error{"Invalid indicator factor"};
  }
  getCombinationsMap_().emplace(std::vector<std::size_t>{value}, 1);
}
} // namespace EFG::distribution

namespace EFG::strct {
std::unique_ptr<distribution::Evidence>
make_evidence(const distribution::Distribution &binary_factor,
              const categoric::VariablePtr &evidence_var,
              const std::size_t evidence) {
  return std::make_unique<distribution::Evidence>(binary_factor, evidence_var,
                                                  evidence);
}
} // namespace EFG::strct
