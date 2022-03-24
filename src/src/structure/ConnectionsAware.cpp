/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/ConnectionsAware.h>

// #include "BeliefPropagationUtils.h"
#include "GraphStateUtils.h"

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
    visit(
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
  visit(
      node_location,
      [&unary_factor](const HiddenNodeLocation &location) {
        location.node->unary_factors.push_back(unary_factor);
        location.node->merged_contributions.reset();
      },
      [&unary_factor](const EvidenceNodeLocation &location) {
        location.node->unary_factors.push_back(unary_factor);
      });
}

namespace {
void check_are_already_connected(Node &a, Node &b) {
  if ((a.active_connections.find(&b) != a.active_connections.end()) ||
      (a.disabled_connections.find(&b) != a.disabled_connections.end())) {
    throw Error{a.variable->name(), " and ", b.variable->name(),
                " are already connected"};
  }
}

void throw_already_connected(const categoric::VariablePtr &a,
                             const categoric::VariablePtr &b) {
  throw Error{a->name(), " and ", b->name(), " are already connected"};
}
} // namespace

void ConnectionsAware::addBinaryDistribution(
    const EFG::distribution::DistributionCnstPtr &binary_factor) {
  const auto &vars = binary_factor->getVariables().getVariables();
  auto &varA = vars.front();
  auto &varB = vars.back();
  auto nodeA_location = findOrMakeNode(varA);
  auto nodeB_location = findOrMakeNode(varB);

  auto hybrid_insertion = [&](const HiddenNodeLocation &hidden,
                              const EvidenceNodeLocation &evidence) {
    auto *node_hidden = hidden.node;
    auto *node_evidence = evidence.node;
    check_are_already_connected(*node_hidden, *node_evidence);
    node_evidence->disabled_connections.emplace(
        node_hidden, Connection{nullptr, binary_factor});
    node_hidden->disabled_connections.emplace(
        node_evidence,
        Connection{make_evidence_message(binary_factor, node_evidence->variable,
                                         evidence.evidence->second),
                   binary_factor});
    node_hidden->merged_contributions.reset();
  };

  visit(
      nodeA_location,
      [&](const HiddenNodeLocation &hiddenA_location) {
        visit(
            nodeB_location,
            [&](const HiddenNodeLocation &hiddenB_location) {
              auto *nodeA = hiddenA_location.node;
              auto *nodeB = hiddenB_location.node;
              // both are hidden
              check_are_already_connected(*nodeA, *nodeB);
              nodeA->active_connections.emplace(
                  nodeB, Connection{nullptr, binary_factor});
              nodeB->active_connections.emplace(
                  nodeA, Connection{nullptr, binary_factor});
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
        visit(
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
                  nodeB, Connection{nullptr, binary_factor});
              nodeB->disabled_connections.emplace(
                  nodeA, Connection{nullptr, binary_factor});
            });
      });
  factorsAll.emplace(binary_factor);
}
} // namespace EFG::strct
