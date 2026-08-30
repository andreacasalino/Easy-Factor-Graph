/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/BeliefManager.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/TunableWeightsManager.h>

#include <unordered_set>

namespace EFG::structure {
void TunableWeightsManager::init(StructurePtr context) {
  std::unordered_set<std::size_t> permanent_evidences_table{
      context->permanent_evidences.begin(), context->permanent_evidences.end()};

  context_ = context;
  context->addListener(listener_);
  for (const auto &info : context_->tunability.order) {
    std::visit(
        [&](const auto &info) {
          if constexpr (std::is_same_v<
                            decltype(info),
                            const Tunability::TunableUnaryFactor &>) {
            tuners_.emplace_back(UnaryTuner{context, info.var_index});
          } else if constexpr (std::is_same_v<
                                   decltype(info),
                                   const Tunability::TunableBinaryFactor &>) {
            auto [var_a_index, var_b_index] = info;
            const auto factor_index =
                context_->locateBinaryFactor(var_a_index, var_b_index);
            bool var_a_is_evidence =
                permanent_evidences_table.contains(var_a_index);
            bool var_b_is_evidence =
                permanent_evidences_table.contains(var_b_index);
            if (var_a_is_evidence && var_b_is_evidence) {
              throw Error{"Can't have a tunable factor connecting two "
                          "evidences, found for "
                          "variables at indices: {} and {}",
                          var_a_index, var_b_index};
            }
            if (var_a_is_evidence) {
              tuners_.emplace_back(
                  BinaryConditionedTuner<true>{context, factor_index});
            } else if (var_b_is_evidence) {
              tuners_.emplace_back(
                  BinaryConditionedTuner<false>{context, factor_index});
            } else {
              tuners_.emplace_back(BinaryTuner{context, factor_index});
            }
          }
        },
        info);
  }
}

void TunableWeightsManager::setTunableWeights(std::span<const float> values) {
  if (values.size() != tuners_.size()) {
    throw Error{"Invalid weights lenght"};
  }
  for (std::size_t t = 0; t < tuners_.size(); ++t) {
    std::visit([&](auto &tuner) { tuner.setWeight(values[t]); }, tuners_[t]);
  }
}

void TunableWeightsManager::getTunableWeights(
    std::vector<float> &recipient) const {
  recipient.clear();
  recipient.reserve(tuners_.size());
  for (const auto &tuner : tuners_) {
    auto val =
        std::visit([&](auto &tuner) { return tuner.getWeight(); }, tuner);
    recipient.push_back(val);
  }
}

TunableWeightsManager::TunableWeightsGradient::TunableWeightsGradient(
    TunableWeightsManager &source,
    std::shared_ptr<const misc::Samples> training_set)
    : source_{source}, training_set_{training_set} {
  setAlfa();
}

void TunableWeightsManager::TunableWeightsGradient::setAlfa() {
  alfa_part_.resize(source_.tuners_.size(), 0);

  float coeff = 1.f / static_cast<float>(training_set_->samplesCount());

  auto compute_ = [&](std::size_t th_id, std::size_t stride) {
    auto it = training_set_->makeIter();
    while (true) {
      auto maybe_sample = it.next();
      if (!maybe_sample.has_value()) {
        break;
      }
      for (std::size_t t = th_id; t < source_.tuners_.size(); t += stride) {
        float a = std::visit(
            [&](const auto &tuner) {
              return tuner.getActivation(*maybe_sample);
            },
            source_.tuners_[t]);
        alfa_part_[t] += coeff * a; // TODO isn't a false sharing ?
      }
    }
  };

  if (auto *pool = source_.listener_.getPool(); pool) {
    pool->compute([&, stride = pool->size()](std::size_t th_id) {
      compute_(th_id, stride);
    });
  } else {
    compute_(0, 1);
  }
}

void TunableWeightsManager::TunableWeightsGradient::setBeta() {
  beta_part_.resize(source_.tuners_.size(), 0);
  if (auto *pool = source_.listener_.getPool(); pool) {
    source_.cmpContext_.resize(pool->size());
  }

  auto compute__ = [&](std::size_t th_id, std::size_t stride, float coeff) {
    auto &ctxt = source_.cmpContext_[th_id];
    for (std::size_t t = th_id; t < source_.tuners_.size(); t += stride) {
      float val =
          std::visit([&](auto &tuner) { return tuner.getGradientBeta(ctxt); },
                     source_.tuners_[t]);
      beta_part_[t] += coeff * val;
    }
  };

  auto compute_ = [&](float coeff) {
    if (auto *pool = source_.listener_.getPool(); pool) {
      pool->compute([&, stride = pool->size()](std::size_t th_id) {
        compute__(th_id, stride, coeff);
      });
    } else {
      compute__(0, 1, coeff);
    }
  };

  if (source_.context_->permanent_evidences.empty()) {
    // random fields
    source_.context_->getManager<EvidenceRemoveManager>()->removeAllEvidences();
    source_.context_->getManager<BeliefManager>()
        ->template propagateBelief<PropagationKind::SUM>();
    compute_(1.f);
  }

  else {
    // conditional random fields
    auto samples_it = training_set_->makeIter();
    float coeff = 1.f / static_cast<float>(training_set_->samplesCount());
    while (true) {
      auto maybe_sample = samples_it.next();
      if (!maybe_sample.has_value()) {
        break;
      }

      // set evidence from sample and re-propagate belief
      auto *setter = source_.context_->getManager<EvidenceSetManager>();
      auto &ev = evidences_cache_.get_buffer();
      for (auto var_index : source_.context_->permanent_evidences) {
        ev.emplace_back(Evidence{var_index, (*maybe_sample)[var_index]});
      }
      setter->setEvidences(ev);
      source_.context_->getManager<BeliefManager>()
          ->template propagateBelief<PropagationKind::SUM>();

      compute_(coeff);
    }
  }
}

void TunableWeightsManager::TunableWeightsGradient::get(
    std::vector<float> &recipient) {
  setBeta();

  recipient.clear();
  recipient.reserve(source_.tuners_.size());
  for (std::size_t k = 0; k < alfa_part_.size(); ++k) {
    recipient.push_back(alfa_part_[k] - beta_part_[k]);
  }
}

float TunableWeightsManager::UnaryTuner::getGradientBeta(
    ComputationContext &ctx) {
  auto &merger = ctx.mergers[0];
  merger.reset(context_->nodes[var_index_].var_size);
  for (const auto &conn : context_->nodes[var_index_].incoming_messages) {
    merger.template merge<true>(context_->getMessageValues(conn));
  }
  merger.template merge_factor<true>(factor_);

  auto &prob = ctx.cache.get_buffer();
  factor::getProbabilities(
      factor::UnaryFactor{misc::Slot<float>::makeNonOwning(merger.getMerged())},
      prob);
  float res{0};
  auto values = factor_.getAllValues();
  for (std::size_t k = 0; k < prob.size(); ++k) {
    res += prob[k] * values[k];
  }
  return res;
}

std::span<float> TunableWeightsManager::BinaryTuner::gatherUnaries(
    factor::UnaryFactorsMerger &merger, std::size_t var_a, std::size_t var_b) {
  merger.reset(context_->nodes[var_a].var_size);
  for (const auto &conn : context_->nodes[var_a].incoming_messages) {
    if (conn.factor_info.sender_index == var_b) {
      continue;
    }
    merger.template merge<true>(context_->getMessageValues(conn));
  }
  use_factor_if(context_->nodes[var_a].unary_factor, [&](const auto &factor) {
    merger.template merge_factor<true>(factor);
  });
  return merger.getMerged();
}

float TunableWeightsManager::BinaryTuner::getGradientBeta(
    ComputationContext &ctx) {
  auto [var_a, var_b] = getVars();
  auto unaries_a = gatherUnaries(ctx.mergers[0], var_a, var_b);
  auto unaries_b = gatherUnaries(ctx.mergers[1], var_b, var_a);

  auto &prob = ctx.cache.get_buffer();
  factor_.forEachCombination<true>(
      [&](const categoric::Combination<2> &comb, float val) {
        prob.push_back(unaries_a[comb[0]] * unaries_b[comb[1]] * val);
      });
  float sum{0};
  for (auto val : prob) {
    sum += val;
  }
  for (auto &val : prob) {
    val /= sum;
  }

  float res{0};
  factor_.forEachCombination<false>(
      [&, index = 0](const auto &, float val) mutable {
        res += prob[index++] * val;
      });
  return res;
}

template <bool EvidenceIsFirst>
float TunableWeightsManager::BinaryConditionedTuner<
    EvidenceIsFirst>::getGradientBeta(ComputationContext &ctx) {
  auto &merger = ctx.mergers[0];

  Node *hidden_node = getHiddenNode();
  merger.reset(hidden_node->var_size);
  for (const auto &conn : hidden_node->incoming_messages) {
    merger.template merge<true>(context_->getMessageValues(conn));
  }

  auto &prob = ctx.cache.get_buffer();
  factor::getProbabilities(
      factor::UnaryFactor{misc::Slot<float>::makeNonOwning(merger.getMerged())},
      prob);

  auto evidence = getEvidence();

  float res{0};
  factor_.template forEachCombination<false>(
      [&](const categoric::Combination<2> &comb, float val) {
        if constexpr (EvidenceIsFirst) {
          if (comb[0] == evidence) {
            res += prob[comb[1]] * val;
          }
        } else {
          if (comb[1] == evidence) {
            res += prob[comb[0]] * val;
          }
        }
      });
  return res;
}

std::size_t
Trainer::train_model(TunableWeightsManager &model,
                     std::shared_ptr<const misc::Samples> training_set) const {
  std::vector<float> w_prev, w, w_grad;
  model.getTunableWeights(w_prev);
  auto gradient = model.gradient(training_set);
  std::size_t i = 0;
  for (; i < max_iterations_; ++i) {
    gradient.get(w_grad);
    w = w_prev;
    float adv_prctg{0};
    for (std::size_t k = 0; k < w.size(); ++k) {
      w[k] += gradient_rescale_ * w_grad[k];
      adv_prctg = std::max<float>(adv_prctg,
                                  std::abs(w[k] - w_prev[k]) / std::abs(w[k]));
    }
    if (adv_prctg < advance_toll_percentage_) {
      break;
    }
  }
  return i;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template float
TunableWeightsManager::BinaryConditionedTuner<true>::getGradientBeta(
    ComputationContext &);
template float
TunableWeightsManager::BinaryConditionedTuner<false>::getGradientBeta(
    ComputationContext &);
} // namespace EFG::structure
