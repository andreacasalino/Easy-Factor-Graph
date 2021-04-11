/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BELIEF_AWARE_H
#define EFG_NODES_BELIEF_AWARE_H

#include <nodes/Node.h>
#include <nodes/bases/Base.h>

namespace EFG::nodes {
    enum PropagationResultInfo { NotDone, Sum, MAP };

    struct PropagationResult {
        PropagationResultInfo kind = PropagationResultInfo::NotDone;
        std::size_t iterations = 0;
        bool terminated = true;
    };

    class BeliefAware : virtual public Base {
    public:
        void setPropagationMaxIterations(std::size_t iterations) { this->maxPropagationIterations = iterations; };
        inline std::size_t getPropagationMaxIterations() const { return this->maxPropagationIterations; }

        inline PropagationResult getLastPropagationResult() const { return this->lastPropagation; };

    protected:
        enum PropagationKind { Sum, MAP };
        virtual void propagateBelief(const PropagationKind& kind) = 0;

        std::size_t maxPropagationIterations = 1000;
        PropagationResult lastPropagation;
    };
}

#endif
