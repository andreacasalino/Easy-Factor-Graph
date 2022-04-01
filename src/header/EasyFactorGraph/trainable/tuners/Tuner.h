/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Group.h>
#include <EasyFactorGraph/trainable/TrainSet.h>

namespace EFG::train {
class Tuner {
public:
  virtual ~Tuner() = default;

  virtual void setTrainSetIterator(const TrainSet::Iterator &iter,
                                   const categoric::VariablesSet &vars) = 0;

  virtual float getGradientAlpha() = 0;
  virtual float getGradientBeta() = 0;
  virtual void setWeight(const float &w) = 0;

protected:
  Tuner() = default;
};

using TunerPtr = std::unique_ptr<Tuner>;
} // namespace EFG::train