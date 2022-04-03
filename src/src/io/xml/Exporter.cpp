/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/io/xml/Exporter.h>

#include <XML-Parser/Tag.h>

#include "../Utils.h"

namespace EFG::io::xml {
namespace {
void printGroup(const categoric::Group &group, xmlPrs::Tag &recipient) {
  auto &attributes = recipient.getAttributes();
  for (const auto &var : group.getVariables()) {
    attributes.emplace("var", var->name());
  }
};

xmlPrs::Tag &printPotential(const distribution::Distribution &distr,
                            xmlPrs::Tag &recipient) {
  auto &pot_tag = recipient.addNested("Potential");
  printGroup(distr.getVariables(), pot_tag);
  for (const auto &[comb, val] : distr.getCombinationsMap()) {
    auto &comb_val = pot_tag.addNested("Distr_val");
    auto &attributes = comb_val.getAttributes();
    attributes.emplace("D", std::to_string(val));
    for (const auto &v : comb.data()) {
      attributes.emplace("v", std::to_string(v));
    }
  }
  return pot_tag;
};

} // namespace

void Exporter::exportComponents(const std::string &filePath,
                                const std::string &modelName,
                                const AwarePtrs &subject) {
  xmlPrs::Root exp_root(modelName);
  if (modelName.empty()) {
    exp_root.setName("graphical-model");
  } else {
    exp_root.setName(modelName);
  }
  // hidden set
  for (const auto &hidden_var :
       subject.as_structure_aware->getHiddenVariables()) {
    auto &temp = exp_root.addNested("Variable");
    temp.getAttributes().emplace("name", hidden_var->name());
    temp.getAttributes().emplace("Size", std::to_string(hidden_var->size()));
  }
  // evidence set
  for (const auto &[evidence_var, evidence_val] :
       subject.as_structure_aware->getEvidences()) {
    auto &temp = exp_root.addNested("Variable");
    temp.getAttributes().emplace("name", evidence_var->name());
    temp.getAttributes().emplace("Size", std::to_string(evidence_var->size()));
    temp.getAttributes().emplace("flag", "O");
  }
  // const factors
  for (const auto &const_factor :
       subject.as_factors_const_aware->getConstFactors()) {
    auto &factor_tag = printPotential(*const_factor, exp_root);
    std::shared_ptr<const distribution::FactorExponential> as_const_exp_factor =
        std::dynamic_pointer_cast<const distribution::FactorExponential,
                                  const distribution::Distribution>(
            const_factor);
    if (nullptr != as_const_exp_factor) {
      factor_tag.getAttributes().emplace(
          "weight", std::to_string(as_const_exp_factor->getWeight()));
      factor_tag.getAttributes().emplace("tunability", "N");
    }
  }
  if (nullptr != subject.as_factors_tunable_aware) {
    // tunable factors
    // for (const auto &cluster :
    //      subject.as_factors_tunable_aware->getTunableFactors()) {
    // }
  }

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
  //                 distribution::factor::modif::FactorExponential>> &cluster)
  //                 {
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

  auto stream = make_out_stream(filePath);
  exp_root.print(*stream);
}
} // namespace EFG::io::xml
