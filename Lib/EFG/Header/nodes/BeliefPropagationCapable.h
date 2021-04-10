/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BELIEF_PROPAGATION_CAPABLE_H
#define EFG_NODES_BELIEF_PROPAGATION_CAPABLE_H

#include <cstdlib>

namespace EFG::nodes {
    class BeliefPropagationCapable {
    public:
        void setPropagationMaxIterations(std::size_t iterations) { this->maxPropagationIterations = iterations; };
        inline std::size_t getPropagationMaxIterations() const { return this->maxPropagationIterations; }

    protected:
        BeliefPropagationCapable() = default;

        enum PropagationKind { Sum, MAP };
        virtual void propagateBelief(const PropagationKind& kind) = 0;

        std::size_t maxPropagationIterations = 1000;
    };
}

#endif
