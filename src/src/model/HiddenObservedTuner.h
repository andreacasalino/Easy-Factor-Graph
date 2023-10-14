/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/StateAware.h>
#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::train {
class HiddenObservedTuner : public BaseTuner {
public:
  HiddenObservedTuner(
      strct::Node &nodeHidden, const strct::Evidences::const_iterator &evidence,
      const std::shared_ptr<factor::FactorExponential> &factor,
      const categoric::VariablesSoup &variables_in_model);

  float getGradientBeta() final;

private:
  strct::Node &nodeHidden;
  strct::Evidences::const_iterator evidence;

  std::size_t pos_in_factor_hidden;
  std::size_t pos_in_factor_evidence;
};
} // namespace EFG::train
