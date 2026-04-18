/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/structure/BeliefManager.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/Structure.h>

#include <algorithm>
#include <limits>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace EFG::structure {
Structure::Structure(ModelSeed seed)
    : named_vars_table{std::move(seed.named)}, permanent_evidences{std::move(
                                                   seed.permanent_evidences)},
      tunability{std::move(seed.tunability)} {
  build_the_structure_(seed);
  transfer_into_pool_();
  updateClusters(misc::Range{nodes.size()});
}

namespace {
struct ConnectionsIter {
  ConnectionsIter(const ModelSeed &seed) {
    edges.reserve(seed.binary_factors.size() * 2);
    std::size_t factor_index{0};
    for (const auto &[key, _] : seed.binary_factors) {
      auto [var_first, var_second] = key;
      edges.emplace_back(EdgeInfo{var_first, var_second, true, factor_index});
      edges.emplace_back(EdgeInfo{var_second, var_first, false, factor_index});
      factor_index += 1;
    }
    std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
      return a.receiver_index > b.receiver_index;
    });
  }

  struct EdgeInfo {
    std::size_t receiver_index;
    std::size_t sender_index;
    bool receiver_is_first_in_factor;
    std::size_t factor_index;
  };

  bool next(std::vector<EdgeInfo> &res) {
    if (edges.empty()) {
      return false;
    }
    std::size_t len{0};
    for (auto it = edges.rbegin(); it != edges.rend(); ++it, len += 1) {
      if (it->receiver_index != edges.back().receiver_index) {
        break;
      }
    }
    auto it_begin = edges.end() - len;
    res.clear();
    std::for_each(it_begin, edges.end(),
                  [&res](const auto &to_add) { res.emplace_back(to_add); });
    edges.erase(it_begin, edges.end());
    return true;
  }

private:
  std::vector<EdgeInfo> edges;
};

} // namespace

void Structure::build_the_structure_(ModelSeed &seed) {
  nodes.resize(seed.variables_sizes.size());
  messages_data_buffer_.resize(seed.binary_factors.size() * 2);
  binary_factors.reserve(seed.binary_factors.size());

  std::size_t support_consumed{0};
  ConnectionsIter conn_it{seed};
  std::vector<ConnectionsIter::EdgeInfo> level;
  while (conn_it.next(level)) {
    std::size_t receiver = level.front().receiver_index;
    std::size_t begin = support_consumed;

    auto &node = nodes[receiver];
    node.var_size = seed.variables_sizes[receiver];
    node.evidence = Evidence::NOT_AN_EVIDENCE;

    if (auto it = seed.unary_factors.find(receiver);
        it != seed.unary_factors.end()) {
      std::visit([&](auto &&factor) { node.unary_factor = std::move(factor); },
                 std::move(it->second));
    }

    for (auto [_, sender, receiver_is_first_in_factor, factor_index] : level) {
      auto &added = messages_data_buffer_[support_consumed];
      added.factor_info.receiver_is_first_in_factor =
          receiver_is_first_in_factor;
      added.factor_info.receiver_index = receiver;
      added.factor_info.sender_index = sender;
      added.factor_info.factor_index = factor_index;
      added.is_available = false;
      added.idx = support_consumed;
      support_consumed += 1;
    }

    node.incoming_messages = {messages_data_buffer_.begin() + begin,
                              messages_data_buffer_.begin() + support_consumed};
  }

  std::size_t connection_idx{0};
  for (auto &connection : messages_data_buffer_) {
    if (connection.factor_info.receiver_is_first_in_factor) {
      const auto &sender_incoming =
          nodes[connection.factor_info.sender_index].incoming_messages;
      auto it = std::find_if(sender_incoming.begin(), sender_incoming.end(),
                             [&](const MessageMetaData &connection_o) {
                               return connection_o.factor_info.sender_index ==
                                      connection.factor_info.receiver_index;
                             });
      connection.twin_idx = static_cast<MessageMetaData::MessageIdx>(
                                it - sender_incoming.begin()) +
                            nodes[connection.factor_info.sender_index]
                                .incoming_messages.front()
                                .idx;
      it->twin_idx = connection_idx;
    }
    connection_idx += 1;
  }

  for (auto &[pair, el] : seed.binary_factors) {
    auto &added = binary_factors.emplace_back(std::move(el));
    added.vars_indices = pair;
  }
}

namespace {
std::size_t getMemoryFootprint(const Structure &subject) {
  std::size_t res{0};
  for (const auto &node : subject.nodes) {
    use_factor_if(node.unary_factor, [&](const auto &factor) {
      res += factor.getMemoryFootprint();
    });
  }
  for (const auto &factor : subject.binary_factors) {
    res += std::visit(
        [](const auto &factor) { return factor.getMemoryFootprint(); },
        factor.factor);
  }
  return res;
}
} // namespace

void Structure::transfer_into_pool_() {
  auto &&[messages_pool, messages_pool_len] = misc::NumbersPoolSizes::make(
      messages_data_buffer_ | std::views::transform([&](const auto &c) {
        return nodes[c.factor_info.receiver_index].var_size;
      }));
  messages_pool_.buffer.resize(messages_pool_len, 0);
  messages_pool_.sizes = std::move(messages_pool);

  factors_pool_ = std::make_unique<misc::MemoryPool>(getMemoryFootprint(*this));
  for (auto &node : nodes) {
    use_factor_if(node.unary_factor, [&](const auto &factor) {
      using RawType = std::remove_reference_t<decltype(factor)>;
      auto &factor_mut = const_cast<std::remove_const_t<RawType> &>(factor);
      factor_mut.transferIntoPool(*factors_pool_);
    });
  }
  for (auto &factor : binary_factors) {
    std::visit([&](auto &factor) { factor.transferIntoPool(*factors_pool_); },
               factor.factor);
  }
}
} // namespace EFG::structure
