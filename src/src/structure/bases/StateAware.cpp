/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/bases/StateAware.h>

#include <algorithm>

namespace EFG::strct {
categoric::VariablePtr StateAware::findVariable(const std::string &name) const {
  auto variables_it =
      std::find_if(state_.variables.begin(), state_.variables.end(),
                   [&name](const categoric::VariablePtr &var) {
                     return var->name() == name;
                   });
  if (variables_it == state_.variables.end()) {
    throw Error::make(name, " is an inexistent variable");
  }
  return *variables_it;
}

categoric::VariablesSet StateAware::getHiddenVariables() const {
  categoric::VariablesSet result;
  for (const auto &cluster : state_.clusters) {
    for (auto *node : cluster.nodes) {
      result.emplace(node->variable);
    }
  }
  return result;
}

categoric::VariablesSet StateAware::getObservedVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, _] : state_.evidences) {
    result.emplace(var);
  }
  return result;
}

std::optional<StateAware::NodeLocation>
StateAware::locate(const categoric::VariablePtr &variable) const {
  Node *node = nullptr;
  if (auto it = state_.nodes.find(variable); it == state_.nodes.end()) {
    return std::nullopt;
  } else {
    node = it->second.get();
  }
  if (auto it = state_.evidences.find(variable); it != state_.evidences.end()) {
    return NodeLocation{node, it};
  }
  auto it =
      std::find_if(state_.clusters.begin(), state_.clusters.end(),
                   [&node](const HiddenCluster &element) {
                     return element.nodes.find(node) != element.nodes.end();
                   });
  return NodeLocation{node, it};
}
} // namespace EFG::strct
