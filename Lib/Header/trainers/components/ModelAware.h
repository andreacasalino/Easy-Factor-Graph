/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAINER_MODEL_AWARE_H
#define EFG_TRAINER_MODEL_AWARE_H

#include <train/Trainable.h>
#include <trainers/Commons.h>
#include <trainers/components/TrainSetAware.h>

namespace EFG::train {
class ModelAware : virtual public TrainSetAware {
protected:
  Trainable *model = nullptr;

  inline Vect getGradient() const {
    Vect direction = this->model->getGradient(this->getTrainSet());
    direction *= -1.f;
    return direction;
  };

  Vect lastWeights;
  Vect lastGrad;
};
} // namespace EFG::train

#endif
