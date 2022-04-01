/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsAdder.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::model {
class RandomField : public strct::EvidenceSetter,
                    public strct::EvidenceRemover,
                    public strct::FactorsAdder,
                    public strct::FactorsTunableAdder,
                    public strct::GibbsSampler,
                    public strct::QueryManager {
public:
  RandomField() = default;

  //   template <typename Model> explicit RandomField(const Model &o) {
  //     this->absorbModel(o);
  //   };

  //   RandomField(const RandomField &o) { this->absorbModel(o, true); };
  RandomField &operator=(const RandomField &) = delete;

  std::vector<float>
  getWeightsGradient(const TrainSet::Iterator &train_set_combinations) final;
};
} // namespace EFG::model
