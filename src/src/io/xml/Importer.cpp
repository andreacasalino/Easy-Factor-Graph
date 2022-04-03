/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

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

std::shared_ptr<distribution::Factor>
importFactor(const std::string &prefix, const xmlPrs::Tag &tag,
             const categoric::VariablesSet &variables) {
  auto group = importGroup(tag, variables);

  const auto *corr = try_access_attribute(tag, "Correlation");
  if (nullptr != corr) {
    if (*corr == "T") {
      return std::make_shared<distribution::Factor>(
          group.getVariables(), distribution::USE_SIMPLE_CORRELATION_TAG);
    }
    if (*corr == "F") {
      return std::make_shared<distribution::Factor>(
          group.getVariables(), distribution::USE_SIMPLE_ANTI_CORRELATION_TAG);
    }
    throw Error("invalid option for Correlation");
  }

  std::shared_ptr<distribution::Factor> result =
      std::make_shared<distribution::Factor>(group.getVariables());

  const auto *source = try_access_attribute(tag, "Source");
  if (nullptr != source) {
    result->importCombiantionsFromFile(prefix + "/" + *source);
    return result;
  }

  auto comb_range = tag.getNested().equal_range("Distr_val");
  for (auto comb_it = comb_range.first; comb_it != comb_range.second;
       ++comb_it) {
    std::vector<std::size_t> combination;
    auto v_range = comb_it->second->getAttributes().equal_range("v");
    for (auto v_it = v_range.first; v_it != v_range.second; ++v_it) {
      combination.push_back(std::atoi(v_it->second.c_str()));
    }
    const float val = static_cast<float>(
        std::atof(access_attribute(*comb_it->second, "D").c_str()));
    result->setImageRaw(categoric::Combination{std::move(combination)}, val);
  }

  return result;
};

std::variant<std::shared_ptr<distribution::FactorExponential>,
             distribution::DistributionCnstPtr>
importDistribution(const std::string &prefix, const xmlPrs::Tag &tag,
                   const categoric::VariablesSet &variables) {
  auto shape = importFactor(prefix, tag, variables);
  const auto *w = try_access_attribute(tag, "weight");
  if (nullptr == w) {
    return shape;
  }

  const auto *tunab = try_access_attribute(tag, "tunability");
  if (nullptr == tunab) {
    distribution::DistributionCnstPtr result =
        std::make_shared<distribution::FactorExponential>(
            *shape, static_cast<float>(std::atof(w->c_str())));
    return result;
  }
  return std::make_shared<distribution::FactorExponential>(
      *shape, static_cast<float>(std::atof(w->c_str())));
};
} // namespace

std::unordered_set<std::string>
Importer::importComponents(const std::string &filePath,
                           const std::string &fileName,
                           const AdderPtrs &subject) {
  auto maybe_parsed_root = xmlPrs::parse_xml(filePath + "/" + fileName);
  auto maybe_parsed_error = std::get_if<xmlPrs::Error>(&maybe_parsed_root);
  if (nullptr != maybe_parsed_error) {
    throw *maybe_parsed_error;
  }
  const auto &parsed_root = std::get<xmlPrs::Root>(maybe_parsed_root);
  // import variables
  categoric::VariablesSet variables;
  std::unordered_set<std::string> evidences;
  {
    auto var_range = parsed_root.getNested().equal_range("Variable");
    for (auto var_it = var_range.first; var_it != var_range.second; ++var_it) {
      const auto &name = access_attribute(*var_it->second, "name");
      const auto &size = access_attribute(*var_it->second, "Size");
      auto new_var = categoric::make_variable(std::atoi(size.c_str()), name);
      if (variables.find(new_var) != variables.end()) {
        throw Error{name, " is a multiple times specified variable"};
      }
      variables.emplace(new_var);
      const auto *obs_flag = try_access_attribute(*var_it->second, "flag");
      if ((nullptr != obs_flag) && (*obs_flag == "O")) {
        evidences.emplace(name);
      }
    }
  }
  // import potentials
  {
    auto factor_range = parsed_root.getNested().equal_range("Potential");
    for (auto factor_it = factor_range.first; factor_it != factor_range.second;
         ++factor_it) {
      auto distribution =
          importDistribution(filePath, *factor_it->second, variables);

      struct Visitor {
        const AdderPtrs &subject;
        const xmlPrs::Tag &tag;
        const categoric::VariablesSet &variables;

        void operator()(const std::shared_ptr<distribution::FactorExponential>
                            &factor) const {
          if (nullptr == subject.as_factors_tunable_adder) {
            subject.as_factors_const_adder->addConstFactor(factor);
            return;
          }
          std::optional<categoric::VariablesSet> group_sharing_weight =
              std::nullopt;
          auto share_tag_it = tag.getNested().find("Share");
          if (share_tag_it != tag.getNested().end()) {
            auto &group = group_sharing_weight.emplace();
            auto it_var_range =
                share_tag_it->second->getAttributes().equal_range("var");
            for (auto it_var = it_var_range.first;
                 it_var != it_var_range.second; ++it_var) {
              group.emplace(findVariable(it_var->second, variables));
            }
          }
          subject.as_factors_tunable_adder->addTunableFactor(
              factor, group_sharing_weight);
        }

        void operator()(const distribution::DistributionCnstPtr &factor) const {
          subject.as_factors_const_adder->addConstFactor(factor);
        }
      };
      Visitor visitor = Visitor{subject, *factor_it->second, variables};
      std::visit(visitor, distribution);
    }
  }
  return evidences;
}
} // namespace EFG::io::xml
