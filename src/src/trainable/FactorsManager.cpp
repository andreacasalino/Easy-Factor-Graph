// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <EasyFactorGraph/distribution/Factor.h>
// #include <EasyFactorGraph/structure/DistributionsAware.h>

// namespace EFG::strct {
// void DistributionsInserter::insertDistribution(
//     const distribution::DistributionCnstPtr &distribution) {
//   addDistribution(distribution);
//   distributions.emplace(distribution);
// }

// void DistributionsInserter::copyDistribution(
//     const distribution::Distribution &distribution) {
//   insertDistribution(std::make_shared<distribution::Factor>(distribution));
// }
// } // namespace EFG::strct

// #include <EasyFactorGraph/Error.h>
// #include <EasyFactorGraph/structure/DistributionsTunableAware.h>

// #include <algorithm>

// namespace EFG::strct {
// std::vector<float> DistributionsReader::getWeights() const {
//   std::vector<float> result;
//   result.reserve(tunable_clusters.size());
//   for (const auto &cluster : tunable_clusters) {
//     for (const auto &distribution : cluster) {
//       result.push_back(distribution->getWeight());
//     }
//   }
//   return result;
// }

// void DistributionsTunableInserter::insertTunableDistribution(
//     const FactorExponentialPtr &tunable_distribution) {
//   addDistribution(tunable_distribution);
//   tunable_clusters.emplace_back().push_back(tunable_distribution);
// }

// namespace {
// bool contains_group(const std::vector<FactorExponentialPtr> &cluster,
//                     const categoric::VariablesSet &group_sharing_weigth) {
//   return std::find_if(
//              cluster.begin(), cluster.end(),
//              [&group_sharing_weigth](const FactorExponentialPtr &element) {
//                return element->getVariables().getVariablesSet() ==
//                       group_sharing_weigth;
//              }) != cluster.end();
// };
// } // namespace

// TunableClusters::const_iterator
// DistributionsTunableInserter::insertTunableDistribution(
//     const FactorExponentialPtr &tunable_distribution,
//     const categoric::VariablesSet &group_sharing_weigth) {
//   auto tunable_clusters_it =
//       std::find_if(tunable_clusters.begin(), tunable_clusters.end(),
//                    [&group_sharing_weigth](
//                        const std::vector<FactorExponentialPtr> &cluster) {
//                      return contains_group(cluster, group_sharing_weigth);
//                    });
//   if (tunable_clusters_it == tunable_clusters.end()) {
//     throw Error{"Invalid group to share the weigth"};
//   }
//   addDistribution(tunable_distribution);
//   tunable_clusters_it->push_back(tunable_distribution);
//   return tunable_clusters_it;
// }

// void DistributionsTunableInserter::copyTunableDistribution(
//     const distribution::FactorExponential &tunable_distribution) {
//   insertTunableDistribution(
//       std::make_shared<distribution::FactorExponential>(tunable_distribution));
// }

// TunableClusters::const_iterator
// DistributionsTunableInserter::copyTunableDistribution(
//     const distribution::FactorExponential &tunable_distribution,
//     const categoric::VariablesSet &group_sharing_weigth) {
//   return insertTunableDistribution(
//       std::make_shared<distribution::FactorExponential>(tunable_distribution),
//       group_sharing_weigth);
// }
// } // namespace EFG::strct
