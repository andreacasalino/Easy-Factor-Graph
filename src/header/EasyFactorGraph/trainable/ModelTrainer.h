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
  std::size_t threads = 1;
  float stochastic_percentage =
      1.f; // 1 means actually to use all the train set
};

void train_model(FactorsTunableAware &subject, ::train::Trainer &trainer,
                 const TrainSet &train_set,
                 const TrainInfo &info = TrainInfo{});

} // namespace EFG::train

#endif
