/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/QueryHandler.h>

#include "Utils.h"

namespace EFG::strct {
namespace {
distribution::UnaryFactor gather_incoming_messages(Node &subject) {
  std::vector<const distribution::Distribution *> messages;
  for (const auto &[connected_node, message] : subject.active_connections) {
    messages.push_back(message.message.get());
  }
  if (messages.empty()) {
    return distribution::UnaryFactor{subject.variable};
  }
  return distribution::UnaryFactor{messages};
}

std::vector<float> merged_values(const NodeLocation &location) {
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

std::vector<float> zeros(const std::size_t size) {
  std::vector<float> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(0);
  }
  return result;
}
} // namespace

std::vector<float>
QueryHandler::getMarginalDistribution(const std::string &var,
                                      const std::size_t threads) {
  {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(SUM);
  }
  return merged_values(*find_node(getGraphState_(), findVariable(var)));
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

std::size_t QueryHandler::getMAP(const std::string &var,
                                 const std::size_t threads) {
  {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(MAP);
  }
  auto values = merged_values(*find_node(getGraphState_(), findVariable(var)));
  return find_max(values);
}

std::vector<size_t> QueryHandler::getHiddenSetMAP(const std::size_t threads) {
  {
    ScopedPoolActivator activator(*this, threads);
    propagateBelief(MAP);
  }
  auto vars = getHiddenVariables();
  std::vector<size_t> result;
  result.reserve(vars.size());
  for (const auto &var : vars) {
    auto values = merged_values(*find_node(getGraphState_(), var));
    result.push_back(find_max(values));
  }
  return result;
}

// distribution::factor::cnst::Factor
// QueryHandler::getJointMarginalDistribution(
//     const std::set<std::string> &subgroup) {
//   std::set<Node *> hiddenGroup;
//   std::list<distribution::factor::cnst::IndicatorFactor> indicators;
//   std::for_each(subgroup.begin(), subgroup.end(), [&](const std::string
//   &name) {
//     auto itN = this->nodes.find(categoric::makeVariable(2, name));
//     if (itN == this->nodes.end()) {
//       throw Error("non existent variable");
//     }
//     auto itOb = this->evidences.find(itN->first);
//     if (itOb == this->evidences.end()) {
//       // hidden
//       hiddenGroup.emplace(&itN->second);
//     } else {
//       indicators.emplace_back(itN->first, itOb->second);
//     }
//   });

//   this->propagateBelief(PropagationKind::Sum);

//   // gather hidden variables factors
//   std::set<const distribution::Distribution *> toMerge;
//   {
//     std::for_each(hiddenGroup.begin(), hiddenGroup.end(),
//                   [&toMerge, &hiddenGroup](const Node *n) {
//                     gatherUnaries(toMerge, *n);
//                     for (auto c = n->activeConnections.begin();
//                          c != n->activeConnections.end(); ++c) {
//                       if (hiddenGroup.find(c->first) == hiddenGroup.end()) {
//                         // connection to node outside of the subgraph
//                         toMerge.emplace(c->second.message2This.get());
//                       } else {
//                         // connection to node inside of the subgraph
//                         toMerge.emplace(c->second.factor.get());
//                       }
//                     }
//                   });
//   }
//   std::for_each(
//       indicators.begin(), indicators.end(),
//       [&toMerge](const distribution::factor::cnst::IndicatorFactor &i) {
//         toMerge.emplace(&i);
//       });
//   return distribution::factor::cnst::Factor(toMerge);
// }
} // namespace EFG::strct
