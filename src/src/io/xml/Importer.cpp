/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#include <EasyFactorGraph/io/FactorImporter.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <XML-Parser/Parser.h>

#include <variant>

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
    throw Error{name, " is an inexistent attribute"};
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
  auto itV = variables.find(categoric::make_variable(2, name));
  if (itV == variables.end()) {
    throw Error("Inexistent variable");
  }
  return *itV;
};

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
};

std::shared_ptr<distribution::Factor>
importFactor(const std::string &prefix, const xmlPrs::Tag &tag,
             const categoric::VariablesSet &variables) {
  auto group = importGroup(tag, variables);

  const auto *corr = try_access_attribute(tag, "Correlation");
  if (nullptr != corr) {
    if (*corr == "T") {
      return std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()},
          distribution::USE_SIMPLE_CORRELATION_TAG);
    }
    if (*corr == "F") {
      return std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()},
          distribution::USE_SIMPLE_ANTI_CORRELATION_TAG);
    }
    throw Error("invalid option for Correlation");
  }

  std::shared_ptr<distribution::Factor> result =
      std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()});

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
                 result->setImageRaw(
                     categoric::Combination{std::move(combination)}, val);
               });

  return result;
};

struct FactorAndSharingGroup {
  std::shared_ptr<distribution::FactorExponential> factor;
  std::optional<categoric::VariablesSet> sharing_group;
};
using PotentialToInsert =
    std::variant<distribution::DistributionCnstPtr, FactorAndSharingGroup>;
PotentialToInsert importPotential(const std::string &prefix,
                                  const xmlPrs::Tag &tag,
                                  const categoric::VariablesSet &variables) {
  auto shape = importFactor(prefix, tag, variables);
  const auto *w = try_access_attribute(tag, "weight");
  if (nullptr == w) {
    return shape;
  }

  const auto *tunab = try_access_attribute(tag, "tunability");
  if ((nullptr != tunab) && (*tunab == "Y")) {
    FactorAndSharingGroup result;
    result.factor = std::make_shared<distribution::FactorExponential>(
        *shape, static_cast<float>(std::atof(w->c_str())));
    auto share_tag_it = tag.getNested().find("Share");
    if (share_tag_it != tag.getNested().end()) {
      result.sharing_group.emplace(
          importGroup(*share_tag_it->second, variables).getVariablesSet());
    }
    return result;
  }
  distribution::DistributionCnstPtr result =
      std::make_shared<distribution::FactorExponential>(
          *shape, static_cast<float>(std::atof(w->c_str())));
  return result;
};
} // namespace

std::unordered_set<std::string> Importer::convert(const AdderPtrs &subject,
                                                  const File &file_path) {
  auto maybe_parsed_root = xmlPrs::parse_xml(file_path.str());
  auto maybe_parsed_error = std::get_if<xmlPrs::Error>(&maybe_parsed_root);
  if (nullptr != maybe_parsed_error) {
    throw *maybe_parsed_error;
  }
  const auto &parsed_root = std::get<xmlPrs::Root>(maybe_parsed_root);
  // import variables
  categoric::VariablesSet variables;
  std::unordered_set<std::string> evidences;
  for_each_key(
      parsed_root.getNested(), xmlPrs::Name{"Variable"},
      [&variables, &evidences](const xmlPrs::TagPtr &var) {
        const auto &name = access_attribute(*var, "name");
        const auto &size = access_attribute(*var, "Size");
        auto new_var = categoric::make_variable(std::atoi(size.c_str()), name);
        if (variables.find(new_var) != variables.end()) {
          throw Error{name, " is a multiple times specified variable "};
        }
        variables.emplace(new_var);
        const auto *obs_flag = try_access_attribute(*var, "flag");
        if ((nullptr != obs_flag) && (*obs_flag == "O")) {
          evidences.emplace(name);
        }
      });
  // import potentials
  std::vector<FactorAndSharingGroup> tunable_sharing_group;
  for_each_key(
      parsed_root.getNested(), xmlPrs::Name{"Potential"},
      [&file_path, &variables, &subject,
       &tunable_sharing_group](const xmlPrs::TagPtr &factor) {
        auto to_insert =
            importPotential(file_path.parent_str(), *factor, variables);

        auto *as_const_distr =
            std::get_if<distribution::DistributionCnstPtr>(&to_insert);
        if (nullptr != as_const_distr) {
          subject.as_factors_const_adder->addConstFactor(*as_const_distr);
          return;
        }

        auto &as_tunable = std::get<FactorAndSharingGroup>(to_insert);
        if (nullptr == subject.as_factors_tunable_adder) {
          subject.as_factors_const_adder->addConstFactor(as_tunable.factor);
          return;
        }

        if (as_tunable.sharing_group == std::nullopt) {
          subject.as_factors_tunable_adder->addTunableFactor(as_tunable.factor);
          return;
        }

        tunable_sharing_group.push_back(as_tunable);
      });
  for (const auto &tunable : tunable_sharing_group) {
    subject.as_factors_tunable_adder->addTunableFactor(tunable.factor,
                                                       tunable.sharing_group);
  }
  return evidences;
}
} // namespace EFG::io::xml
#endif
