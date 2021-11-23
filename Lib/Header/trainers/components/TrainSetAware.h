/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_TRAINSET_AWARE_H
#define EFG_TRAINER_TRAINSET_AWARE_H

#include <train/TrainSet.h>
#include <Component.h>

namespace EFG::train {
    class TrainSetAware : public virtual Component {
    protected:
        virtual TrainSetPtr getTrainSet() const = 0;

        /**
         * @param the new training set to use
         */
        virtual void resetTrainSet(TrainSetPtr trainSet) = 0;
    };
}

#endif
