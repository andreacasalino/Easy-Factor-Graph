// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include "../../structure/Commons.h"
// #include <train/handlers/UnaryHandler.h>

// namespace EFG::train::handler {
// UnaryHandler::UnaryHandler(
//     strct::Node &node,
//     std::shared_ptr<distribution::factor::modif::FactorExponential> factor)
//     : BaseHandler(factor), node(&node) {}

// float UnaryHandler::getGradientBeta() {
//   float beta = 0.f;
//   std::set<const distribution::Distribution *> toMerge;
//   strct::gatherUnaries(toMerge, *this->node);
//   for (auto it = this->node->activeConnections.begin();
//        it != this->node->activeConnections.end(); ++it) {
//     toMerge.emplace(it->second.message2This.get());
//   }
//   return this->dotProduct(
//       distribution::factor::cnst::Factor(toMerge).getProbabilities());
// }
// } // namespace EFG::train::handler
