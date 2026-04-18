/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <math.h>

namespace EFG::factor {
struct NullTrasform {
  NullTrasform();

  float operator()(float val) const { return val; }
};

struct ExponentialTrasform {
  ExponentialTrasform();

  float operator()(float val) const { return expf(w_ * val); }

  /**
   * @brief sets the weight used by the exponential evaluator.
   */
  void setWeight(float w) { w_ = w; }

  /**
   * @return the weight used by the exponential evaluator.
   */
  float getWeight() const { return w_; }

private:
  float w_;
};
}
