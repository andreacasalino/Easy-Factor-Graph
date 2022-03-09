/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <memory>

namespace EFG::distribution {
class Evaluator {
public:
  virtual ~Evaluator() = default;

  /**
   * @brief applies a specific function to obtain the image from a the raw image
   * value
   * @param the raw value to convert
   * @return the converted image
   */
  virtual float evaluate(const float &input) const = 0;
};

using EvaluatorPtr = std::shared_ptr<Evaluator>;
} // namespace EFG::distribution
