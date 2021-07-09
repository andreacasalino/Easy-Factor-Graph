/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_STOCHASTIC_TRAINSET_H
#define EFG_TRAINER_STOCHASTIC_TRAINSET_H

#include <trainers/strategies/BasicTrainSet.h>

namespace EFG::train {
    class StochasticTrainSet : public BasicTrainSet {
    public:
        inline void setPercentage(const float& percentage) { this->percentage = percentage; };

    protected:
        inline TrainSetPtr getTrainSet() const override { return std::make_shared<TrainSet>(this->completeTrainSet->getRandomSubSet(this->percentage)); };

    private:
        float percentage = 0.1f;
    };
}

#endif
