/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ConnectionsAware.h>
#include <EasyFactorGraph/structure/Pool.h>

namespace EFG::strct {
class BeliefPropagator : virtual public ConnectionsAware,
                         virtual public PoolAware {
protected:
  PropagationResult propagateBelief(const PropagationKind &kind) override;
};
} // namespace EFG::strct
