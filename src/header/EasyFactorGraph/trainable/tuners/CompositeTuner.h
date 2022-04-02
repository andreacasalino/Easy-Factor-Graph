/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/trainable/tuners/Tuner.h>

#include <vector>

namespace EFG::train {
class CompositeTuner : public Tuner {
public:
  CompositeTuner(TunerPtr elementA, TunerPtr elementB);

  void setTrainSetIterator(const TrainSet::Iterator &iter) final;

  float getGradientAlpha() final;
  float getGradientBeta() final;
  void setWeight(const float &w) final;
  float getWeight() const final { return elements.front()->getWeight(); };
  bool isHereGroup(const categoric::VariablesSet &group) const final;

  void addElement(TunerPtr element);

private:
  std::vector<TunerPtr> elements;
};
} // namespace EFG::train
