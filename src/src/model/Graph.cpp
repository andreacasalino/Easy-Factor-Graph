/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/Graph.h>

namespace EFG::model {
void Graph::absorb(const strct::ConnectionsManager &to_absorb,
                   const bool copy) {
  const auto &factors = to_absorb.getAllFactors();
  absorbConstFactors(factors.begin(), factors.end(), copy);
  for (const auto& [var, val] : to_absorb.getEvidences()) {
      setEvidence(var, val);
  }
}
} // namespace EFG::model
