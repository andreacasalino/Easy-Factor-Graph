/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/io/xml/Exporter.h>

#include "../Utils.h"

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

xmlPrs::Tag &printPotential(const factor::Immutable &distr,
                            xmlPrs::Tag &recipient) {
  auto &pot_tag = recipient.addNested("Potential");
  printGroup(distr.function().vars(), pot_tag);
  distr.function().forEachNonNullCombination<false>(
      [&](const auto &comb, float img) {
        auto &comb_val = pot_tag.addNested("Distr_val");
        auto &attributes = comb_val.getAttributes();
        attributes.emplace("D", std::to_string(img));
        for (auto v : comb) {
          attributes.emplace("v", std::to_string(v));
        }
      });
  return pot_tag;
}

xmlPrs::Tag &printExpPotential(const factor::FactorExponential &distr,
                               xmlPrs::Tag &recipient) {
  auto &pot_tag = printPotential(distr, recipient);
  pot_tag.getAttributes().emplace("weight", std::to_string(distr.getWeight()));
  return pot_tag;
}
} // namespace

void Exporter::convert(const std::filesystem::path &out_file, Getters subject,
                       const std::string &model_name) {
  auto [state, constGetter, tunableGetter] = subject;
  xmlPrs::Root exp_root(model_name);
  // hidden set
  for (const auto &hidden_var : state->getHiddenVariables()) {
    printVariable(hidden_var, exp_root);
  }
  // evidence set
  for (const auto &[evidence_var, evidence_val] : state->getEvidences()) {
    auto &var_tag = printVariable(evidence_var, exp_root);
    var_tag.getAttributes().emplace("evidence", std::to_string(evidence_val));
  }
  // const factors
  for (const auto &const_factor : constGetter->getConstFactors()) {
    std::shared_ptr<const factor::FactorExponential> as_const_exp_factor =
        std::dynamic_pointer_cast<const factor::FactorExponential,
                                  const factor::Immutable>(const_factor);
    if (nullptr == as_const_exp_factor) {
      printPotential(*const_factor, exp_root);
    } else {
      auto &factor_tag = printExpPotential(*as_const_exp_factor, exp_root);
      factor_tag.getAttributes().emplace("tunability", "N");
    }
  }
  if (tunableGetter) {
    // tunable factors
    struct Visitor {
      xmlPrs::Root &root;

      void operator()(const train::FactorExponentialPtr &factor) const {
        printExpPotential(*factor, root)
            .getAttributes()
            .emplace("tunability", "Y");
      }
      void operator()(const train::TunableClusters &cluster) const {
        printExpPotential(*cluster.front(), root)
            .getAttributes()
            .emplace("tunability", "Y");
        const auto &vars = cluster.front()->function().vars();
        std::for_each(cluster.begin() + 1, cluster.end(),
                      [&](const train::FactorExponentialPtr &factor) {
                        auto &factor_tag = printExpPotential(*factor, root);
                        factor_tag.getAttributes().emplace("tunability", "Y");
                        printGroup(vars, factor_tag["Share"]);
                      });
      }
    } visitor{exp_root};
    for (const auto &cluster : tunableGetter->getTunableClusters()) {
      std::visit(visitor, cluster);
    }
  }
  useOutStrem(out_file,
              [&exp_root](std::ofstream &stream) { exp_root.print(stream); });
}
} // namespace EFG::io::xml
#endif
