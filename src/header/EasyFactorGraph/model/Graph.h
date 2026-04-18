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
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

namespace EFG::model {
namespace detail {
using GraphBase =
    Model<structure::BeliefManager, structure::ConfigManager,
          structure::EvidenceSetManager, structure::EvidenceRemoveManager,
          structure::GibbsSampler, structure::QueryManager,
          structure::WorkerPoolManager
#ifdef EFG_JSON_IO
          ,
          structure::ModelExport
#endif
          >;
} // namespace detail

/**
 * @brief A simple graph object, that stores only const factors.
 * Evidences may be changed over the time.
 */
class Graph : public detail::GraphBase {
public:
  Graph(structure::ModelSeed &&seed);
};
} // namespace EFG::model
