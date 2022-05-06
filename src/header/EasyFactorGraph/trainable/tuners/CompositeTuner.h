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
  Tuners &getElements() { return elements; };
  const Tuners &getElements() const { return elements; };

  CompositeTuner(TunerPtr elementA, TunerPtr elementB);

  float getGradientAlpha(const TrainSet::Iterator &iter) final;
  float getGradientBeta() final;
  void setWeight(const float &w) final;
  float getWeight() const final { return elements.front()->getWeight(); };

  void addElement(TunerPtr element);

private:
  Tuners elements;
};
} // namespace EFG::train
