/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#ifndef EFG_TRAINER_LINE_SEARCHER_H
#define EFG_TRAINER_LINE_SEARCHER_H

#include <trainers/components/ModelAware.h>
#include <trainers/Commons.h>

namespace EFG::train {
    class LineSearcher
        : public virtual ModelAware {
    protected:
        virtual void minimize(const Vect& direction) = 0;
    };
}

#endif
#endif
