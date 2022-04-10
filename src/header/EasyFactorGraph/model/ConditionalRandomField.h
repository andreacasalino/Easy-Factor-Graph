/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::model {
class ConditionalRandomField : protected strct::EvidenceSetter,
                               protected strct::EvidenceRemover,
                               virtual public strct::FactorsAware,
                               protected strct::FactorsAdder,
                               virtual public train::FactorsTunableAware,
                               protected train::FactorsTunableAdder,
                               public strct::GibbsSampler,
                               public strct::QueryManager {
public:
  ConditionalRandomField() = delete;

  ConditionalRandomField(const ConditionalRandomField &o);
  ConditionalRandomField &operator=(const ConditionalRandomField &) = delete;

  // evidences are deduced from source, and in case there are no, an exception
  // is thrown
  ConditionalRandomField(const RandomField &source, const bool copy);

  void setEvidences(const std::vector<std::size_t> &values);

protected:
  std::vector<float> getWeightsGradient_(
      const train::TrainSet::Iterator &train_set_combinations) final;

private:
  struct SourceStructure {
    const strct::FactorsAware *factors_structure;
    const train::FactorsTunableAware *factors_tunable_structure;
  };
  void absorb(const SourceStructure &source, const bool copy);

  void replaceIfNeeded(train::TunerPtr &container,
                       const train::BaseTuner &subject);

  const std::vector<std::size_t> evidence_vars_positions;
};
} // namespace EFG::model
