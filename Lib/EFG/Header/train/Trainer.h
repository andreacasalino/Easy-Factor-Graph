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
    class GradientDescend {
    public:
        GradientDescend() = default;

        void train(train::Trainable& model, train::TrainSetPtr trainSet) const;

        void setAdvancement(float coeff) { this->advancementCoefficient = coeff; };
        void setMaxIterations(std::size_t iterations) { this->maxIterations = iterations; };

    private:
        float advancementCoefficient = 0.05f;
        std::size_t maxIterations = 1000;

    };
}

#endif
