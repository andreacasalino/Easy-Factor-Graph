/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/ImageFinder.h>
#include <EasyFactorGraph/trainable/tuners/Tuner.h>

namespace EFG::train {
class BaseTuner : public Tuner {
public:
  FactorExponentialPtr getFactorPtr() const { return factor; }
  const factor::FactorExponential &getFactor() const { return *factor; }

  float getGradientAlpha(const TrainSet::Iterator &iter) final;
  void setWeight(float w) final { factor->setWeight(w); }
  float getWeight() const final { return factor->getWeight(); };

protected:
  BaseTuner(const FactorExponentialPtr &factor,
            const categoric::VariablesSoup &variables_in_model);

  float dotProduct(const std::vector<float> &prob) const;

private:
  FactorExponentialPtr factor;

  factor::ImageFinder finder;
  struct GradientAlphaPart {
    const TrainSet::Iterator *train_set_iterator;
    float value;
  };
  std::optional<GradientAlphaPart> alpha_part;
};
} // namespace EFG::train
