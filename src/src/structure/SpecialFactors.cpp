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

UnaryFactor::UnaryFactor(const std::vector<DistributionCnstPtr> &factors)
    : UnaryFactor(factors.front()->getVariables().getVariables().front()) {
  for (const auto &factor : factors) {
    merge(*factor);
  }
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
categoric::VariablePtr
get_remaining(const Distribution &binary_factor,
              const categoric::VariablePtr &evidence_var) {
  const auto &vars = binary_factor.getVariables().getVariables();
  if (vars.front() == evidence_var) {
    return vars.back();
  }
  return vars.front();
}
} // namespace

Evidence::Evidence(const Distribution &binary_factor,
                   const categoric::VariablePtr &evidence_var,
                   const std::size_t evidence)
    : UnaryFactor(get_remaining(binary_factor, evidence_var),
                  DONT_FILL_DOMAIN_TAG) {
  std::size_t pos_evidence = 0;
  std::size_t pos_hidden = 1;
  if (binary_factor.getVariables().getVariables().back() == evidence_var) {
    std::swap(pos_evidence, pos_hidden);
  }
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
