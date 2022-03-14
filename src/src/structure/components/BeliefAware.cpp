/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/components/BeliefAware.h>

namespace EFG::strct {
void BeliefAware::lazyBeliefPropagation(const PropagationKind &kind) {
  if ((lastPropagation != nullptr) &&
      (lastPropagation->propagation_kind_done == kind)) {
    return;
  }
  this->lastPropagation =
      std::make_unique<PropagationResult>(propagateBelief(kind));
}
} // namespace EFG::strct
