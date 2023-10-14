/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/Cast.h>
#include <EasyFactorGraph/model/RandomField.h>

namespace EFG::model {
void RandomField::absorb(const strct::FactorsAware &to_absorb, bool copy) {
  castConstAndUse<strct::FactorsConstGetter>(
      to_absorb,
      [this, &copy](const strct::FactorsConstGetter &as_factor_aware) {
        const auto &factors = as_factor_aware.getConstFactors();
        this->absorbConstFactors(factors.begin(), factors.end(), copy);
      });
  castConstAndUse<train::FactorsTunableGetter>(
      to_absorb,
      [this,
       &copy](const train::FactorsTunableGetter &as_factor_tunable_aware) {
        this->absorbTunableClusters(as_factor_tunable_aware, copy);
      });
  for (const auto &[var, val] : to_absorb.getEvidences()) {
    setEvidence(var, val);
  }
}

std::vector<float> RandomField::getWeightsGradient_(
    const train::TrainSet::Iterator &train_set_combinations) {
  if (!getEvidences().empty()) {
    removeAllEvidences();
  }
  resetBelief();
  propagateBelief(strct::PropagationKind::SUM);
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
