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
                               virtual public strct::FactorsConstGetter,
                               protected strct::FactorsConstInserter,
                               virtual public train::FactorsTunableGetter,
                               protected train::FactorsTunableInserter,
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
  ConditionalRandomField(const RandomField &source, bool copy);

  /**
   * @brief Sets the new set of evidences.
   * @param the new set of evidence values. The variables order is the same of
   * the set obtained using getObservedVariables().
   * @throw the number of passed values does not match the number of evidences.
   * @throw in case some evidence values are inconsistent
   */
  void setEvidences(const std::vector<std::size_t> &values);

  /**
   * @brief Builds a training set for the conditioned model.
   * Instead of using Gibbs sampler for a single combination of evidence, it
   * tries to span all the possible combination of evidences and generate some
   * samples conditioned to each of this evidences value.
   * Then, gather results to build the training set.
   * Actually, not ALL possible evidence are spwan if that would be too much
   * computationally demanding. In such cases, simply pass a number lower than 1
   * as range_percentage.
   *
   * @param information used for samples generation
   * @param parameter handling how many evidence values are accounted for the
   * samples generation
   * @param the number of threads to use for speeding up the process
   */
  std::vector<std::vector<std::size_t>>
  makeTrainSet(const GibbsSampler::SamplesGenerationContext &context,
               float range_percentage = 1.f, std::size_t threads = 1);

protected:
  std::vector<float> getWeightsGradient_(
      const train::TrainSet::Iterator &train_set_combinations) final;

private:
  struct SourceStructure {
    const strct::FactorsConstGetter *factors_structure;
    const train::FactorsTunableGetter *factors_tunable_structure;
  };
  void absorb(const SourceStructure &source, bool copy);

  void replaceIfNeeded(train::TunerPtr &container,
                       const train::BaseTuner &subject);

  const std::vector<std::size_t> evidence_vars_positions;
};
} // namespace EFG::model
