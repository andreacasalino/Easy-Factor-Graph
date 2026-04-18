/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/UniformSampler.h>

#include <algorithm>
#include <time.h>

namespace EFG::misc {
UniformSampler::UniformSampler() {
  auto random_seed = static_cast<unsigned int>(time(NULL));
  resetSeed(random_seed);
}

void UniformSampler::resetSeed(std::size_t newSeed) {
  this->generator.seed(static_cast<unsigned int>(newSeed));
}

std::size_t UniformSampler::sampleFromDiscrete(std::span<const float> distribution) const {
  float s = this->sample();
  float cumul = 0.f;
  for (std::size_t k = 0; k < distribution.size(); ++k) {
    cumul += distribution[k];
    if (s <= cumul) {
      return k;
    }
  }
  return distribution.size() - 1;
}
} // namespace EFG::strct
