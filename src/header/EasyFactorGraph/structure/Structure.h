/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/misc/MemoryPool.h>
#include <EasyFactorGraph/misc/WorkerPool.h>
#include <EasyFactorGraph/structure/Definitions.h>
#include <EasyFactorGraph/structure/ModelSeed.h>

#include <memory>
#include <ranges>
#include <span>
#include <stack>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace EFG::structure {
class BeliefManager;
class EvidenceSetManager;
class EvidenceRemoveManager;

struct WorkerPoolActivityListener {
  virtual void updatePool(misc::WorkerPool *pool) { pool_ = pool; };

  auto *getPool() { return pool_; }

private:
  misc::WorkerPool *pool_{nullptr};
};

struct Structure {
  Structure(ModelSeed seed);

  Structure(const Structure &) = delete;
  Structure &operator=(const Structure &) = delete;

  Structure(Structure &&) noexcept = delete;
  Structure &operator=(Structure &&) noexcept = delete;

  ///////////////////////////////////////////////////////////////////////////
  /// modifiers
  ///////////////////////////////////////////////////////////////////////////

  template <typename VarsGen>
  requires misc::IsGenerator<VarsGen, std::size_t>
  void updateClusters(VarsGen vars);

  template <typename M> void registerManager(M &manager) {
    std::get<M *>(concrete_managers_) = &manager;
  }

  void addListener(WorkerPoolActivityListener &listener) {
    listeners_.push_back(&listener);
  }

  std::vector<float> make_messages_pool_swap() const {
    return messages_pool_.buffer;
  }

  void swap_messages_pool(std::vector<float> &swap) {
    std::vector<float> tmp{std::move(messages_pool_.buffer)};
    messages_pool_.buffer = std::move(swap);
    swap = std::move(tmp);
  }

  ///////////////////////////////////////////////////////////////////////////
  /// getters
  ///////////////////////////////////////////////////////////////////////////

  auto get_active_incoming(std::size_t var_index) const {
    const auto &node = nodes[var_index];
    return node.incoming_messages | std::views::filter([&](const auto &msg) {
             return nodes[msg.factor_info.sender_index].evidence ==
                    Evidence::NOT_AN_EVIDENCE;
           });
  }

  auto get_message_dependencies_rng(const MessageMetaData &msg) const {
    const auto &sender_node = nodes[msg.factor_info.sender_index];
    return sender_node.incoming_messages |
           std::views::filter([&](const auto &msg_dep) {
             return msg_dep.factor_info.sender_index !=
                    msg.factor_info.receiver_index;
           });
  }

  template <typename M> M *getManager() {
    return std::get<M *>(concrete_managers_);
  }

  auto &getMessageData(MessageMetaData::MessageIdx idx) {
    return messages_data_buffer_[idx];
  }

  std::span<float> getMessageValues(const MessageMetaData &msg,
                                    std::vector<float> &buffer) {
    return messages_pool_.sizes.get_slot(msg.idx, buffer);
  }
  std::span<float> getMessageValues(const MessageMetaData &msg) {
    return getMessageValues(msg, messages_pool_.buffer);
  }
  std::span<const float> getMessageValues(const MessageMetaData &msg) const {
    Structure &self = const_cast<Structure &>(*this);
    return self.getMessageValues(msg);
  }

  auto &getListeners() { return listeners_; }

  std::size_t locateBinaryFactor(std::size_t var_a, std::size_t var_b) const {
    if (nodes.size() <= var_a || nodes.size() <= var_b) {
      throw Error{"Invalid variables indices"};
    }
    const auto &node_a = nodes[var_a];
    auto it =
        std::find_if(node_a.incoming_messages.begin(),
                     node_a.incoming_messages.end(), [&](const auto &msg_data) {
                       return msg_data.factor_info.sender_index == var_b;
                     });
    if (it == node_a.incoming_messages.end()) {
      throw Error{
          "There is no binary factor connecting vars with indices {} and {}",
          var_a, var_b};
    }
    return it->factor_info.factor_index;
  }

  ///////////////////////////////////////////////////////////////////////////
  /// data
  ///////////////////////////////////////////////////////////////////////////

  struct BinaryFactorInStructure {
    FactorOrFactorExponential<2> factor;
    std::pair<std::size_t, std::size_t> vars_indices;
  };

  Config config;
  NamedVariables named_vars_table;
  std::vector<std::size_t> permanent_evidences;
  std::vector<Node> nodes;
  std::vector<BinaryFactorInStructure> binary_factors;
  Tunability tunability;
  HiddenClusters clusters;

private:
  void build_the_structure_(ModelSeed &seed);

  void transfer_into_pool_();

  std::tuple<BeliefManager *, EvidenceSetManager *, EvidenceRemoveManager *>
      concrete_managers_;

  std::vector<WorkerPoolActivityListener *> listeners_;

  std::unique_ptr<misc::MemoryPool> factors_pool_;

  struct MessagesPool {
    misc::NumbersPoolSizes sizes;
    std::vector<float> buffer;
  };
  std::vector<MessageMetaData> messages_data_buffer_;
  MessagesPool messages_pool_;
};

using StructurePtr = std::shared_ptr<Structure>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename VarsGen>
requires misc::IsGenerator<VarsGen, std::size_t>
void Structure::updateClusters(VarsGen vars) {
  VarsGen vars_clone = vars;

  misc::for_each_generated<std::size_t>(std::move(vars), [&](std::size_t var) {
    nodes[var].hidden_cluster_it = clusters.end();
  });

  misc::for_each_generated<std::size_t>(
      std::move(vars_clone), [&](std::size_t var) {
        if (nodes[var].hidden_cluster_it != clusters.end()) {
          return;
        }

        // start exploration from var and build the cluster that contains it
        auto &created = clusters.emplace_front();

        std::stack<std::size_t> queue;
        queue.push(var);

        while (!queue.empty()) {
          auto reached = queue.top();
          queue.pop();
          if (nodes[reached].hidden_cluster_it != clusters.end()) {
            continue;
          }
          created.variables.push_back(reached);
          nodes[reached].hidden_cluster_it = clusters.begin();
          // loop across currently active connections
          for (const auto &msg : get_active_incoming(reached)) {
            queue.push(msg.factor_info.sender_index);
          }
        }
      });
}
} // namespace EFG::structure
