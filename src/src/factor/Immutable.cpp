/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/Immutable.h>

namespace EFG::factor {
Immutable::Immutable(FunctionPtr data) : function_{data} {
  if (data == nullptr) {
    throw Error{"null data"};
  }
}

std::vector<float> Immutable::getProbabilities() const {
  std::vector<float> probs;
  probs.reserve(function_->getInfo().totCombinations);
  function_->forEachCombination<true>(
      [&probs](const std::vector<std::size_t> &, float img) {
        probs.push_back(img);
      });
  // normalize values
  float sum = 0.f;
  for (const auto &val : probs) {
    sum += val;
  }
  if (sum == 0.f) {
    float e = 1.f / static_cast<float>(probs.size());
    for (auto &val : probs) {
      val = e;
    }
  } else {
    for (auto &val : probs) {
      val /= sum;
    }
  }
  return probs;
}

ImageFinder
Immutable::makeFinder(const categoric::VariablesSoup &bigger_group) const {
  return ImageFinder{function_, bigger_group};
}
} // namespace EFG::factor
