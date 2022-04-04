#ifdef EFG_XML_IO
/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/FactorExponential.h>
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

xmlPrs::Tag &printExpPotential(const distribution::FactorExponential &distr,
                               xmlPrs::Tag &recipient) {
  auto &pot_tag = printPotential(distr, recipient);
  pot_tag.getAttributes().emplace("weight", std::to_string(distr.getWeight()));
  return pot_tag;
}
} // namespace

void Exporter::exportComponents(const File &filePath,
                                const std::string &model_name,
                                const AwarePtrs &subject) {
  xmlPrs::Root exp_root(model_name);
  if (model_name.empty()) {
    exp_root.setName("EFG-model");
  } else {
    exp_root.setName(model_name);
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
    std::shared_ptr<const distribution::FactorExponential> as_const_exp_factor =
        std::dynamic_pointer_cast<const distribution::FactorExponential,
                                  const distribution::Distribution>(
            const_factor);
    if (nullptr == as_const_exp_factor) {
      printPotential(*const_factor, exp_root);
    } else {
      auto &factor_tag = printExpPotential(*as_const_exp_factor, exp_root);
      factor_tag.getAttributes().emplace("tunability", "N");
    }
  }
  if (nullptr != subject.as_factors_tunable_aware) {
    // tunable factors
    for (const auto &cluster :
         subject.as_factors_tunable_aware->getTunableClusters()) {
      const auto &front_factor = *cluster.front();
      printExpPotential(front_factor, exp_root);
      for (std::size_t k = 1; k < cluster.size(); ++k) {
        auto &factor_tag = printExpPotential(*cluster[k], exp_root);
        printGroup(front_factor.getVariables(), factor_tag["Share"]);
      }
    }
  }
  auto stream = make_out_stream(filePath.str());
  exp_root.print(*stream);
}
} // namespace EFG::io::xml
#endif
