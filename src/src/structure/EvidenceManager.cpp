/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>

#include <algorithm>
#include <ranges>
#include <span>
#include <unordered_map>

namespace EFG::structure {
ScopedEvidenceUpdate::~ScopedEvidenceUpdate() {
  if (ctxt_.vars_involved_by_update.empty() &&
      ctxt_.clusters_involved_by_update.empty()) {
    return;
  }

  for (const auto &[_, it] : ctxt_.clusters_involved_by_update) {
    ctxt_.vars_involved_by_update.insert(ctxt_.vars_involved_by_update.end(),
                                         it->variables.begin(),
                                         it->variables.end());
    ctxt_.structure->clusters.erase(it);
  }
  // newly generated clusters will start having a null previous belief
  // propagation information
  ctxt_.structure->updateClusters(
      misc::GenFromBlock<std::size_t>{ctxt_.vars_involved_by_update});
}

void EvidenceSetManager::setEvidences_(const std::vector<Evidence> &ev) {
  ScopedEvidenceUpdate guard{context_};
  for (auto e : ev) {
    guard.validateEvidence<true>(e.var_index, e.value);
  }
  for (auto e : ev) {
    setEvidence_(e, guard);
  }
}

void EvidenceSetManager::setEvidence_(const Evidence &ev,
                                      ScopedEvidenceUpdate &updater) {
  auto [var_index, value] = ev;

  auto &structure = *context_.structure;
  auto &node = structure.nodes[var_index];
  if (node.evidence == value) {
    return;
  }
  node.evidence = value;

  // recompute messages sent by the evidence
  for (auto &msg_data : node.incoming_messages) {
    msg_data.is_available = false;

    std::visit(
        [&](const auto &binary_factor) {
          auto &msg_twin = structure.getMessageData(msg_data.twin_idx);
          auto msg_twin_message = structure.getMessageValues(msg_twin);
          make_evidence_message(
              msg_twin_message, binary_factor,
              msg_data.factor_info.receiver_is_first_in_factor, value);
          msg_twin.is_available = true;

          auto it = structure.nodes[msg_data.factor_info.sender_index]
                        .hidden_cluster_it;
          if (it != structure.clusters.end()) {
            it->last_performed_propagation.reset();
          }
        },
        structure.binary_factors[msg_data.factor_info.factor_index].factor);
  }

  auto it = node.hidden_cluster_it;
  if (it != structure.clusters.end()) {
    // was hidden before
    auto it_cl =
        std::find(it->variables.begin(), it->variables.end(), var_index);
    it->variables.erase(it_cl);
    node.hidden_cluster_it = structure.clusters.end();
    updater.addCluster(it);
  }
}

template <bool Validate>
void EvidenceRemoveManager::removeEvidences_(
    const std::vector<std::size_t> &var) {
  ScopedEvidenceUpdate guard{context_};
  if constexpr (Validate) {
    for (auto v : var) {
      guard.validateEvidence<false>(v, 0);
    }
  }
  for (auto v : var) {
    removeEvidence_(v, guard);
  }
}

void EvidenceRemoveManager::removeEvidence_(std::size_t var_index,
                                            ScopedEvidenceUpdate &updater) {
  auto &structure = *context_.structure;
  auto &node = structure.nodes[var_index];
  if (node.evidence == Evidence::NOT_AN_EVIDENCE) {
    return;
  }
  node.evidence = Evidence::NOT_AN_EVIDENCE;
  updater.addVar(var_index);
  for (auto &msg : structure.get_active_incoming(var_index)) {
    msg.is_available = false;
    structure.getMessageData(msg.twin_idx).is_available = false;
    updater.addCluster(
        structure.nodes[msg.factor_info.sender_index].hidden_cluster_it);
  }
}

void EvidenceRemoveManager::removeAllEvidences() {
  auto &structure = *context_.structure;
  auto &buffer = this->ev_.get_buffer();
  for (std::size_t index = 0; index < structure.nodes.size(); ++index) {
    if (structure.nodes[index].evidence != Evidence::NOT_AN_EVIDENCE) {
      buffer.push_back(index);
    }
  }
  removeEvidences_<false>(buffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template void EvidenceRemoveManager::removeEvidences_<true>(
    const std::vector<std::size_t> &var);
template void EvidenceRemoveManager::removeEvidences_<false>(
    const std::vector<std::size_t> &var);
} // namespace EFG::structure
