/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/model/Graph.h>

namespace EFG::model {
Graph::Graph(structure::ModelSeed &&seed)
    : detail::GraphBase{std::forward<structure::ModelSeed>(seed)} {
  auto &ctxt = const_cast<structure::Structure &>(this->getStructure());
  ctxt.tunability.order.clear();
}
} // namespace EFG::model
