/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/tuners/Tuner.h>

#include <optional>

namespace EFG::train {
using FactorExponentialPtr = std::shared_ptr<distribution::FactorExponential>;

class FactorsTunableAware : virtual public strct::ConnectionsManager {
public:
  // comment that belief propagation will be not invalidate if weights are
  // changed outside
  const std::unordered_set<FactorExponentialPtr> &getTunableFactors() const {
    return tunable_factors;
  }

  std::vector<float> getWeights() const;
  void setWeights(const std::vector<float> &weights);

  virtual std::vector<float>
  getWeightsGradient(const TrainSet::Iterator &train_set_combinations) = 0;

protected:
  std::unordered_set<FactorExponentialPtr> tunable_factors;
  std::vector<TunerPtr> tuners;
};

class FactorsTunableAdder : virtual public FactorsTunableAware {
public:
  void addTunableFactor(const FactorExponentialPtr &factor,
                        const std::optional<categoric::VariablesSet>
                            &group_sharing_weight = std::nullopt);

  void copyTunableFactor(const distribution::FactorExponential &factor,
                         const std::optional<categoric::VariablesSet>
                             &group_sharing_weight = std::nullopt);

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

protected:
  virtual TunerPtr makeTuner(const FactorExponentialPtr &factor);

  void
  addTuner(train::TunerPtr tuner,
           const std::optional<categoric::VariablesSet> &group_sharing_weight);
};

/**
 * @param sets equal to 1 the weight of all the tunable clusters
 */
void set_ones(FactorsTunableAware &subject);
} // namespace EFG::train
