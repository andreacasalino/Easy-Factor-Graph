/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Group.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/trainable/TrainSet.h>

namespace EFG::train {
using FactorExponentialPtr = std::shared_ptr<factor::FactorExponential>;

class Tuner {
public:
  virtual ~Tuner() = default;

  virtual float getGradientAlpha(const TrainSet::Iterator &iter) = 0;
  virtual float getGradientBeta() = 0;

  virtual void setWeight(float w) = 0;
  virtual float getWeight() const = 0;
};

using TunerPtr = std::unique_ptr<Tuner>;

using Tuners = std::vector<TunerPtr>;
} // namespace EFG::train
