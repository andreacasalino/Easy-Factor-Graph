/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/Samples.h>
#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/misc/WorkerPool.h>
#include <EasyFactorGraph/structure/Structure.h>

#include <memory>

namespace EFG::structure {
class TunableWeightsManager {
  // TODO shall align as a cache line ?
  struct ComputationContext {
    std::array<factor::UnaryFactorsMerger, 2> mergers;
    misc::VectorCache<float> cache;
  };

public:
  struct TunableWeightsGradient {
    TunableWeightsGradient(TunableWeightsManager &source,
                           std::shared_ptr<const misc::Samples> training_set);

    void get(std::vector<float> &recipient);

  private:
    TunableWeightsManager &source_;
    std::shared_ptr<const misc::Samples> training_set_;

    void setAlfa();
    std::vector<float> alfa_part_;

    void setBeta();
    std::vector<float> beta_part_;

    misc::VectorCache<Evidence> evidences_cache_;
  };
  TunableWeightsGradient
  gradient(std::shared_ptr<const misc::Samples> training_set) {
    return {*this, training_set};
  }

  void setTunableWeights(const std::vector<float> &values) {
    setTunableWeights(std::span<const float>{values.begin(), values.end()});
  }

  void setTunableWeights(std::span<const float> values);

  void getTunableWeights(std::vector<float> &recipient) const;

protected:
  void init(StructurePtr context);

private:
  template <std::size_t N> struct BaseTuner {
    void setWeight(float w) { factor_.trsfm.setWeight(w); }

    float getWeight() const { return factor_.trsfm.getWeight(); }

  protected:
    template <typename F>
    BaseTuner(F &factor)
        : factor_{std::get<factor::FactorT<N, factor::ExponentialTrasform>>(
              factor)} {}

    factor::FactorT<N, factor::ExponentialTrasform> &factor_;
  };

  struct UnaryTuner : BaseTuner<1> {
    UnaryTuner(StructurePtr context, std::size_t var_index)
        : BaseTuner<1>{context->nodes[var_index].unary_factor},
          context_{context}, var_index_{var_index} {}

    float getActivation(
        const std::span<const categoric::VarStateSize> &sample) const {
      std::size_t val = sample[var_index_];
      return factor_.getAllValues()[val];
    }

    float getGradientBeta(ComputationContext &ctx);

  private:
    StructurePtr context_;
    std::size_t var_index_;
  };

  struct BinaryTuner : BaseTuner<2> {
    BinaryTuner(StructurePtr context, std::size_t factor_index)
        : BaseTuner<2>{context->binary_factors[factor_index].factor},
          context_{context}, factor_index_{factor_index} {}

    float getActivation(
        const std::span<const categoric::VarStateSize> &sample) const {
      auto vars = getVars();
      return factor_.get<false>({sample[vars.first], sample[vars.second]});
    }

    float getGradientBeta(ComputationContext &ctx);

  protected:
    const std::pair<std::size_t, std::size_t> &getVars() const {
      return context_->binary_factors[factor_index_].vars_indices;
    }

    std::span<float> gatherUnaries(factor::UnaryFactorsMerger &merger,
                                   std::size_t var_a, std::size_t var_b);

    StructurePtr context_;
    std::size_t factor_index_;
  };

  template <bool EvidenceIsFirst> struct BinaryConditionedTuner : BinaryTuner {
    using BinaryTuner::BinaryTuner;

    float getGradientBeta(ComputationContext &ctx);

  private:
    Node *getHiddenNode() {
      if constexpr (EvidenceIsFirst) {
        return &context_->nodes[getVars().second];
      } else {
        return &context_->nodes[getVars().first];
      }
    }

    categoric::VarStateSize getEvidence() const {
      if constexpr (EvidenceIsFirst) {
        return context_->nodes[getVars().first].evidence;
      } else {
        return context_->nodes[getVars().second].evidence;
      }
    }
  };

  using Tuner =
      std::variant<UnaryTuner, BinaryTuner, BinaryConditionedTuner<true>,
                   BinaryConditionedTuner<false>>;
  std::vector<Tuner> tuners_;

  StructurePtr context_;
  WorkerPoolActivityListener listener_;
  std::vector<ComputationContext> cmpContext_{ComputationContext{}};
};

// Simple gradient descend.
//
// More sophisticated ones are available at
// https://github.com/andreacasalino/TrainingTools.git
struct Trainer {
  Trainer() = default;

  Trainer &max_iterations(std::size_t val) {
    max_iterations_ = val;
    return *this;
  }
  Trainer &gradient_rescale(float val) {
    gradient_rescale_ = val;
    return *this;
  }
  Trainer &advance_toll_percentage(float val) {
    advance_toll_percentage_ = val;
    return *this;
  }

  struct NoCB {};

  /**
   * @return the number of actually performed iterations
   *
   */
  template <typename CB>
  std::size_t
  train_model_with_cb(TunableWeightsManager &model,
                      std::shared_ptr<const misc::Samples> training_set,
                      CB cb) const {
    std::vector<float> w, w_grad;
    auto gradient = model.gradient(training_set);
    std::size_t i = 0;
    for (; i < max_iterations_; ++i) {
      model.getTunableWeights(w);
      gradient.get(w_grad);
      float adv_prctg{0};
      for (std::size_t k = 0; k < w.size(); ++k) {
        float w_snap = w[k];
        float advancement = gradient_rescale_ * w_grad[k];
        w[k] += advancement;
        adv_prctg = std::max<float>(adv_prctg,
                                    std::abs(advancement) / std::abs(w_snap));
      }
      if (adv_prctg < advance_toll_percentage_) {
        break;
      }
      model.setTunableWeights(w);
      if constexpr (!std::is_same_v<CB, NoCB>) {
        cb(w);
      }
    }
    return i;
  }

  std::size_t
  train_model(TunableWeightsManager &model,
              std::shared_ptr<const misc::Samples> training_set) const {
    return train_model_with_cb<NoCB>(model, training_set, NoCB{});
  }

private:
  std::size_t max_iterations_{1000};
  float gradient_rescale_{0.01f};
  float advance_toll_percentage_{1e-5};
};
} // namespace EFG::structure
