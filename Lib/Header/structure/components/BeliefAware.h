/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_BELIEF_AWARE_H
#define EFG_STRUCTURE_BELIEF_AWARE_H

#include <structure/Node.h>
#include <Component.h>
#include <memory>

namespace EFG::strct {
    enum PropagationKind { Sum, MAP };

    struct PropagationResult {
        PropagationKind kindDone;
        //std::size_t iterationsRequired;
        bool wasTerminated;
    };

    class BeliefAware : virtual public Component {
    public:
        void setMaxIterationsLoopyPropagation(std::size_t iterations) { this->maxIterationsLoopyPropagtion = iterations; };
        inline std::size_t getMaxIterationsLoopyPropagation() const { return this->maxIterationsLoopyPropagtion; }

        inline PropagationResult getLastPropagationResult() const {return *this->lastPropagation.get(); };

    protected:
        virtual void propagateBelief(const PropagationKind& kind) = 0;

        /** 
         * @brief maximum number of iterations considered when doing loopy propagation
         */
        std::size_t maxIterationsLoopyPropagtion = 100;
        /**
         * @brief results about the last belief propagation done. It is a nullptr until the first propagation is triggered
         */
        std::unique_ptr<PropagationResult> lastPropagation;
    };
}

#endif
