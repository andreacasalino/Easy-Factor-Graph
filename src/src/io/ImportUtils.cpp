/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "ImportUtils.h"

namespace EFG::io {

void ImportPredicate::importConst(
    const distribution::DistributionCnstPtr &factor) {
  model.as_factors_const_adder->addConstFactor(factor);
}

void ImportPredicate::importTunable(
    const std::shared_ptr<distribution::FactorExponential> &factor,
    const std::optional<categoric::VariablesSet> &sharing_group) {
  if (nullptr == model.as_factors_tunable_adder) {
    model.as_factors_const_adder->addConstFactor(factor);
    return;
  }
  if (std::nullopt == sharing_group) {
    model.as_factors_tunable_adder->addTunableFactor(factor);
    return;
  }
  this->cumulated.push_back(FactorAndSharingGroup{factor, *sharing_group});
}

void ImportPredicate::importCumulatedTunable() const {
  for (const auto &remaining : cumulated) {
    if (nullptr == model.as_factors_tunable_adder) {
      model.as_factors_const_adder->addConstFactor(remaining.factor);
      return;
    }
    model.as_factors_tunable_adder->addTunableFactor(remaining.factor,
                                                     remaining.group);
  }
}

} // namespace EFG::io
