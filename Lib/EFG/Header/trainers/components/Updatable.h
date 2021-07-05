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
    class Updatable : virtual public Component {
    protected:
        virtual void update() = 0;
        virtual void reset() = 0;
    };
}

#endif
