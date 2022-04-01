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
  std::unique_ptr<distribution::UnaryFactor> merged_a;
  {
    std::vector<const distribution::Distribution *> unaries = {
        nodeA.merged_unaries.get()};
    for (const auto &[connected_node, connection] : nodeA.active_connections) {
      unaries.push_back(connection.message.get());
    }
    merged_a = std::make_unique<distribution::UnaryFactor>(unaries);
  }
  std::unique_ptr<distribution::UnaryFactor> merged_b;
  {
    std::vector<const distribution::Distribution *> unaries = {
        nodeB.merged_unaries.get()};
    for (const auto &[connected_node, connection] : nodeB.active_connections) {
      unaries.push_back(connection.message.get());
    }
    merged_b = std::make_unique<distribution::UnaryFactor>(unaries);
  }

  throw 0; // are we sure the order of the variables in the merged factor is the
           // same of the original binary factor?
  return dotProduct(
      distribution::Factor{getFactor(), merged_a, merged_b}.getProbabilities());
}
} // namespace EFG::train
