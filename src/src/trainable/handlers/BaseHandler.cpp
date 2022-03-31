// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <train/handlers/BaseHandler.h>
// #include <distribution/DistributionFinder.h>
// #include <distribution/DistributionIterator.h>
// #include <algorithm>

// namespace EFG::train::handler {
//     BaseHandler::BaseHandler(std::shared_ptr<distribution::factor::modif::FactorExponential>
//     factor)
//         : factor(factor) {
//     }

//     void BaseHandler::setTrainSet(TrainSetPtr newSet, const
//     std::set<categoric::VariablePtr>& modelVariables) {
//         distribution::DistributionFinder finder(*this->factor,
//         modelVariables); this->gradientAlpha = 0.f; float coeff = 1.f /
//         newSet->getSet().size(); std::for_each(newSet->getSet().begin(),
//         newSet->getSet().end(), [&](const CombinationPtr& c) {
//             this->gradientAlpha += coeff * finder.findRaw(*c).second;
//         });
//     }

//     float BaseHandler::dotProduct(const std::vector<float>& prob) const {
//         float dot = 0.f;
//         auto iter = this->factor->getIterator();
//         auto probIter = prob.begin();
//         iterator::forEach(iter, [&](const distribution::DistributionIterator&
//         iter) {
//             dot += *probIter * iter.getImageRaw();
//             ++probIter;
//         });
//         return dot;
//     }
// }
