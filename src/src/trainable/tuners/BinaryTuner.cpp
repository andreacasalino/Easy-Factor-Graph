/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/trainable/tuners/BinaryTuner.h>

namespace EFG::train {
BinaryTuner::BinaryTuner(
    strct::Node &nodeA, strct::Node &nodeB,
    const std::shared_ptr<factor::FactorExponential> &factor,
    const categoric::VariablesSoup &variables_in_model)
    : BaseTuner(factor, variables_in_model), nodeA(nodeA), nodeB(nodeB) {
  const auto &variables = factor->function().vars().getVariables();
  if (variables.front().get() != nodeA.variable.get()) {
    throw Error{"Invalid BinaryTuner"};
  }
  if (variables.back().get() != nodeB.variable.get()) {
    throw Error{"Invalid BinaryTuner"};
  }
}

namespace {
std::pair<std::vector<float>, std::vector<float>>
marginal_distributions(strct::Node &a, strct::Node &b) {
  auto gather = [](strct::Node &subject, strct::Node &other) {
    std::unordered_set<const factor::Immutable *> res{
        subject.merged_unaries.get()};
    for (const auto &[node, conn] : subject.active_connections) {
      if (node == &other) {
        continue;
      }
      res.emplace(conn.message.get());
    }
    return std::vector<const factor::Immutable *>{res.begin(), res.end()};
  };
  return std::make_pair(factor::MergedUnaries{gather(a, b)}.getProbabilities(),
                        factor::MergedUnaries{gather(b, a)}.getProbabilities());
}
} // namespace

float BinaryTuner::getGradientBeta() {
  auto marginals = marginal_distributions(nodeA, nodeB);
  auto &merged_a = marginals.first;
  auto &merged_b = marginals.second;
  std::vector<float> probs;
  probs.reserve(getFactor().function().getInfo().totCombinations);
  float probs_coeff = 0;
  getFactor().function().forEachCombination<true>(
      [&](const auto &comb, float img) {
        probs_coeff +=
            probs.emplace_back(img * merged_a[comb[0]] * merged_b[comb[1]]);
      });
  probs_coeff = 1.f / probs_coeff;
  for (auto &val : probs) {
    val *= probs_coeff;
  }
  return dotProduct(probs);
}
} // namespace EFG::train
