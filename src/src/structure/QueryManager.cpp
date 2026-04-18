/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/structure/QueryManager.h>

#include <unordered_map>

namespace EFG::structure {
namespace {
class JointDistributionBuilder {
public:
  JointDistributionBuilder(StructurePtr model) : model_{model} {}

  void compute(std::vector<float> &recipient,
               std::span<const std::size_t> vars) {
    cache_.vars_positions_in_joint_combination.clear();
    std::size_t pos{0};
    for (auto var : vars) {
      cache_.vars_positions_in_joint_combination.emplace(var, pos);
      pos += 1;
    }

    auto &contributions = cache_.template get_buffer<0>();

    auto add_unary_ = [&](auto factor, std::size_t var_pos_in_comb) {
      auto &added = contributions.emplace_back().emplace<UnaryContribution>();
      added.var_pos_in_comb = var_pos_in_comb;
      added.factor = factor;
    };

    auto add_binary_ = [&](const auto &factor, std::size_t var_a_pos_in_comb,
                           std::size_t var_b_pos_in_comb) {
      auto &added = contributions.emplace_back().emplace<BinaryContribution>();
      added.var_first_pos_in_comb = var_a_pos_in_comb;
      added.var_second_pos_in_comb = var_b_pos_in_comb;
      added.factor = &factor;
    };

    auto &structure = *model_;

    // load all contributions
    pos = 0;
    for (auto var : vars) {
      const auto &node = structure.nodes[var];
      if (node.evidence == Evidence::NOT_AN_EVIDENCE) {
        std::visit(
            [&](const auto &factor) {
              if constexpr (!std::is_same_v<decltype(factor),
                                            const std::monostate &>) {
                add_unary_(&factor, pos);
              }
            },
            node.unary_factor);

        for (const auto &connection : structure.nodes[var].incoming_messages) {
          auto var_o = connection.factor_info.sender_index;
          auto it = cache_.vars_positions_in_joint_combination.find(var_o);
          if (structure.nodes[var_o].evidence != Evidence::NOT_AN_EVIDENCE ||
              it == cache_.vars_positions_in_joint_combination.end()) {
            auto connection_message = structure.getMessageValues(connection);
            add_unary_(connection_message, pos);
          } else if (connection.factor_info.receiver_is_first_in_factor) {
            // insert binary factor involving two vars that are part of the
            // joint set only once
            std::visit(
                [&](const auto &factor) {
                  add_binary_(factor, pos, it->second);
                },
                structure.binary_factors[connection.factor_info.factor_index]
                    .factor);
          }
        }
      } else {
        contributions.emplace_back(EvidenceContribution{var, node.evidence});
      }
      pos += 1;
    }

    // build the joint distribution
    auto &group_sizes = cache_.template get_buffer<1>();
    group_sizes.reserve(vars.size());
    for (auto var_index : vars) {
      group_sizes.push_back(structure.nodes[var_index].var_size);
    }
    recipient.clear();
    float sum{0};
    misc::for_each_generated<const std::vector<categoric::VarStateSize> *>(
        categoric::RangeVec{std::move(group_sizes)},
        [&](const std::vector<categoric::VarStateSize> *comb) {
          float val = 1.f;
          for (const auto &contribution : contributions) {
            std::visit(
                [&](const auto &contribution) {
                  val *= contribution.evaluate(*comb);
                },
                contribution);
          }
          sum += val;
          recipient.push_back(val);
        });
    for (auto &val : recipient) {
      val /= sum;
    }
  }

private:
  struct EvidenceContribution {
    float evaluate(const std::vector<categoric::VarStateSize> &comb) const {
      return comb[var_pos_in_comb] == evidence_value ? 1.f : 0;
    }

    std::size_t var_pos_in_comb;
    categoric::VarStateSize evidence_value;
  };

  struct UnaryContribution {
    float evaluate(const std::vector<categoric::VarStateSize> &comb) const {
      return std::visit(
          [&](const auto &factor) {
            if constexpr (std::is_same_v<decltype(factor),
                                         const std::span<const float> &>) {
              return factor[comb[var_pos_in_comb]];
            } else {
              return factor->getTrsfm(comb[var_pos_in_comb]);
            }
          },
          factor);
    }

    std::size_t var_pos_in_comb;

    std::variant<std::span<const float>, const factor::UnaryFactor *,
                 const factor::UnaryFactorExponential *>
        factor;
  };

  struct BinaryContribution {
    float evaluate(const std::vector<categoric::VarStateSize> &comb) const {
      return std::visit(
          [&](const auto *factor) {
            return factor->template get<true>(
                {comb[var_first_pos_in_comb], comb[var_second_pos_in_comb]});
          },
          factor);
    }

    std::size_t var_first_pos_in_comb;
    std::size_t var_second_pos_in_comb;

    std::variant<const factor::BinaryFactor *,
                 const factor::BinaryFactorExponential *>
        factor;
  };

  using Contribution =
      std::variant<EvidenceContribution, UnaryContribution, BinaryContribution>;

  StructurePtr model_;

  struct Cache : misc::VectorCache<Contribution, categoric::VarStateSize> {
    std::unordered_map<std::size_t, std::size_t>
        vars_positions_in_joint_combination;
  };
  Cache cache_;
};
} // namespace

QueryManager::QueryManager() = default;

QueryManager::~QueryManager() {
  JointDistributionBuilder *ptr =
      reinterpret_cast<JointDistributionBuilder *>(joint_distribution_builder_);
  delete ptr;
}

std::span<float> QueryManager::gatherNodeUnaries_(std::size_t variable_index) {
  const auto &node = context_->nodes[variable_index];
  merger_.reset(node.var_size);
  use_factor_if(node.unary_factor, [&](const auto &factor) {
    merger_.template merge_factor<true>(factor);
  });
  for (const auto &connection : node.incoming_messages) {
    merger_.template merge<true>(context_->getMessageValues(connection));
  }
  return merger_.getMerged();
}

void QueryManager::getJointMarginalDistributionProbabilities_(
    std::vector<float> &recipient, std::span<const std::size_t> vars) {
  // TODO check there is no variable that repeat
  // TODO check all variables are actually existing

  context_->getManager<BeliefManager>()
      ->template propagateBelief<PropagationKind::SUM>();

  if (!joint_distribution_builder_) {
    joint_distribution_builder_ = new JointDistributionBuilder{context_};
  }
  JointDistributionBuilder *ptr =
      reinterpret_cast<JointDistributionBuilder *>(joint_distribution_builder_);
  ptr->compute(recipient, vars);
}

void QueryManager::getHiddenSetMAP(
    std::vector<categoric::VarStateSize> &recipient) {
  recipient.clear();

  context_->getManager<BeliefManager>()
      ->template propagateBelief<PropagationKind::MAP>();

  std::size_t index = 0;
  for (const auto &node : context_->nodes) {
    if (node.evidence == Evidence::NOT_AN_EVIDENCE) {
      auto distr = gatherNodeUnaries_(index);
      auto it = std::max_element(distr.begin(), distr.end());
      recipient.push_back(it - distr.begin());
    }
    index += 1;
  }
}
} // namespace EFG::structure
