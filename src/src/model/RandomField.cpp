/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/RandomField.h>

namespace EFG::model {
std::vector<float> RandomField::getWeightsGradient_(
    const train::TrainSet::Iterator &train_set_combinations) {
  if (!getEvidences().empty()) {
    removeAllEvidences();
  }
  propagateBelief(strct::SUM);
  std::vector<float> result;
  result.resize(tuners.size());
  strct::Tasks tasks;
  std::size_t result_pos = 0;
  for (auto &tuner : tuners) {
    tasks.emplace_back([&receiver = result[result_pos], &tuner = tuner,
                        &train_set_combinations](const std::size_t) {
      tuner->setTrainSetIterator(train_set_combinations);
      receiver = tuner->getGradientAlpha() - tuner->getGradientBeta();
    });
    ++result_pos;
  }
  getPool().parallelFor(tasks);
  return result;
}

} // namespace EFG::model
