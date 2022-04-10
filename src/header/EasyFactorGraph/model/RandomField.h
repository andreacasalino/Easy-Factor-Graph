/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::model {
class RandomField : public strct::EvidenceSetter,
                    public strct::EvidenceRemover,
                    public strct::FactorsAdder,
                    public train::FactorsTunableAdder,
                    public strct::GibbsSampler,
                    public strct::QueryManager {
public:
  RandomField() = default;

  RandomField(const RandomField &o) { absorb(o, true); };
  RandomField &operator=(const RandomField &) = delete;

  template <typename Model>
  void absorb(const Model &to_absorb, const bool copy) {
    {
      const strct::FactorsAware *model =
          dynamic_cast<const strct::FactorsAware *>(&to_absorb);
      if (nullptr != model) {
        const auto &factors = model->getConstFactors();
        absorbConstFactors(factors.begin(), factors.end(), copy);
      }
    }
    {
      const train::FactorsTunableAware *model =
          dynamic_cast<const train::FactorsTunableAware *>(&to_absorb);
      if (nullptr != model) {
        absorbTunableClusters(*model, copy);
      }
    }
  }

protected:
  std::vector<float> getWeightsGradient_(
      const train::TrainSet::Iterator &train_set_combinations) final;
};
} // namespace EFG::model
