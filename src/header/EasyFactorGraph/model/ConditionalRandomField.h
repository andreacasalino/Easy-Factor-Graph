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
/**
 * @brief Similar to RandomField, with the difference that the model structure
 * is immutable after construction. This applies also to the evidence set, which
 * can't be changed over the time.
 */
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

  /**
   * @brief All the factors of the passed source are inserted/copied. The
   * evidence set is deduced by the passed source.
   * @param the model to emulate for building the structure of this one.
   * @param then passing true the factors are deep copied, while in the contrary
   * case the smart pointers storing the factors of the source are copied and
   * inserted.
   * @throw in case the passed source has no evidences
   */
  ConditionalRandomField(const RandomField &source, const bool copy);

  /**
   * @brief Sets the new set of evidences.
   * @param the new set of evidence values. The variables order is the same of
   * the set obtained using getObservedVariables().
   * @throw the number of passed values does not match the number of evidences.
   * @throw in case some evidence values are inconsistent
   */
  void setEvidences(const std::vector<std::size_t> &values);

  // TODO comment
  std::vector<categoric::Combination>
  makeTrainSet(const GibbsSampler::SamplesGenerationContext &context,
               const float range_percentage = 1.f,
               const std::size_t threads = 1);

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
