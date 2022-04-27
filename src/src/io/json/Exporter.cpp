/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#include <EasyFactorGraph/io/json/Exporter.h>

namespace EFG::io::json {
namespace {
void printVariable(const categoric::VariablePtr &var,
                   nlohmann::json &recipient) {
  recipient["name"] = var->name();
  recipient["Size"] = std::to_string(var->size());
}

void printGroup(const categoric::Group &group, nlohmann::json &recipient) {
  for (const auto &var : group.getVariables()) {
    auto &added = recipient.emplace_back();
    added = var->name();
  }
};

nlohmann::json &printPotential(const distribution::Distribution &distr,
                               nlohmann::json &recipient) {
  auto &potential = recipient.emplace_back();
  printGroup(distr.getGroup(), potential["Variables"]);
  auto &Distr_val = potential["Distr_val"];
  for (const auto &[comb, val] : distr.getCombinationsMap()) {
    auto &Distr_val_added = Distr_val.emplace_back();
    Distr_val_added["D"] = std::to_string(val);
    auto &values = Distr_val_added["v"];
    for (const auto &v : comb.data()) {
      values.emplace_back() = std::to_string(v);
    }
  }
  return potential;
};

nlohmann::json &printExpPotential(const distribution::FactorExponential &distr,
                                  nlohmann::json &recipient) {
  auto &pot_tag = printPotential(distr, recipient);
  pot_tag["weight"] = std::to_string(distr.getWeight());
  return pot_tag;
}
} // namespace

void Exporter::convert(nlohmann::json &recipient, const AwarePtrs &subject) {
  // hidden set
  auto &variables = recipient["Variables"];
  for (const auto &hidden_var :
       subject.as_structure_aware->getHiddenVariables()) {
    auto &new_var = variables.emplace_back();
    printVariable(hidden_var, new_var);
  }
  // evidence set
  for (const auto &[evidence_var, evidence_val] :
       subject.as_structure_aware->getEvidences()) {
    auto &new_var = variables.emplace_back();
    printVariable(evidence_var, new_var);
    new_var["evidence"] = std::to_string(evidence_val);
  }
  auto &potentials = recipient["Potentials"];
  // const factors
  for (const auto &const_factor :
       subject.as_factors_const_aware->getConstFactors()) {
    std::shared_ptr<const distribution::FactorExponential> as_const_exp_factor =
        std::dynamic_pointer_cast<const distribution::FactorExponential,
                                  const distribution::Distribution>(
            const_factor);
    if (nullptr == as_const_exp_factor) {
      printPotential(*const_factor, potentials);
    } else {
      auto &factor_tag = printExpPotential(*as_const_exp_factor, potentials);
      factor_tag["tunability"] = "N";
    }
  }
  if (nullptr != subject.as_factors_tunable_aware) {
    // tunable factors
    for (const auto &cluster :
         subject.as_factors_tunable_aware->getTunableClusters()) {
      const auto &front_factor = *cluster.front();
      auto &factor_tag = printExpPotential(front_factor, potentials);
      factor_tag["tunability"] = "Y";
      for (std::size_t k = 1; k < cluster.size(); ++k) {
        auto &factor_tag2 = printExpPotential(*cluster[k], potentials);
        printGroup(front_factor.getGroup(), factor_tag2["Share"]);
        factor_tag2["tunability"] = "Y";
      }
    }
  }
}
} // namespace EFG::io::json

#endif
