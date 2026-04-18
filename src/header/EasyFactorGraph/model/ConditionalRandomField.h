/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/model/Model.h>
#include <EasyFactorGraph/structure/BeliefManager.h>
#include <EasyFactorGraph/structure/ConfigManager.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/ModelExport.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/structure/TunableWeightsManager.h>
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

#include <span>

namespace EFG::model {
namespace detail {
using ConditionalRandomFieldBase =
    Model<structure::BeliefManager, structure::ConfigManager,
          structure::GibbsSampler, structure::QueryManager,
          structure::WorkerPoolManager, structure::TunableWeightsManager
#ifdef EFG_JSON_IO
          ,
          structure::ModelExport
#endif
          >;
} // namespace detail

class ConditionalRandomField : public detail::ConditionalRandomFieldBase,
                               private structure::EvidenceSetManager,
                               private structure::EvidenceRemoveManager {
public:
  ConditionalRandomField(structure::ModelSeed &&seed);

  void
  setEvidences(const std::vector<categoric::VarStateSize> &evidence_values) {
    setEvidences(std::span<const categoric::VarStateSize>{
        evidence_values.begin(), evidence_values.end()});
  }

  // just the values packed all together, same order of getHiddenSetRng()
  void setEvidences(std::span<const categoric::VarStateSize> evidence_values);

private:
  misc::VectorCache<structure::Evidence> cache_;
};
} // namespace EFG::model
