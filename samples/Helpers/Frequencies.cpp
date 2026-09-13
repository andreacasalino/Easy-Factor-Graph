/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <Frequencies.h>
#include <Printing.h>

#include <algorithm>
#include <ranges>
#include <unordered_map>

std::vector<float> getEmpiricalMarginals(std::size_t var_index,
                                         EFG::categoric::VarStateSize var_size,
                                         const EFG::misc::Samples &samples) {
  std::vector<std::size_t> freq;
  freq.resize(var_size, 0);
  EFG::misc::for_each_generated<std::span<const EFG::categoric::VarStateSize>>(
      samples.makeIter(),
      [&](const auto &comb) { freq[comb[var_index]] += 1; });
  auto res =
      freq |
      std::views::transform(
          [den = static_cast<float>(samples.samplesCount())](auto val) mutable {
            return static_cast<float>(val) / den;
          });
  return {res.begin(), res.end()};
}

float getEmpiricalProbability(
    const std::vector<std::pair<std::size_t, EFG::categoric::VarStateSize>>
        &to_search,
    const EFG::misc::Samples &samples) {
  std::size_t freq{0};
  EFG::misc::for_each_generated<std::span<const EFG::categoric::VarStateSize>>(
      samples.makeIter(), [&](const auto &comb) {
        for (auto [index, val] : to_search) {
          if (comb[index] != val) {
            return;
          }
        }
        freq += 1;
      });
  return static_cast<float>(freq) / static_cast<float>(samples.samplesCount());
}

float getEmpiricalProbabilityInsideHidden(
    const std::vector<
        std::pair<std::size_t /* var index ... absolute */,
                  EFG::categoric::VarStateSize /* var value to seach */>>
        &to_search,
    const EFG::misc::Samples &samples,
    const EFG::structure::Structure &context) {
  std::unordered_map<std::size_t, EFG::categoric::VarStateSize> to_search_table{
      to_search.begin(), to_search.end()};
  std::vector<std::pair<std::size_t, EFG::categoric::VarStateSize>> mapped;
  std::size_t i_rel{0};
  for (std::size_t i = 0;
       i < context.nodes.size() && mapped.size() < to_search.size(); ++i) {
    const auto &node = context.nodes[i];
    if (node.evidence == EFG::structure::Evidence::NOT_AN_EVIDENCE) {
      auto it = to_search_table.find(i);
      if (it != to_search_table.end()) {
        mapped.emplace_back(std::make_pair(i_rel, it->second));
      }
      i_rel += 1;
    }
  }

  return getEmpiricalProbability(mapped, samples);
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
