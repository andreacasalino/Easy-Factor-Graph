/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/io/xml/Exporter.h>

#include <XML-Parser/Tag.h>

namespace EFG::io::xml {
namespace {
xmlPrs::Tag &printVariable(const categoric::VariablePtr &var,
                           xmlPrs::Tag &recipient) {
  auto &var_tag = recipient.addNested("Variable");
  auto &attributes = var_tag.getAttributes();
  attributes.emplace("name", var->name());
  attributes.emplace("Size", std::to_string(var->size()));
  return var_tag;
}

void printGroup(const categoric::Group &group, xmlPrs::Tag &recipient) {
  auto &attributes = recipient.getAttributes();
  for (const auto &var : group.getVariables()) {
    attributes.emplace("var", var->name());
  }
}

xmlPrs::Tag &printPotential(const distribution::Distribution &distr,
                            xmlPrs::Tag &recipient) {
  auto &pot_tag = recipient.addNested("Potential");
  printGroup(distr.getGroup(), pot_tag);
  for (const auto &[comb, val] : distr.getCombinationsMap()) {
    auto &comb_val = pot_tag.addNested("Distr_val");
    auto &attributes = comb_val.getAttributes();
    attributes.emplace("D", std::to_string(val));
    for (const auto &v : comb.data()) {
      attributes.emplace("v", std::to_string(v));
    }
  }
  return pot_tag;
}

xmlPrs::Tag &printExpPotential(const distribution::FactorExponential &distr,
                               xmlPrs::Tag &recipient) {
  auto &pot_tag = printPotential(distr, recipient);
  pot_tag.getAttributes().emplace("weight", std::to_string(distr.getWeight()));
  return pot_tag;
}
} // namespace

void Exporter::convert(std::ostream &recipient, const AwarePtrs &subject,
                       const std::string &model_name) {
  xmlPrs::Root exp_root(model_name);
  // hidden set
  for (const auto &hidden_var :
       subject.as_structure_aware->getHiddenVariables()) {
    printVariable(hidden_var, exp_root);
  }
  // evidence set
  for (const auto &[evidence_var, evidence_val] :
       subject.as_structure_aware->getEvidences()) {
    auto &var_tag = printVariable(evidence_var, exp_root);
    var_tag.getAttributes().emplace("evidence", std::to_string(evidence_val));
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
      auto &factor_tag = printExpPotential(front_factor, exp_root);
      factor_tag.getAttributes().emplace("tunability", "Y");
      for (std::size_t k = 1; k < cluster.size(); ++k) {
        auto &factor_tag2 = printExpPotential(*cluster[k], exp_root);
        factor_tag2.getAttributes().emplace("tunability", "Y");
        printGroup(front_factor.getGroup(), factor_tag2["Share"]);
      }
    }
  }
  exp_root.print(recipient);
}
} // namespace EFG::io::xml
#endif
