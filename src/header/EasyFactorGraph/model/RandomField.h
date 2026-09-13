/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/model/Model.h>
#include <EasyFactorGraph/structure/BeliefManager.h>
#include <EasyFactorGraph/structure/ConfigManager.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/ModelExport.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/structure/TunableWeightsManager.h>
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

namespace EFG::model {
/**
 * @brief A simple graph object, that stores only const factors.
 * Evidences may be changed over the time.
 */
using RandomField =
    Model<structure::BeliefManager, structure::ConfigManager,
          structure::EvidenceSetManager, structure::EvidenceRemoveManager,
          structure::GibbsSampler, structure::QueryManager,
          structure::WorkerPoolManager, structure::TunableWeightsManager
#ifdef EFG_JSON_IO
          ,
          structure::ModelExport
#endif
          >;
} // namespace EFG::model
