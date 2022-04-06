/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/RandomField.h>

namespace EFG::model {
std::vector<float> RandomField::getWeightsGradient(
    const train::TrainSet::Iterator &train_set_combinations) {
  throw 0; // decide what to do to set the pool
  if (!getEvidences().empty()) {
    removeAllEvidences();
  }
  propagateBelief(strct::SUM);
  std::vector<float> result;
  strct::Tasks tasks;
  result.reserve(tuners.size());
  for (auto &tuner : tuners) {
    auto &receiver = result.emplace_back();
    tasks.emplace_back([&receiver = receiver, &tuner = tuner,
                        &train_set_combinations](const std::size_t) {
      tuner->setTrainSetIterator(train_set_combinations);
      receiver = tuner->getGradientAlpha() - tuner->getGradientBeta();
    });
  }
  getPool().parallelFor(tasks);
  return result;
}

} // namespace EFG::model
