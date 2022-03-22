/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceAware.h>

#include <memory>

namespace EFG::strct {
class BeliefPropagator : virtual public EvidenceAware {
protected:
  PropagationResult propagateBelief(const PropagationKind &kind) override;
};
} // namespace EFG::strct
