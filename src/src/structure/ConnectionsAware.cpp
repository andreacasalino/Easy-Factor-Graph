/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/ConnectionsAware.h>

#include "GraphStateUtils.h"

namespace EFG::strct {
categoric::VariablesSet ConnectionsAware::getVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, node] : state->nodes) {
    result.emplace(var);
  }
  return result;
}

categoric::VariablePtr
ConnectionsAware::findVariable(const std::string &name) const {
  auto nodes_it = state->nodes.find(categoric::make_variable(2, name));
  if (nodes_it == state->nodes.end()) {
    throw Error{name, " is an inexistent variable"};
  }
  return nodes_it->first;
}

Node &ConnectionsAware::findOrMakeNode(const categoric::VariablePtr &var) {
  auto nodes_it = state->nodes.find(var);
  if (nodes_it == state->nodes.end()) {
    nodes_it = state->nodes.emplace(var, Node{}).first;
    nodes_it->second.variable = var;
    auto &added = state->hidden_clusters.emplace_back();
    added.nodes.emplace(&nodes_it->second);
  } else if (nodes_it->first.get() != var.get()) {
    throw Error{"Trying to insert variable named: ", var->name(),
                " multiple times with different VariablePtr"};
  }
  return nodes_it->second;
}

void ConnectionsAware::addUnaryDistribution(
    const EFG::distribution::DistributionCnstPtr &unary_factor) {
  const auto &vars = unary_factor->getVariables().getVariables();
  auto nodeA = findOrMakeNode(vars.front());
  nodeA.unaryFactors.push_back(unary_factor);
  factorsAll.emplace(unary_factor);
}

namespace {
void connect(const EFG::distribution::DistributionCnstPtr &binary_factor,
             Node &A, Node &B) {
  if (A.activeConnections.find(&B) != A.activeConnections.end()) {
    throw Error{A.variable->name(), " and ", B.variable->name(),
                " are already connected with a factor"};
  }
  if (A.disabledConnections.find(&B) != A.disabledConnections.end()) {
    throw Error{A.variable->name(), " and ", B.variable->name(),
                " are already connected with a factor"};
  }
  A.activeConnections.emplace(&B, Connection{binary_factor, nullptr});
  B.activeConnections.emplace(&A, Connection{binary_factor, nullptr});
}
} // namespace

void ConnectionsAware::addBinaryDistribution(
    const EFG::distribution::DistributionCnstPtr &binary_factor) {
  const auto &vars = binary_factor->getVariables().getVariables();
  auto nodeA = findOrMakeNode(vars.front());
  auto nodeB = findOrMakeNode(vars.back());

  connect(binary_factor, nodeA, nodeB);
  factorsAll.emplace(binary_factor);

  auto nodeA_info = find_node(*state, nodeA);
  auto nodeB_info = find_node(*state, nodeB);

  auto *nodeA_as_hidden =
      std::get_if<std::vector<HiddenCluster>::iterator>(&nodeA_info);
  auto *nodeB_as_hidden =
      std::get_if<std::vector<HiddenCluster>::iterator>(&nodeB_info);
  if ((nullptr != nodeA_as_hidden) && (nullptr != nodeB_as_hidden)) {
    if (*nodeA_as_hidden != *nodeB_as_hidden) {
      // merge clusters
      (*nodeA_as_hidden)
          ->insert((*nodeB_as_hidden)->begin(), (*nodeB_as_hidden)->end());
      state->hidden_clusters.erase(*nodeB_as_hidden);
    }
    return;
  }

  disable_connection(nodeA, nodeB);

  auto *nodeA_as_evidence = std::get_if<Evidences::iterator>(&nodeA_info);
  auto *nodeB_as_evidence = std::get_if<Evidences::iterator>(&nodeB_info);
  if ((nullptr != nodeA_as_evidence) && (nullptr != nodeB_as_evidence)) {
    return;
  }

  if (nullptr == nodeA_as_hidden) {
    // nodeA is observation, nodeB is hidden
    nodeB.disabledConnections[&nodeA].message2ThisNode = make_evidence_message(
        binary_factor, nodeA.variable, (*nodeA_as_evidence)->second);
    return;
  }

  // nodeB is observation, nodeA is hidden
  nodeA.disabledConnections[&nodeA].message2ThisNode = make_evidence_message(
      binary_factor, nodeB.variable, (*nodeB_as_evidence)->second);
}

void ConnectionsAware::addDistribution(
    const EFG::distribution::DistributionCnstPtr &distribution) {
  if (nullptr == distribution) {
    throw Error{"null distribution can't be add"};
  }
  if (factorsAll.find(distribution) != factorsAll.end()) {
    throw Error{"Already inserted factor"};
  }
  resetBelief();
  switch (distribution->getVariables().getVariables().size()) {
  case 1:
    addUnaryDistribution(distribution);
    return;
  case 2:
    addBinaryDistribution(distribution);
    return;
  default:
    break;
  }
  throw Error{"Factor with invalid number of variables"};
}
} // namespace EFG::strct
