// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <train/handlers/BinaryHandler.h>
// #include "../../structure/Commons.h"

// namespace EFG::train::handler {
//     BinaryHandler::BinaryHandler(strct::Node& nodeA, strct::Node& nodeB,
//     std::shared_ptr<distribution::factor::modif::FactorExponential> factor)
//         : BaseHandler(factor)
//         , nodeA(&nodeA)
//         , nodeB(&nodeB) {
//     }

//     float BinaryHandler::getGradientBeta() {
//         float beta = 0.f;
//         std::set<const distribution::Distribution*> toMerge;
//         strct::gatherUnaries(toMerge, *this->nodeA);
//         for (auto it = this->nodeA->activeConnections.begin(); it !=
//         this->nodeA->activeConnections.end(); ++it) {
//             toMerge.emplace(it->second.message2This.get());
//         }
//         strct::gatherUnaries(toMerge, *this->nodeB);
//         for (auto it = this->nodeB->activeConnections.begin(); it !=
//         this->nodeB->activeConnections.end(); ++it) {
//             toMerge.emplace(it->second.message2This.get());
//         }
//         toMerge.extract(this->nodeA->activeConnections.find(this->nodeB)->second.message2This.get());
//         toMerge.extract(this->nodeB->activeConnections.find(this->nodeA)->second.message2This.get());
//         toMerge.emplace(this->factor.get());
//         return
//         this->dotProduct(distribution::factor::cnst::Factor(toMerge).getProbabilities());
//     }
// }
