/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/RandomField.h>

namespace EFG::model {
void RandomField::absorb(const strct::ConnectionsManager& to_absorb, const bool copy) {
    {
        const strct::FactorsAware* model =
            dynamic_cast<const strct::FactorsAware*>(&to_absorb);
        if (nullptr != model) {
            const auto& factors = model->getConstFactors();
            absorbConstFactors(factors.begin(), factors.end(), copy);
        }
    }
    {
        const train::FactorsTunableAware* model =
            dynamic_cast<const train::FactorsTunableAware*>(&to_absorb);
        if (nullptr != model) {
            absorbTunableClusters(*model, copy);
        }
    }
    for (const auto& [var, val] : to_absorb.getEvidences()) {
        setEvidence(var, val);
    }
}

std::vector<float> RandomField::getWeightsGradient_(
    const train::TrainSet::Iterator &train_set_combinations) {
  if (!getEvidences().empty()) {
    removeAllEvidences();
  }
  resetBelief();
  propagateBelief(strct::SUM);
  std::vector<float> result;
  result.resize(tuners.size());
  strct::Tasks tasks;
  std::size_t result_pos = 0;
  for (auto &tuner : tuners) {
    tasks.emplace_back([&receiver = result[result_pos], &tuner = tuner,
                        &train_set_combinations](const std::size_t) {
      receiver = tuner->getGradientAlpha(train_set_combinations) -
                 tuner->getGradientBeta();
    });
    ++result_pos;
  }
  getPool().parallelFor(tasks);
  return result;
}

} // namespace EFG::model
