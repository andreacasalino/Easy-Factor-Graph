/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// #ifdef EFG_LEARNING_ENABLED

// #include <EasyFactorGraph/trainable/ModelTrainer.h>

// namespace EFG::train {
// namespace {
// class ModelWrapper : public train::ParametersAware {
// public:
//   ModelWrapper(FactorsTunableAware &subject,
//                const TrainSet::Iterator &train_set_combinations,
//                const std::size_t threads)
//       : subject(subject), train_set_combinations(train_set_combinations),
//         activator(subject.getPool(), threads) {}

//   train::Vect getParameters() const final;
//   void setParameters(const train::Vect &) final;

//   train::Vect getGradient() const final {
//     subject.getWeightsGradient_(train_set_combinations);
//   }

// private:
//   FactorsTunableAware &subject;
//   const TrainSet::Iterator &train_set_combinations;
//   strct::ScopedPoolActivator activator;
// };
// } // namespace

// void train(FactorsTunableAware &subject,
//            const TrainSet::Iterator &train_set_combinations,
//            train::Trainer &trainer, const std::size_t threads) {
//   // TODO open thread pool once here
//   throw 0;
//   ModelWrapper wrapper(subject, train_set_combinations, threads);
//   trainer.train(wrapper);
// }
// } // namespace EFG::train

// #endif
