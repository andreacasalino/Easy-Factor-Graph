/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/structure/Structure.h>

namespace EFG::structure {
struct ScopedEvidenceUpdate {
  struct Context {
    void reset() {
      vars_involved_by_update.clear();
      clusters_involved_by_update.clear();
    }

    std::vector<std::size_t> vars_involved_by_update;
    std::unordered_map<HiddenCluster *, typename HiddenClusters::iterator>
        clusters_involved_by_update;

    StructurePtr structure;
  };

  ScopedEvidenceUpdate(Context &ctxt) : ctxt_{ctxt} { ctxt_.reset(); }
  ~ScopedEvidenceUpdate();

  template <bool ValidateValue>
  void validateEvidence(std::size_t variable_index,
                        categoric::VarStateSize value) {
    if (ctxt_.structure->nodes.size() <= variable_index) {
      throw Error{"Out of range variable for an evidence"};
    }
    if constexpr (ValidateValue) {
      auto &node = ctxt_.structure->nodes[variable_index];
      if (node.var_size <= value) {
        throw Error(
            "Cannot set evidence to {} for variable at index {} has its "
            "size is {}",
            value, variable_index, node.var_size);
      }
    }
  }

  void addCluster(typename HiddenClusters::iterator it) {
    ctxt_.clusters_involved_by_update.emplace(&(*it), it);
  }

  void addVar(std::size_t var_index) {
    ctxt_.vars_involved_by_update.push_back(var_index);
  }

private:
  Context &ctxt_;
};

class EvidenceSetManager {
public:
  /**
   * @brief update the evidence set with the specified new evidence.
   * In case the involved variable was already part of the evidence set, the
   * evidence value is simply updated.
   * On the contrary case, the involved variable is moved into the evidence set,
   * with the specified value.
   * @param the involved variable
   * @param the evidence value
   * @throw in case the passed variable is not part of the model.
   */
  template <typename... ARGS>
  void setEvidences(Evidence first, ARGS &&...others) {
    auto &buffer = ev_.get_buffer();
    buffer.push_back(first);
    (buffer.emplace_back(others), ...);
    setEvidences_(buffer);
  }

  void setEvidences(const std::vector<Evidence> &evidences) {
    setEvidences_(evidences);
  }

protected:
  void init(StructurePtr context) {
    context_.structure = context;
    context->registerManager(*this);
  }

private:
  void setEvidences_(const std::vector<Evidence> &ev);

  void setEvidence_(const Evidence &ev, ScopedEvidenceUpdate &updater);

  misc::VectorCache<Evidence> ev_;
  ScopedEvidenceUpdate::Context context_;
};

class EvidenceRemoveManager {
public:
  /**
   * @brief update the evidence set by removing the specified variable.
   * @param the involved variable
   * @throw in case the passed variable is not part of the model.
   * @throw in case the passed variable is not part of the current evidence
   set.
   */
  template <typename... ARGS>
  void removeEvidences(std::size_t first, ARGS &&...others) {
    auto &buffer = this->ev_.get_buffer();
    buffer.push_back(first);
    (buffer.emplace_back(others), ...);
    removeEvidences_<true>(buffer);
  }

  void removeEvidences(const std::vector<std::size_t> &vars) {
    removeEvidences_<true>(vars);
  }

  /**
   * @brief removes all the evidences currently set for this model.
   */
  void removeAllEvidences();

protected:
  void init(StructurePtr context) {
    context_.structure = context;
    context->registerManager(*this);
  }

private:
  template <bool Validate>
  void removeEvidences_(const std::vector<std::size_t> &var);

  void removeEvidence_(std::size_t var, ScopedEvidenceUpdate &updater);

  misc::VectorCache<std::size_t> ev_;
  ScopedEvidenceUpdate::Context context_;
};
} // namespace EFG::structure
