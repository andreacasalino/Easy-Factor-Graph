/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/ConnectionsAware.h>

#include "Utils.h"

#include <algorithm>

namespace EFG::strct {
categoric::VariablesSet ConnectionsAware::getVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, el] : state->nodes) {
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

namespace {
void check_is_same_variable(const categoric::VariablePtr &a,
                            const categoric::VariablePtr &b) {
  if (a.get() != b.get()) {
    throw Error{"Trying to insert variable named: ", a->name(),
                " multiple times with different VariablePtr"};
  }
}
} // namespace

NodeLocation
ConnectionsAware::findOrMakeNode(const categoric::VariablePtr &var) {
  auto info = find_node(*state, var);
  if (info) {
    visit_location(
        *info,
        [&var](const HiddenNodeLocation &info) {
          check_is_same_variable(var, info.node->variable);
        },
        [&var](const EvidenceNodeLocation &info) {
          check_is_same_variable(var, info.node->variable);
        });
    return *info;
  }
  // create this node
  auto *added = &state->nodes.emplace(var, Node{}).first->second;
  added->variable = var;
  HiddenNodeLocation result;
  state->clusters.emplace_back().nodes.emplace(added);
  result.cluster = state->clusters.end();
  --result.cluster;
  result.node = added;
  return result;
}

void ConnectionsAware::addUnaryDistribution(
    const EFG::distribution::DistributionCnstPtr &unary_factor) {
  const auto &var = unary_factor->getVariables().getVariables().front();
  auto node_location = findOrMakeNode(var);
  factorsAll.emplace(unary_factor);
  Node *node = nullptr;
  visit_location(
      node_location,
      [&node](const HiddenNodeLocation &location) { node = location.node; },
      [&node](const EvidenceNodeLocation &location) { node = location.node; });
  node->unary_factors.push_back(unary_factor);
  node->merged_unaries.reset();
}

namespace {
void check_are_already_connected(Node &a, Node &b) {
  if ((a.active_connections.find(&b) != a.active_connections.end()) ||
      (a.disabled_connections.find(&b) != a.disabled_connections.end())) {
    throw Error{a.variable->name(), " and ", b.variable->name(),
                " are already connected"};
  }
}

void connect(Node &a, Node &b,
             const distribution::DistributionCnstPtr &factor) {
  a.active_connections[&b].factor = factor;
  b.active_connections[&a].factor = factor;
  a.disabled_connections.erase(&b);
  b.disabled_connections.erase(&a);
}
} // namespace

void ConnectionsAware::addBinaryDistribution(
    const EFG::distribution::DistributionCnstPtr &binary_factor) {
  const auto &vars = binary_factor->getVariables().getVariables();
  auto nodeA_location = findOrMakeNode(vars.front());
  auto nodeB_location = findOrMakeNode(vars.back());

  auto hybrid_insertion = [&](const HiddenNodeLocation &hidden,
                              const EvidenceNodeLocation &evidence) {
    auto *node_hidden = hidden.node;
    auto *node_evidence = evidence.node;
    check_are_already_connected(*node_hidden, *node_evidence);
    node_evidence->disabled_connections.emplace(
        node_hidden, Connection{binary_factor, nullptr});
    node_hidden->disabled_connections.emplace(
        node_evidence,
        Connection{binary_factor,
                   make_evidence(*binary_factor, node_evidence->variable,
                                 evidence.evidence->second)});
    node_hidden->merged_unaries.reset();
  };

  visit_location(
      nodeA_location,
      [&](const HiddenNodeLocation &hiddenA_location) {
        visit_location(
            nodeB_location,
            [&](const HiddenNodeLocation &hiddenB_location) {
              auto *nodeA = hiddenA_location.node;
              auto *nodeB = hiddenB_location.node;
              // both are hidden
              check_are_already_connected(*nodeA, *nodeB);
              connect(*nodeA, *nodeB, binary_factor);
              hiddenA_location.cluster->connectivity.reset();
              if (hiddenA_location.cluster != hiddenB_location.cluster) {
                hiddenA_location.cluster->nodes.insert(
                    hiddenB_location.cluster->nodes.begin(),
                    hiddenB_location.cluster->nodes.end());
                this->state->clusters.erase(hiddenB_location.cluster);
              }
            },
            [&](const EvidenceNodeLocation &evidenceB_location) {
              hybrid_insertion(hiddenA_location, evidenceB_location);
            });
      },
      [&](const EvidenceNodeLocation &evidenceA_location) {
        visit_location(
            nodeB_location,
            [&](const HiddenNodeLocation &hiddenB_location) {
              hybrid_insertion(hiddenB_location, evidenceA_location);
            },
            [&](const EvidenceNodeLocation &evidenceB_location) {
              auto *nodeA = evidenceA_location.node;
              auto *nodeB = evidenceB_location.node;
              // both are evidences
              check_are_already_connected(*nodeA, *nodeB);
              nodeA->disabled_connections.emplace(
                  nodeB, Connection{binary_factor, nullptr});
              nodeB->disabled_connections.emplace(
                  nodeA, Connection{binary_factor, nullptr});
            });
      });
  factorsAll.emplace(binary_factor);
}
} // namespace EFG::strct
