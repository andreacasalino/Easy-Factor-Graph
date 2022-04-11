/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/tuners/Tuner.h>

#include <optional>

namespace EFG::train {
class FactorsTunableAware : virtual public strct::ConnectionsManager {
public:
  /**
   * @brief Get the collections of tunable exponential factors.
   */
  const std::unordered_set<FactorExponentialPtr> &getTunableFactors() const {
    return tunable_factors;
  }

  /**
   * @brief Get the clusters of tunable exponential factors. Elements in the
   * same cluster, shares the weight.
   */
  std::vector<std::vector<FactorExponentialPtr>> getTunableClusters() const;

  /**
   * @return the weights of all the tunable factors that are part of the model.
   * The same order assumed by getTunableClusters() is assumed.
   */
  std::vector<float> getWeights() const;
  /**
   * @return sets the weights to use for of all the tunable factors that are
   * part of the model. The same order assumed by getTunableClusters() should be
   * assumed.
   * @throw in case the number of specified weights is inconsistent
   */
  void setWeights(const std::vector<float> &weights);

  /**
   * @return the gradients of the weights of all the tunable factors that are
   * part of the model, w.r.t a certain training set. The same order assumed by
   * getTunableClusters() is assumed.
   * @param the training set to use
   * @param the number of threads to use for the gradient computation
   */
  std::vector<float>
  getWeightsGradient(const TrainSet::Iterator &train_set_combinations,
                     const std::size_t threads = 1);

  class ModelWrapper;

protected:
  virtual std::vector<float>
  getWeightsGradient_(const TrainSet::Iterator &train_set_combinations) = 0;

  std::unordered_set<FactorExponentialPtr> tunable_factors;
  Tuners tuners;
};

class FactorsTunableAdder : virtual public FactorsTunableAware {
public:
  /**
   * @brief add a shallow copy of the passed tunable expoenential factor to this
   * model.
   * @param the factor to insert
   * @param an optional group of variables specifying the tunable factor that
   * should share the weight with the one to insert. When passing a nullopt the
   * factor will be inserted without sharing its weight.
   */
  void addTunableFactor(const FactorExponentialPtr &factor,
                        const std::optional<categoric::VariablesSet>
                            &group_sharing_weight = std::nullopt);

  /**
   * @brief add a deep copy of the passed tunable expoenential factor to this
   * model.
   * @param the factor to insert
   * @param an optional group of variables specifying the tunable factor that
   * should share the weight with the one to insert. When passing a nullopt the
   * factor will be inserted without sharing its weight.
   */
  void copyTunableFactor(const distribution::FactorExponential &factor,
                         const std::optional<categoric::VariablesSet>
                             &group_sharing_weight = std::nullopt);

  /**
   * @brief adds a collection of tunable expoenential factors ot this model.
   * Passing copy = true, deep copies are created and inserted in this model.
   * Passing copy = false, shallow copies are created and inserted in this
   * model.
   */
  template <typename FactorExponentialIt>
  void absorbTunableFactors(const FactorExponentialIt &begin,
                            const FactorExponentialIt &end, const bool copy) {
    for (auto it = begin; it != end; ++it) {
      try {
        if (copy) {
          copyTunableFactor(**it);
        } else {
          addTunableFactor(*it);
        }
      } catch (...) {
      }
    }
  }

  /**
   * @brief adds a collection of tunable expoenential factors ot this model,
   * preserving the fact that elements in the same cluster should share the
   * weight.
   * Passing copy = true, deep copies are created and inserted in this model.
   * Passing copy = false, shallow copies are created and inserted in this
   * model.
   */
  void absorbTunableClusters(const FactorsTunableAware &source,
                             const bool copy);

protected:
  TunerPtr &findTuner(const categoric::VariablesSet &tuned_vars_group);

private:
  TunerPtr makeTuner(const FactorExponentialPtr &factor);
};

/**
 * @param sets equal to 1 the weight of all the tunable clusters
 */
void set_ones(FactorsTunableAware &subject);
} // namespace EFG::train
