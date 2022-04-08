/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/BaselineBeliefPropagator.h>
#include <EasyFactorGraph/structure/components/BeliefAware.h>

namespace EFG::strct {
BeliefAware::BeliefAware() {
  propagator = std::make_unique<BaselineBeliefPropagator>();
}

void BeliefAware::setPropagationStrategy(BeliePropagationStrategyPtr strategy) {
  if (nullptr == strategy) {
    throw Error{"trying ot set null propagator"};
  }
  propagator = std::move(strategy);
}

bool BeliefAware::wouldNeedPropagation(const PropagationKind &kind) const {
  return (lastPropagation == nullptr) ||
         (lastPropagation->propagation_kind_done != kind);
}

void BeliefAware::propagateBelief(const PropagationKind &kind) {
  if (wouldNeedPropagation(kind)) {
    this->lastPropagation =
        std::make_unique<PropagationResult>(this->propagator->propagateBelief(
            getState_().clusters, kind, context, getPool()));
  }
}
} // namespace EFG::strct
