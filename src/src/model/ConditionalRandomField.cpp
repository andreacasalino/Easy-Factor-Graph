/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/ConditionalRandomField.h>

namespace EFG::model {
ConditionalRandomField::ConditionalRandomField(structure::ModelSeed &&seed)
    : detail::ConditionalRandomFieldBase{
          std::forward<structure::ModelSeed>(seed)} {
  const auto &ctxt = getStructure();

  // TODO init EvidenceSetManager and EvidenceRemoveManager

  if (ctxt.permanent_evidences.empty()) {
    throw Error{
        "A ConditionalRandomField should have at least one permanent evidence"};
  }
  std::vector<categoric::VarStateSize> ev;
  ev.resize(ctxt.permanent_evidences.size(), 0);
  setEvidences(ev);
}

void ConditionalRandomField::setEvidences(
    std::span<const categoric::VarStateSize> evidence_values) {
  const auto &ctxt = getStructure();

  if (ctxt.permanent_evidences.size() != evidence_values.size()) {
    throw Error{"Invalid number of evidences for ConditionalRandomField"};
  }
  auto &buffer = cache_.get_buffer();
  std::size_t i{0};
  for (auto var_index : ctxt.permanent_evidences) {
    buffer.emplace_back(structure::Evidence{var_index, evidence_values[i]});
    i += 1;
  }
  this->structure::EvidenceSetManager::setEvidences(buffer);
}
} // namespace EFG::model
