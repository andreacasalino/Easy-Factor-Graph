/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Error.h>
#include <XML-Parser/Parser.h>
#include <algorithm>
#include <distribution/factor/const/FactorExponential.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/modifiable/FactorExponential.h>
#include <io/xml/Importer.h>

namespace EFG::io::xml {
const std::string *findAttribute(const xmlPrs::Tag &tag,
                                 const std::string &attributeName) {
  auto it = tag.getAttributes().find(attributeName);
  if (it == tag.getAttributes().end()) {
    return nullptr;
  }
  return &it->second;
};

categoric::VariablePtr
findVariable(const std::string &name,
             const std::set<categoric::VariablePtr> &variables) {
  auto itV = variables.find(categoric::makeVariable(2, name));
  if (itV == variables.end()) {
    throw Error("Inexistent variable");
  }
  return *itV;
};

namespace {
template <typename Predicate>
void for_each_nested_with_name(const xmlPrs::Tag &father,
                               const std::string &name, Predicate predicate) {
  auto range = father.getNested().equal_range(name);
  for (auto it = range.first; it != range.second; ++it) {
    predicate(*it->second);
  }
}

template <typename Predicate>
void for_each_attribute_with_name(const xmlPrs::Tag &subject,
                                  const std::string &name,
                                  Predicate predicate) {
  auto range = subject.getAttributes().equal_range(name);
  std::for_each(
      range.first, range.second,
      [&predicate](const std::pair<xmlPrs::Name, std::string> &element) {
        predicate(element.second);
      });
}
} // namespace

categoric::Group
importGroup(const xmlPrs::Tag &tag,
            const std::set<categoric::VariablePtr> &variables) {
  auto vars = tag.getAttributes().equal_range("var");
  if ((std::distance(vars.first, vars.second) != 1) &&
      (std::distance(vars.first, vars.second) != 2)) {
    throw Error("only unary or binary factor are supported");
  }
  auto itV = vars.first;
  categoric::Group group(findVariable(itV->second, variables));
  ++itV;
  for (itV; itV != vars.second; ++itV) {
    group.add(findVariable(itV->second, variables));
  }
  return group;
};

std::shared_ptr<distribution::factor::cnst::Factor>
importFactor(const std::string &prefix, const xmlPrs::Tag &tag,
             const std::set<categoric::VariablePtr> &variables) {
  auto group = importGroup(tag, variables);

  const auto *corr = findAttribute(tag, "Correlation");
  if (nullptr != corr) {
    if (0 == corr->compare("T")) {
      return std::make_shared<distribution::factor::cnst::Factor>(
          group.getVariables(), true);
    }
    if (0 == corr->compare("F")) {
      return std::make_shared<distribution::factor::cnst::Factor>(
          group.getVariables(), false);
    }
    throw Error("invalid option for Correlation");
  }

  const auto *source = findAttribute(tag, "Source");
  if (nullptr != source) {
    return std::make_shared<distribution::factor::cnst::Factor>(
        group.getVariables(), prefix + "/" + *source);
  }

  std::shared_ptr<distribution::factor::modif::Factor> factor =
      std::make_shared<distribution::factor::modif::Factor>(
          group.getVariables());
  for_each_nested_with_name(
      tag, "Distr_val", [&factor](const xmlPrs::Tag &tag) {
        auto itComb = tag.getAttributes().equal_range("v");
        if (std::distance(itComb.first, itComb.second) !=
            factor->getGroup().getVariables().size()) {
          throw Error("invalid combination");
        }
        std::vector<std::size_t> comb;
        comb.clear();
        comb.reserve(std::distance(itComb.first, itComb.second));
        for (auto itC = itComb.first; itC != itComb.second; ++itC) {
          comb.push_back(std::atoi(itC->second.c_str()));
        }
        const auto *val = findAttribute(tag, "D");
        if (nullptr == val) {
          throw Error("image value not found");
        }
        factor->setImageRaw(categoric::Combination(comb.data(), comb.size()),
                            static_cast<float>(std::atof(val->c_str())));
      });
  return factor;
};

distribution::DistributionPtr
importDistribution(const std::string &prefix, const xmlPrs::Tag &tag,
                   const std::set<categoric::VariablePtr> &variables) {
  auto shape = importFactor(prefix, tag, variables);
  const auto *w = findAttribute(tag, "weight");
  if (nullptr == w) {
    return shape;
  }

  const auto *tunab = findAttribute(tag, "tunability");
  if (nullptr == tunab) {
    return std::make_shared<distribution::factor::cnst::FactorExponential>(
        *shape, static_cast<float>(std::atof(w->c_str())));
  }
  return std::make_shared<distribution::factor::modif::FactorExponential>(
      *shape, static_cast<float>(std::atof(w->c_str())));
};

std::map<std::string, std::size_t> Importer::importComponents(
    const std::string &filePath, const std::string &fileName,
    const std::pair<strct::InsertCapable *, strct::InsertTunableCapable *>
        &components) {
  auto parsed_raw = xmlPrs::parse_xml(filePath + "/" + fileName);
  auto *as_error = std::get_if<xmlPrs::Error>(&parsed_raw);
  if (as_error != nullptr) {
    throw Error{as_error->what()};
  }
  auto &parsed_root = std::get<xmlPrs::Root>(parsed_raw);
  // import variables
  std::set<categoric::VariablePtr> variables;
  std::map<std::string, std::size_t> evidence;
  for_each_nested_with_name(
      parsed_root, "Variable", [&variables, &evidence](const xmlPrs::Tag &tag) {
        const auto *name = findAttribute(tag, "name");
        if (nullptr == name) {
          throw Error("variable name not found");
        }
        const auto *size = findAttribute(tag, "Size");
        if (nullptr == size) {
          throw Error("variable size not found");
        }
        categoric::VariablePtr newVar =
            categoric::makeVariable(std::atoi(size->c_str()), *name);
        if (variables.find(newVar) != variables.end()) {
          throw Error(
              "Found multiple variables with same name in the passed xml");
        }
        variables.emplace(newVar);
        const auto *flag = findAttribute(tag, "flag");
        if ((nullptr != flag) && (0 == flag->compare("O"))) {
          evidence.emplace(newVar->name(), 0);
        }
      });
  // import potentials
  for_each_nested_with_name(
      parsed_root, "Potential",
      [&filePath, &variables, &components](const xmlPrs::Tag &tag) {
        auto distribution = importDistribution(filePath, tag, variables);
        distribution::factor::cnst::FactorExponential *expCnstPtr =
            dynamic_cast<distribution::factor::cnst::FactorExponential *>(
                distribution.get());
        distribution::factor::modif::FactorExponential *expPtr =
            dynamic_cast<distribution::factor::modif::FactorExponential *>(
                distribution.get());
        if (nullptr == expCnstPtr) {
          // import as non tunable factor
          components.first->insertCopy(*distribution);
        } else if ((nullptr == expPtr) || (nullptr == components.second)) {
          // import as an exponential non tunable factor
          components.first->insertCopy(*expCnstPtr);
        } else {
          // import as a tunable factor
          auto shareTag_it = tag.getNested().find("Share");
          if (shareTag_it == tag.getNested().end()) {
            components.second->insertTunableCopy(*expPtr);
          } else {
            components.second->insertTunableCopy(
                *expPtr,
                importGroup(*shareTag_it->second, variables).getVariables());
          }
        }
      });
  return evidence;
}
} // namespace EFG::io::xml
