/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#ifndef EFG_TRAIN_YUNDA_SEARCHER_H
#define EFG_TRAIN_YUNDA_SEARCHER_H

#include <trainers/components/LineSearcher.h>
#include <trainers/components/IterationsAware.h>

namespace EFG::train {
    class YundaSearcher
        : public LineSearcher
        , virtual public IterationsAware {
    protected:
        void minimize(const Vect& direction) override;

        float computeC1() const;
    };
}

#endif
#endif
