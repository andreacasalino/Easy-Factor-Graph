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
} // namespace EFG::factor
