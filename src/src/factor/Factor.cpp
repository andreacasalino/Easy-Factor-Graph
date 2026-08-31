/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/Factor.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace EFG::factor {
void UnaryFactorsMerger::normalize() {
  float coeff = 1.f / *std::max_element(values_.begin(), values_.end());
  for (auto &val : values_) {
    val *= coeff;
  }
}

factor::UnaryFactor make_all_same(std::span<float> support, float value) {
  std::fill_n(support.data(), support.size(), value);
  return factor::UnaryFactor{misc::Slot<float>::makeNonOwning(support)};
}

factor::UnaryFactor make_indicator(std::span<float> support,
                                   categoric::VarStateSize value) {
  std::fill_n(support.data(), support.size(), 0);
  support[value] = 1.f;
  return factor::UnaryFactor{misc::Slot<float>::makeNonOwning(support)};
}

void normalize_probabilities(std::vector<float> &values) {
  if (values.empty()) {
    return;
  }

  // normalize values
  float sum = 0.f;
  for (const auto &val : values) {
    sum += val;
  }
  if (sum == 0.f) {
    float e = 1.f / static_cast<float>(values.size());
    for (auto &val : values) {
      val = e;
    }
  } else {
    for (auto &val : values) {
      val /= sum;
    }
  }
}

std::vector<float> make_probabilities(std::vector<float> values) {
  normalize_probabilities(values);
  return std::move(values);
}
} // namespace EFG::factor
