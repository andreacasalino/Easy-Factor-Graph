/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/trainable/TrainSet.h>

namespace EFG::train {
class Tuner {
public:
  virtual ~Tuner() = default;

  virtual void setTrainSetIterator(const TrainSet::Iterator &iter) = 0;

  virtual float getGradientAlpha() = 0;
  virtual void setWeight(const float &w) = 0;

protected:
  Tuner() = default;
};
} // namespace EFG::train
