/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <Printing.h>

#include <algorithm>

namespace {
std::size_t
find_var_position(EFG::categoric::VariablePtr var2Search,
                  const EFG::categoric::VariablesSoup &samplesGroup) {
  return std::distance(
      samplesGroup.begin(),
      std::find(samplesGroup.begin(), samplesGroup.end(), var2Search));
}
} // namespace

std::vector<float>
getEmpiricalMarginals(EFG::categoric::VariablePtr var2Search,
                      const std::vector<EFG::categoric::Combination> &samples,
                      const EFG::categoric::VariablesSoup &samplesGroup) {
  std::vector<std::size_t> counters;
  counters.reserve(var2Search->size());
  for (std::size_t k = 0; k < var2Search->size(); ++k) {
    counters.push_back(0);
  }

  const std::size_t var_pos = find_var_position(var2Search, samplesGroup);
  for (const auto &sample : samples) {
    ++counters[sample.data()[var_pos]];
  }

  std::vector<float> result;
  result.reserve(counters.size());
  for (const auto counter : counters) {
    result.push_back(static_cast<float>(counter) /
                     static_cast<float>(samples.size()));
  }
  return result;
}

float getEmpiricalProbability(
    const EFG::categoric::Combination &comb2Search,
    const EFG::categoric::VariablesSoup &combGroup,
    const std::vector<EFG::categoric::Combination> &samples,
    const EFG::categoric::VariablesSoup &samplesGroup) {
  std::size_t counter = 0;

  std::vector<std::size_t> var2Search_positions;
  for (const auto &var : combGroup) {
    var2Search_positions.push_back(find_var_position(var, samplesGroup));
  }

  for (const auto &sample : samples) {
    bool increment = true;
    const auto &sample_data = sample.data();
    const auto &comb2Search_data = comb2Search.data();
    for (std::size_t k = 0; k < var2Search_positions.size(); ++k) {
      if (sample_data[var2Search_positions[k]] != comb2Search_data[k]) {
        increment = false;
        break;
      }
    }
    if (increment) {
      ++counter;
    }
  }

  return static_cast<float>(counter) / static_cast<float>(samples.size());
}

std::vector<float> make_distribution(const std::vector<float> &values) {
  float coeff = 0;
  for (const auto value : values) {
    coeff += value;
  }
  coeff = 1.f / coeff;
  auto result = values;
  for (auto &value : result) {
    value *= coeff;
  }
  return result;
}
