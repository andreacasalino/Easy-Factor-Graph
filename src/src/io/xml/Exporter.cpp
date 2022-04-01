// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #include <XML-Parser/Parser.h>
// #include <algorithm>
// #include <distribution/DistributionIterator.h>
// #include <fstream>
// #include <io/xml/Exporter.h>

// namespace EFG::io::xml {
// void printGroup(const categoric::Group &group, xmlPrs::Tag &tag) {
//   std::for_each(group.getVariables().begin(), group.getVariables().end(),
//                 [&tag](const categoric::VariablePtr &v) {
//                   tag.getAttributes().emplace("var", v->name());
//                 });
// };

// xmlPrs::Tag &printPotential(const distribution::Distribution &distr,
//                             xmlPrs::Tag &tag) {
//   auto &pot_tag = tag.addNested("Potential");
//   printGroup(distr.getGroup(), pot_tag);
//   auto it = distr.getIterator();
//   iterator::forEach(
//       it, [&pot_tag](const distribution::DistributionIterator &it) {
//         auto &temp = pot_tag.addNested("Distr_val");
//         for (std::size_t k = 0; k < it.getCombination().size(); ++k)
//           temp.getAttributes().emplace(
//               "v", std::to_string(it.getCombination().data()[k]));
//         temp.getAttributes().emplace("D", std::to_string(it.getImageRaw()));
//       });
//   return pot_tag;
// };

// void Exporter::exportComponents(
//     const std::string &filePath, const std::string &modelName,
//     const std::tuple<const strct::EvidenceAware *,
//                      const strct::StructureAware *,
//                      const strct::StructureTunableAware *> &components) {
//   xmlPrs::Root exp_root(modelName);
//   if (modelName.empty()) {
//     exp_root.setName("graphical-model");
//   } else {
//     exp_root.setName(modelName);
//   }
//   // hidden set
//   auto H_vars = std::get<0>(components)->getHiddenVariables();
//   std::for_each(H_vars.begin(), H_vars.end(),
//                 [&exp_root](const categoric::VariablePtr &v) {
//                   auto &temp = exp_root.addNested("Variable");
//                   temp.getAttributes().emplace("name", v->name());
//                   temp.getAttributes().emplace("Size",
//                                                std::to_string(v->size()));
//                 });
//   // evidence set
//   auto O_vars = std::get<0>(components)->getEvidences();
//   for (auto itO = O_vars.begin(); itO != O_vars.end(); ++itO) {
//     auto &temp = exp_root.addNested("Variable");
//     temp.getAttributes().emplace("name", itO->first->name());
//     temp.getAttributes().emplace("Size", std::to_string(itO->first->size()));
//     temp.getAttributes().emplace("flag", "O");
//   }
//   // factors
//   auto factors = std::get<1>(components)->getConstFactors();
//   std::for_each(
//       factors.begin(), factors.end(),
//       [&exp_root](
//           const std::shared_ptr<distribution::factor::cnst::Factor> &f) {
//         printPotential(*f, exp_root);
//       });
//   // exp const factors
//   auto factorsExp = std::get<1>(components)->getConstFactorsExp();
//   std::for_each(
//       factorsExp.begin(), factorsExp.end(),
//       [&exp_root](
//           const
//           std::shared_ptr<distribution::factor::cnst::FactorExponential>
//               &f) {
//         auto &temp = printPotential(*f, exp_root);
//         temp.getAttributes().emplace("weight",
//         std::to_string(f->getWeight()));
//         temp.getAttributes().emplace("tunability", "N");
//       });
//   if (nullptr != std::get<2>(components)) {
//     // exp tunable factors
//     auto factorsExp = std::get<2>(components)->getFactorsExp();
//     std::for_each(
//         factorsExp.begin(), factorsExp.end(),
//         [&exp_root](
//             const std::vector<std::shared_ptr<
//                 distribution::factor::modif::FactorExponential>> &cluster) {
//           auto itCl = cluster.begin();
//           auto &temp = printPotential(**itCl, exp_root);
//           temp.getAttributes().emplace("weight",
//                                        std::to_string((*itCl)->getWeight()));
//           const auto &varsFront = (*itCl)->getGroup();
//           ++itCl;
//           std::for_each(
//               itCl, cluster.end(),
//               [&exp_root,
//                &varsFront](const std::shared_ptr<
//                            distribution::factor::modif::FactorExponential>
//                            &f) {
//                 auto &temp = printPotential(*f, exp_root);
//                 temp.getAttributes().emplace("weight",
//                                              std::to_string(f->getWeight()));
//                 auto &shareTag = temp.addNested("Share");
//                 printGroup(varsFront, shareTag);
//               });
//         });
//   }
//   std::ofstream stream(filePath);
//   stream << exp_root;
// }
// } // namespace EFG::io::xml
