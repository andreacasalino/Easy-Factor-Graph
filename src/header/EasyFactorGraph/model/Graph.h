/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsConstManager.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/QueryManager.h>

namespace EFG::model {
/**
 * @brief A simple graph object, that stores only const factors.
 * Evidences may be changed over the time.
 */
class Graph : public strct::EvidenceSetter,
              public strct::EvidenceRemover,
              public strct::FactorsConstInserter,
              public strct::GibbsSampler,
              public strct::QueryManager {
public:
  Graph() = default;

  Graph(const Graph &o) { absorb(o, false); };
  Graph &operator=(const Graph &) = delete;

  /**
   * @brief Gather all the factors (tunable and constant) of another model and
   * insert/copy them into this object.
   * @param the model whose factors should be inserted/copied
   * @param when passing true the factors are deep copied, while in the contrary
   * case shallow copies of the smart pointers are inserted into this model.
   */
  void absorb(const strct::FactorsAware &to_absorb, bool copy);
};
} // namespace EFG::model
