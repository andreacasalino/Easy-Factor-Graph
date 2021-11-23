/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_ITERATIONS_AWARE_H
#define EFG_TRAINER_ITERATIONS_AWARE_H

#include <Component.h>
#include <cstdlib>

namespace EFG::train {
    class IterationsAware 
        : virtual public Component {
    public:
        /**
         * @param the maximum number of iterations allowed to search the optimum
         * @throw in case the passed number of interations is too low
         */
        void setMaxIterations(const std::size_t iter);

    protected:
        std::size_t maxIterations = 100;
        std::size_t doneIterations = 1;
    };
}

#endif
