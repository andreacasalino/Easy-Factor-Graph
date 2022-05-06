/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/StateAware.h>
#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::train {
class UnaryTuner : public BaseTuner {
public:
  UnaryTuner(strct::Node &node,
             const std::shared_ptr<distribution::FactorExponential> &factor,
             const categoric::VariablesSoup &variables_in_model);

  float getGradientBeta() final;

protected:
  strct::Node &node;
};
} // namespace EFG::train
