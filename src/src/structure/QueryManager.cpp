/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include "Utils.h"

namespace EFG::strct {
namespace {
distribution::UnaryFactor gather_incoming_messages(Node &subject) {
  std::vector<const distribution::Distribution *> messages = {
      subject.merged_unaries.get()};
  for (const auto &[connected_node, message] : subject.active_connections) {
    messages.push_back(message->message.get());
  }
  if (messages.empty()) {
    return distribution::UnaryFactor{subject.variable};
  }
  return distribution::UnaryFactor{messages};
}

std::vector<float> zeros(const std::size_t size) {
  std::vector<float> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(0);
  }
  return result;
}

std::vector<float> get_marginal_distribution(const NodeLocation &location) {
  std::vector<float> result;
  visit_location(
      location,
      [&result](const HiddenNodeLocation &location) {
        result = gather_incoming_messages(*location.node).getProbabilities();
      },
      [&result](const EvidenceNodeLocation &location) {
        result = zeros(location.node->variable->size());
        result[location.evidence->second] = 1.f;
      });
  return result;
}

void throw_inexistent_var(const std::string &var) {
  throw Error{var, " is a not part of the graph"};
}
} // namespace

std::vector<float>
QueryManager::getMarginalDistribution(const categoric::VariablePtr &var,
                                      const std::size_t threads) {
  if (wouldNeedPropagation(SUM)) {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(SUM);
  }
  auto location = locate(var);
  if (!location) {
    throw_inexistent_var(var->name());
  }
  return get_marginal_distribution(*location);
}

std::vector<float>
QueryManager::getMarginalDistribution(const std::string &var,
                                      const std::size_t threads) {
  return getMarginalDistribution(findVariable(var), threads);
}

namespace {
std::size_t find_max(const std::vector<float> &values) {
  std::size_t result_max_pos = 0;
  for (std::size_t k = 0; k < values.size(); ++k) {
    if (values[k] > values[result_max_pos]) {
      result_max_pos = k;
    }
  }
  return result_max_pos;
}
} // namespace

std::size_t QueryManager::getMAP(const categoric::VariablePtr &var,
                                 const std::size_t threads) {
  if (wouldNeedPropagation(MAP)) {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(MAP);
  }
  auto location = locate(var);
  if (!location) {
    throw_inexistent_var(var->name());
  }
  auto values = get_marginal_distribution(*location);
  return find_max(values);
}

std::size_t QueryManager::getMAP(const std::string &var,
                                 const std::size_t threads) {
  return getMAP(findVariable(var), threads);
}

std::vector<size_t> QueryManager::getHiddenSetMAP(const std::size_t threads) {
  if (wouldNeedPropagation(MAP)) {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(MAP);
  }
  auto vars = getHiddenVariables();
  std::vector<size_t> result;
  result.reserve(vars.size());
  auto &nodes = getState_().nodes;
  for (const auto &var : vars) {
    auto values = gather_incoming_messages(*nodes[var]).getProbabilities();
    result.push_back(find_max(values));
  }
  return result;
}

namespace {
class SubgraphFactor : public distribution::Factor {
public:
  SubgraphFactor(
      const std::vector<const distribution::Distribution *> &contributions)
      : distribution::Factor(contributions) {}
};
} // namespace

std::unique_ptr<distribution::Distribution>
QueryManager::getJointMarginalDistribution(
    const categoric::VariablesSet &subgroup, const std::size_t threads) {
  if (wouldNeedPropagation(SUM)) {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(SUM);
  }
  std::map<Node *, NodeLocation> subgroup_locations;
  for (const auto &var_name : subgroup) {
    auto location = locate(var_name);
    if (!location) {
      throw_inexistent_var(var_name->name());
    }
    Node *node = nullptr;
    visit_location(
        *location,
        [&node](const HiddenNodeLocation &location) { node = location.node; },
        [&node](const EvidenceNodeLocation &location) {
          node = location.node;
        });
    subgroup_locations[node] = *location;
  }

  std::set<const distribution::Distribution *> contributions;
  std::vector<distribution::Indicator> indicators;
  for (const auto &[node, location] : subgroup_locations) {
    visit_location(
        location,
        [&](const HiddenNodeLocation &location) {
          contributions.emplace(location.node->merged_unaries.get());
          for (const auto &[connected_node, connection] :
               location.node->active_connections) {
            auto subgroup_locations_it =
                subgroup_locations.find(connected_node);
            if (subgroup_locations_it == subgroup_locations.end()) {
              contributions.emplace(connection->message.get());
            } else {
              contributions.emplace(connection->factor.get());
            }
          }
        },
        [&](const EvidenceNodeLocation &location) {
          auto &added = indicators.emplace_back(location.node->variable,
                                                location.evidence->second);
          contributions.emplace(&added);
        });
  }

  return std::make_unique<SubgraphFactor>(
      std::vector<const distribution::Distribution *>{contributions.begin(),
                                                      contributions.end()});
}

std::unique_ptr<distribution::Distribution>
QueryManager::getJointMarginalDistribution(
    const std::unordered_set<std::string> &subgroup,
    const std::size_t threads) {
  categoric::VariablesSet vars;
  for (const auto &var : subgroup) {
    vars.emplace(findVariable(var));
  }
  return getJointMarginalDistribution(vars, threads);
}
} // namespace EFG::strct
