/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_LEARNING_ENABLED

#pragma once

#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <TrainingTools/Trainer.h>

namespace EFG::train {
struct TrainInfo {
  /**
   * @brief Number of threads to use for the training procedure.
   */
  std::size_t threads = 1;
  /**
   * @brief 1 means actually to use all the train set, adopting a classical
   * gradient based approach. A lower value implies to a stochastic gradient
   * based approach.
   */
  float stochastic_percentage = 1.f;
};

/**
 * @param the model to tune
 * @param the training approach to adopt
 * @param the train set to use
 */
void train_model(FactorsTunableAware &subject, ::train::Trainer &trainer,
                 const TrainSet &train_set,
                 const TrainInfo &info = TrainInfo{});

} // namespace EFG::train

#endif
