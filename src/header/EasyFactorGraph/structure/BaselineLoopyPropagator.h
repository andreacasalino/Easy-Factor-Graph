/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>

namespace EFG::strct {
class BaselineLoopyPropagator : public LoopyBeliefPropagationStrategy {
public:
  bool propagateBelief(HiddenCluster &subject, const PropagationKind &kind,
                       const PropagationContext &context, Pool &pool) final;
};
} // namespace EFG::strct
