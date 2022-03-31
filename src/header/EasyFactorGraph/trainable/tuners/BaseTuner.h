/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/trainable/tuners/Tuner.h>

namespace EFG::train {
class BaseTuner : public Tuner {
public:
  void setTrainSetIterator(const TrainSet::Iterator &iter,
                           const categoric::VariablesSet &vars) final;

  float getGradientAlpha() final { return gradientAlpha; };
  void setWeight(const float &w) final { factor->setWeight(w); }

protected:
  BaseHandler(const std::shared_ptr<distribution::FactorExponential> &factor);

  float dotProduct(const std::vector<float> &prob) const;

  std::shared_ptr<distribution::FactorExponential> factor;
  float gradientAlpha = 0.f;
};
} // namespace EFG::train
