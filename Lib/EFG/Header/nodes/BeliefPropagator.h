/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BELIEF_PROPAGATOR_H
#define EFG_NODES_BELIEF_PROPAGATOR_H

#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>

namespace EFG::nodes {
    class BeliefPropagator
        : virtual public EvidenceAware
        , virtual public BeliefAware {
    protected:
        void propagateBelief(const PropagationKind& kind) override;
    };
}

#endif
