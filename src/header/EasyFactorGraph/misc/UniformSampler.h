/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <random>
#include <vector>
#include <span>

namespace EFG::misc {
class UniformSampler {
public:
  UniformSampler();

  std::size_t sampleFromDiscrete(const std::vector<float> &distribution) const {
    return sampleFromDiscrete(std::span<const float>{distribution.begin(), distribution.end()});
  }

  std::size_t sampleFromDiscrete(std::span<const float> distribution) const;

  void resetSeed(std::size_t newSeed);

private:
  float sample() const { return this->distribution(this->generator); };

  mutable std::default_random_engine generator;
  mutable std::uniform_real_distribution<float> distribution;
};
} // namespace EFG::strct
