/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsAdder.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/QueryManager.h>

namespace EFG::model {
/**
 * @brief A simple graph object, that can't store tunable factors.
 */
class Graph : public strct::EvidenceSetter,
              public strct::EvidenceRemover,
              public strct::FactorsAdder,
              public strct::GibbsSampler,
              public strct::QueryManager {
public:
  Graph() = default;

  //   template <typename Model> explicit Graph(const Model &o) {
  //     this->absorbModel(o);
  //   };

  //   Graph(const Graph &o) { this->absorbModel(o, true); };
  Graph &operator=(const Graph &) = delete;
};
} // namespace EFG::model
