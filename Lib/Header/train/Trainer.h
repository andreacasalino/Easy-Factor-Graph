/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINER_H
#define EFG_TRAIN_TRAINER_H

#include <train/Trainable.h>
#include <Component.h>

namespace EFG::train {
    class Trainer : virtual public Component {
    public:
        /**
         * @brief trains the passed model, using the passed training set
         * @param the model to train
         * @param the training set to use
         */
        virtual void train(Trainable& model, TrainSetPtr trainSet) = 0;
    };
}

#endif
