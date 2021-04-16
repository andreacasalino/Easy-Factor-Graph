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
    void train(Trainable& model, std::size_t maxIterations, TrainSetPtr trainset);
}

#endif
