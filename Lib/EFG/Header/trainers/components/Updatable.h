/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_UPDATABLE_H
#define EFG_TRAINER_UPDATABLE_H

#include <Component.h>

namespace EFG::train {
    /**
     * @brief an object that is updated at every iteration
     */
    class Updatable : virtual public Component {
    protected:
        /**
         * @brief called at every iteration
         */
        virtual void update() = 0;
        /**
         * @brief called before starting a new training process to clean up
         */
        virtual void reset() = 0;
    };
}

#endif
