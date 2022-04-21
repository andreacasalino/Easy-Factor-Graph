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
    const std::shared_ptr<distribution::FactorExponential> &factor,
    const categoric::VariablesSoup &variables_in_model)
    : BaseTuner(factor, variables_in_model), nodeA(nodeA), nodeB(nodeB) {
  const auto &variables = factor->getVariables().getVariables();
  if (variables.front().get() != nodeA.variable.get()) {
    throw Error{"Invalid BinaryTuner"};
  }
  if (variables.back().get() != nodeB.variable.get()) {
    throw Error{"Invalid BinaryTuner"};
  }
}

float BinaryTuner::getGradientBeta() {
  std::vector<float> merged_a;
  {
    std::vector<const distribution::Distribution *> unaries = {
        nodeA.merged_unaries.get()};
    for (const auto &[connected_node, connection] : nodeA.active_connections) {
      unaries.push_back(connection->message.get());
    }
    merged_a = distribution::UnaryFactor{unaries}.getProbabilities();
  }

  std::vector<float> merged_b;
  {
    std::vector<const distribution::Distribution *> unaries = {
        nodeB.merged_unaries.get()};
    for (const auto &[connected_node, connection] : nodeB.active_connections) {
      unaries.push_back(connection->message.get());
    }
    merged_b = distribution::UnaryFactor{unaries}.getProbabilities();
  }

  std::vector<float> probs;
  const auto &eval = getFactor().getEvaluator();
  probs.reserve(getFactor().getCombinationsMap().size());
  float probs_coeff = 0;
  for (const auto &[comb, val] : getFactor().getCombinationsMap()) {
    const auto &data = comb.data();
    probs.push_back(eval.evaluate(val) * merged_a[data[0]] * merged_b[data[1]]);
    probs_coeff += probs.back();
  }
  probs_coeff = 1.f / probs_coeff;
  for (auto &val : probs) {
    val *= probs_coeff;
  }
  return dotProduct(probs);
}
} // namespace EFG::train
