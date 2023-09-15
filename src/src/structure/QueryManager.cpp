/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Visitor.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/structure/bases/StateAware.h>

namespace EFG::strct {
namespace {
factor::MergedUnaries gather_incoming_messages(Node &subject) {
  std::vector<const factor::Immutable *> messages = {
      subject.merged_unaries.get()};
  for (const auto &[connected_node, message] : subject.active_connections) {
    messages.push_back(message.message.get());
  }
  if (messages.empty()) {
    return factor::MergedUnaries{subject.variable};
  }
  return factor::MergedUnaries{messages};
}

std::vector<float> zeros(std::size_t size) {
  std::vector<float> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(0);
  }
  return result;
}

} // namespace

std::vector<float>
QueryManager::getMarginalDistribution(const NodeLocation &location) {
  std::vector<float> result;
  auto &[node, it] = location;
  VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
      [&result, &node = node](const HiddenClusters::iterator &) {
        result = gather_incoming_messages(*node).getProbabilities();
      },
      [&result, &node = node](const Evidences::iterator &location) {
        result = zeros(node->variable->size());
        result[location->second] = 1.f;
      }}
      .visit(it);
  return result;
}

void QueryManager::throwInexistentVar(const std::string &var) {
  throw Error::make(var, " is a not part of the graph");
}

std::vector<float> QueryManager::getMarginalDistribution(const std::string &var,
                                                         std::size_t threads) {
  return getMarginalDistribution(findVariable(var), threads);
}

namespace {
std::size_t find_max(const std::vector<float> &values) {
  std::size_t res = 0;
  float max = values.front();
  for (std::size_t k = 1; k < values.size(); ++k) {
    if (values[k] > max) {
      max = values[k];
      res = k;
    }
  }
  return res;
}
} // namespace

std::size_t QueryManager::getMAP(const categoric::VariablePtr &var,
                                 std::size_t threads) {
  auto values = marginalQuery_<PropagationKind::MAP>(var, threads);
  return find_max(values);
}

std::size_t QueryManager::getMAP(const std::string &var, std::size_t threads) {
  return getMAP(findVariable(var), threads);
}

std::vector<size_t> QueryManager::getHiddenSetMAP(std::size_t threads) {
  checkPropagation_<PropagationKind::MAP>(threads);
  auto vars = getHiddenVariables();
  std::vector<size_t> result;
  result.reserve(vars.size());
  auto &nodes = stateMutable().nodes;
  for (const auto &var : vars) {
    auto values = gather_incoming_messages(*nodes[var]).getProbabilities();
    result.push_back(find_max(values));
  }
  return result;
}

factor::Factor
QueryManager::getJointMarginalDistribution(const categoric::Group &subgroup,
                                           std::size_t threads) {
  checkPropagation_<PropagationKind::SUM>(threads);
  std::vector<NodeLocation> locations;
  std::unordered_set<Node *> subgroup_nodes;
  for (const auto &var_name : subgroup.getVariables()) {
    auto location = locate(var_name);
    if (!location.has_value()) {
      throwInexistentVar(var_name->name());
    }
    subgroup_nodes.emplace(location->node);
    locations.emplace_back(location.value());
  }

  std::unordered_set<const factor::Immutable *> contributions;
  std::vector<factor::Indicator> indicators;
  for (auto &[node, location] : locations) {
    VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
        [&node = node, &subgroup_nodes,
         &contributions](const HiddenClusters::iterator &) {
          contributions.emplace(node->merged_unaries.get());
          for (const auto &[connected_node, connection] :
               node->active_connections) {
            if (subgroup_nodes.find(connected_node) == subgroup_nodes.end()) {
              contributions.emplace(connection.message.get());
            } else {
              contributions.emplace(connection.factor.get());
            }
          }
        },
        [&node = node, &contributions,
         &indicators](const Evidences::iterator &location) {
          auto &added =
              indicators.emplace_back(node->variable, location->second);
          contributions.emplace(&added);
        }}
        .visit(location);
  }

  return factor::Factor{std::vector<const factor::Immutable *>{
                            contributions.begin(), contributions.end()}}
      .cloneWithPermutedGroup(subgroup);
}

factor::Factor QueryManager::getJointMarginalDistribution(
    const std::vector<std::string> &subgroup, std::size_t threads) {
  categoric::VariablesSoup vars;
  for (const auto &var : subgroup) {
    vars.push_back(findVariable(var));
  }
  return getJointMarginalDistribution(categoric::Group{vars}, threads);
}
} // namespace EFG::strct
