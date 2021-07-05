/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_BASIC_TRAINSET_H
#define EFG_TRAINER_BASIC_TRAINSET_H

#include <trainers/components/TrainSetAware.h>

namespace EFG::train {
    class BasicTrainSet : public virtual TrainSetAware {
    protected:
        inline TrainSetPtr getTrainSet() const override { return this->completeTrainSet; };

        inline void resetTrainSet(TrainSetPtr trainSet) final { this->completeTrainSet = trainSet; };

        TrainSetPtr completeTrainSet;
    };
}

#endif
