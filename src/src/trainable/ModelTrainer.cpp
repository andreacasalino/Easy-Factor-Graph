/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_LEARNING_ENABLED

#include <EasyFactorGraph/trainable/ModelTrainer.h>
#include <TrainingTools/ParametersAware.h>

namespace EFG::train {
namespace {
::train::Vect to_Vect(const std::vector<float> &subject) {
  ::train::Vect result(subject.size());
  for (std::size_t k = 0; k < subject.size(); ++k) {
    result(static_cast<Eigen::Index>(k)) = static_cast<double>(subject[k]);
  }
  return result;
}

std::vector<float> to_vector(const ::train::Vect &subject) {
  std::vector<float> result;
  result.resize(subject.size());
  for (Eigen::Index k = 0; k < subject.size(); ++k) {
    result[static_cast<std::size_t>(k)] = static_cast<float>(subject(k));
  }
  return result;
}

struct TrainSetWrapper {
  TrainSetWrapper(const TrainSet &source, const float percentage)
      : source(source), percentage(percentage) {
    combinations = std::make_unique<TrainSet::Iterator>(source, 1.f);
  }

  const TrainSet &source;
  float percentage;

  mutable std::unique_ptr<TrainSet::Iterator> combinations;

  const TrainSet::Iterator &get() const {
    if (1.f != percentage) {
      combinations = std::make_unique<TrainSet::Iterator>(source, percentage);
    }
    return *combinations;
  }
};
} // namespace

class FactorsTunableAware::ModelWrapper : public ::train::ParametersAware {
public:
  ModelWrapper(FactorsTunableAware &subject, const TrainSet &train_set,
               const TrainInfo &info)
      : subject(subject),
        train_set(TrainSetWrapper{train_set, info.stochastic_percentage}),
        activator(subject, info.threads) {}

  ::train::Vect getParameters() const final {
    return to_Vect(subject.getWeights());
  };
  void setParameters(const ::train::Vect &w) final {
    subject.setWeights(to_vector(w));
  }

  ::train::Vect getGradient() const final {
    return -to_Vect(subject.getWeightsGradient_(train_set.get()));
  }

private:
  FactorsTunableAware &subject;
  TrainSetWrapper train_set;
  strct::PoolAware::ScopedPoolActivator activator;
};

void train_model(FactorsTunableAware &subject, ::train::Trainer &trainer,
                 const TrainSet &train_set, const TrainInfo &info) {
  FactorsTunableAdder::ModelWrapper wrapper(subject, train_set, info);
  trainer.train(wrapper);
}

} // namespace EFG::train

#endif
