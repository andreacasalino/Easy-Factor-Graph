/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_MODEL_AWARE_H
#define EFG_TRAINER_MODEL_AWARE_H

#include <train/Trainable.h>
#include <trainers/components/TrainSetAware.h>
#ifdef ADVANCED_TRAINERS_ENABLED
#include <trainers/Commons.h>
#endif

namespace EFG::train {
    class ModelAware 
        : virtual public TrainSetAware {
    protected:
        Trainable* model = nullptr;

        inline Vect getGradient() const { return -this->model->getGradient(this->getTrainSet()); };

#ifdef ADVANCED_TRAINERS_ENABLED
        Vect lastWeights;
        Vect lastGrad;
#endif
    };
}

#endif
