/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BELIEF_PROPAGATOR_H
#define EFG_NODES_BELIEF_PROPAGATOR_H

#include <nodes/NodesContainer.h>
#include <nodes/BeliefPropagationCapable.h>

namespace EFG::nodes {
    class BeliefPropagator
        : virtual public NodesContainer
        , virtual public BeliefPropagationCapable {
    protected:
        void propagateBelief(const PropagationKind& kind) override;
    };
}

#endif
