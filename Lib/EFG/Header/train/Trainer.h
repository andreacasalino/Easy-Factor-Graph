/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINER_H
#define EFG_TRAIN_TRAINER_H

#include <train/Trainable.h>

namespace EFG::train {
    class Trainer {
    public:
        virtual void train(Trainable& model, TrainSetPtr trainSet) = 0;

        std::size_t getMaxIterations() const { return this->maxIterations; };
        void setMaxIterations(std::size_t iter);

    protected:
        Trainer() = default;

        std::size_t maxIterations = 100;
    };
}

#endif
