/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#include <EasyFactorGraph/io/json/Exporter.h>

#include "../Utils.h"

namespace EFG::io::json {
void Exporter::exportToFile(const nlohmann::json &source,
                            const std::filesystem::path &out) {
  useOutStrem(out,
              [&source](std::ofstream &stream) { stream << source.dump(); });
}

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
}

nlohmann::json &printPotential(const factor::Immutable &distr,
                               nlohmann::json &recipient) {
  auto &potential = recipient.emplace_back();
  printGroup(distr.function().vars(), potential["Variables"]);
  auto &Distr_val = potential["Distr_val"];
  distr.function().forEachNonNullCombination<false>(
      [&](const auto &comb, float img) {
        auto &Distr_val_added = Distr_val.emplace_back();
        Distr_val_added["D"] = std::to_string(img);
        auto &values = Distr_val_added["v"];
        for (const auto &v : comb) {
          values.emplace_back() = std::to_string(v);
        }
      });
  return potential;
}

nlohmann::json &printExpPotential(const factor::FactorExponential &distr,
                                  nlohmann::json &recipient) {
  auto &pot_tag = printPotential(distr, recipient);
  pot_tag["weight"] = std::to_string(distr.getWeight());
  return pot_tag;
}
} // namespace

void Exporter::convert(nlohmann::json &recipient, Getters subject) {
  auto [state, constGetter, tunableGetter] = subject;
  // hidden set
  auto &variables = recipient["Variables"];
  for (const auto &hidden_var : state->getHiddenVariables()) {
    auto &new_var = variables.emplace_back();
    printVariable(hidden_var, new_var);
  }
  // evidence set
  for (const auto &[evidence_var, evidence_val] : state->getEvidences()) {
    auto &new_var = variables.emplace_back();
    printVariable(evidence_var, new_var);
    new_var["evidence"] = std::to_string(evidence_val);
  }
  auto &potentials = recipient["Potentials"];
  // const factors
  for (const auto &const_factor : constGetter->getConstFactors()) {
    std::shared_ptr<const factor::FactorExponential> as_const_exp_factor =
        std::dynamic_pointer_cast<const factor::FactorExponential,
                                  const factor::Immutable>(const_factor);
    if (nullptr == as_const_exp_factor) {
      printPotential(*const_factor, potentials);
    } else {
      auto &factor_tag = printExpPotential(*as_const_exp_factor, potentials);
      factor_tag["tunability"] = "N";
    }
  }
  if (tunableGetter) {
    // tunable factors
    struct Visitor {
      nlohmann::json &potentials;

      void operator()(const train::FactorExponentialPtr &factor) const {
        printExpPotential(*factor, potentials)["tunability"] = "Y";
      }
      void operator()(const train::TunableClusters &cluster) const {
        printExpPotential(*cluster.front(), potentials)["tunability"] = "Y";
        const auto &vars = cluster.front()->function().vars();
        std::for_each(cluster.begin() + 1, cluster.end(),
                      [&](const train::FactorExponentialPtr &factor) {
                        auto &factor_json =
                            printExpPotential(*factor, potentials);
                        factor_json["tunability"] = "Y";
                        printGroup(vars, factor_json["Share"]);
                      });
      }
    } visitor{potentials};
    for (const auto &cluster : tunableGetter->getTunableClusters()) {
      std::visit(visitor, cluster);
    }
  }
}
} // namespace EFG::io::json

#endif
