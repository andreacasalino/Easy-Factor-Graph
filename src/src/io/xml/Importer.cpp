/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#include <EasyFactorGraph/io/xml/Importer.h>

#include "../Utils.h"

#include <XML-Parser/Parser.h>

namespace EFG::io::xml {
namespace {
const std::string *try_access_attribute(const xmlPrs::Tag &subject,
                                        const std::string &name) {
  auto it = subject.getAttributes().find(name);
  if (it == subject.getAttributes().end()) {
    return nullptr;
  }
  return &it->second;
}

const std::string &access_attribute(const xmlPrs::Tag &subject,
                                    const std::string &name) {
  const auto *attr = try_access_attribute(subject, name);
  if (nullptr == attr) {
    throw Error::make(name, " is an inexistent attribute");
  }
  return *attr;
}

template <typename K, typename V, typename Predicate>
void for_each_key(const std::unordered_multimap<K, V> &subject, const K &key,
                  const Predicate &pred) {
  auto range = subject.equal_range(key);
  for (auto it = range.first; it != range.second; ++it) {
    pred(it->second);
  }
}

categoric::VariablePtr findVariable(const std::string &name,
                                    const categoric::VariablesSet &variables) {
  const auto itV = std::find_if(variables.begin(), variables.end(),
                                [&name](const categoric::VariablePtr &var) {
                                  return var->name() == name;
                                });
  if (itV == variables.end()) {
    throw Error("Inexistent variable");
  }
  return *itV;
}

categoric::Group importGroup(const xmlPrs::Tag &tag,
                             const categoric::VariablesSet &variables) {
  categoric::VariablesSoup group;
  for_each_key(tag.getAttributes(), xmlPrs::Name{"var"},
               [&variables, &group](const std::string &name) {
                 group.push_back(findVariable(name, variables));
               });
  if ((group.size() != 1) && (group.size() != 2)) {
    throw Error("only unary or binary factor are supported");
  }
  return categoric::Group{group};
}

std::shared_ptr<factor::Factor>
importFactor(const std::string &prefix, const xmlPrs::Tag &tag,
             const categoric::VariablesSet &variables) {
  auto group = importGroup(tag, variables);

  const auto *corr = try_access_attribute(tag, "Correlation");
  if (nullptr != corr) {
    if (*corr == "T") {
      return std::make_shared<factor::Factor>(
          categoric::Group{group.getVariables()},
          factor::Factor::SimplyCorrelatedTag{});
    }
    if (*corr == "F") {
      return std::make_shared<factor::Factor>(
          categoric::Group{group.getVariables()},
          factor::Factor::SimplyAntiCorrelatedTag{});
    }
    throw Error("invalid option for Correlation");
  }

  std::shared_ptr<factor::Factor> result =
      std::make_shared<factor::Factor>(categoric::Group{group.getVariables()});

  const auto *source = try_access_attribute(tag, "Source");
  if (nullptr != source) {
    import_values(*result, prefix + "/" + *source);
    return result;
  }

  for_each_key(tag.getNested(), xmlPrs::Name{"Distr_val"},
               [&result](const xmlPrs::TagPtr &comb) {
                 std::vector<std::size_t> combination;
                 for_each_key(comb->getAttributes(), xmlPrs::Name{"v"},
                              [&combination](const std::string &var) {
                                combination.push_back(std::atoi(var.c_str()));
                              });
                 const float val = static_cast<float>(
                     std::atof(access_attribute(*comb, "D").c_str()));

                 result->set(combination, val);
               });

  return result;
}

void importPotential(const std::string &prefix, const xmlPrs::Tag &tag,
                     const categoric::VariablesSet &variables,
                     ImportHelper &importer) {
  auto shape = importFactor(prefix, tag, variables);
  const auto *w = try_access_attribute(tag, "weight");
  if (nullptr == w) {
    importer.importConst(shape);
    return;
  }

  auto factor = std::make_shared<factor::FactorExponential>(
      *shape, static_cast<float>(std::atof(w->c_str())));
  const auto *tunab = try_access_attribute(tag, "tunability");
  if ((nullptr != tunab) && (*tunab == "Y")) {
    auto share_tag_it = tag.getNested().find("Share");
    if (share_tag_it == tag.getNested().end()) {
      importer.importTunable(factor);
    } else {
      auto group =
          importGroup(*share_tag_it->second, variables).getVariablesSet();
      importer.importTunable(factor, group);
    }
    return;
  }
  importer.importConst(factor);
}
} // namespace

std::unordered_map<std::string, std::size_t>
Importer::convert(Inserters subject, const std::filesystem::path &file_path) {
  auto maybe_parsed_root = xmlPrs::parse_xml(file_path.string());
  auto maybe_parsed_error = std::get_if<xmlPrs::Error>(&maybe_parsed_root);
  if (nullptr != maybe_parsed_error) {
    throw *maybe_parsed_error;
  }
  const auto &parsed_root = std::get<xmlPrs::Root>(maybe_parsed_root);
  // import variables
  categoric::VariablesSet variables;
  std::unordered_map<std::string, std::size_t> evidences;
  for_each_key(
      parsed_root.getNested(), xmlPrs::Name{"Variable"},
      [&variables, &evidences](const xmlPrs::TagPtr &var) {
        const auto &name = access_attribute(*var, "name");
        const auto &size = access_attribute(*var, "Size");
        auto new_var = categoric::make_variable(std::atoi(size.c_str()), name);
        if (variables.find(new_var) != variables.end()) {
          throw Error::make(name, " is a multiple times specified variable ");
        }
        variables.emplace(new_var);
        const auto *obs_flag = try_access_attribute(*var, "evidence");
        if (nullptr != obs_flag) {
          const std::size_t val =
              static_cast<std::size_t>(atoi(obs_flag->c_str()));
          evidences.emplace(name, val);
        }
      });
  // import potentials
  ImportHelper importer{subject};
  for_each_key(parsed_root.getNested(), xmlPrs::Name{"Potential"},
               [&](const xmlPrs::TagPtr &factor) {
                 importPotential(file_path.parent_path().string(), *factor, variables,
                                 importer);
               });
  importer.importCumulatedTunable();
  return evidences;
}
} // namespace EFG::io::xml
#endif
