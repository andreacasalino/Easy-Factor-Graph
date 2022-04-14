/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#include <EasyFactorGraph/io/FactorImporter.h>
#include <EasyFactorGraph/io/json/Importer.h>

#include "../ImportUtils.h"

namespace EFG::io::json {
namespace {
const nlohmann::json *try_access(const nlohmann::json &subject,
                                 const std::string &name) {
  if (subject.contains(name)) {
    return &subject[name];
  }
  return nullptr;
}

const nlohmann::json &access(const nlohmann::json &subject,
                             const std::string &name) {
  const auto *attr = try_access(subject, name);
  if (nullptr == attr) {
    throw Error{name, " is inexistent"};
  }
  return *attr;
}

std::string to_string(const nlohmann::json &subject) {
  if (subject.is_string()) {
    auto raw_dump = subject.dump();
    return std::string{raw_dump, 1, raw_dump.size() - 2};
  }
  throw Error{"Expected a string"};
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
};

categoric::Group importGroup(const nlohmann::json &subject,
                             const categoric::VariablesSet &variables) {
  categoric::VariablesSoup group;
  for (const auto &var : subject) {
    group.push_back(findVariable(to_string(var), variables));
  }
  if ((group.size() != 1) && (group.size() != 2)) {
    throw Error("only unary or binary factor are supported");
  }
  return categoric::Group{group};
};

std::shared_ptr<distribution::Factor>
importFactor(const nlohmann::json &subject,
             const categoric::VariablesSet &variables) {
  auto group = importGroup(access(subject, "Variables"), variables);

  const auto *corr = try_access(subject, "Correlation");
  if (nullptr != corr) {
    if (to_string(*corr) == "T") {
      return std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()},
          distribution::USE_SIMPLE_CORRELATION_TAG);
    }
    if (to_string(*corr) == "F") {
      return std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()},
          distribution::USE_SIMPLE_ANTI_CORRELATION_TAG);
    }
    throw Error("invalid option for Correlation");
  }

  std::shared_ptr<distribution::Factor> result =
      std::make_shared<distribution::Factor>(
          categoric::Group{group.getVariables()});

  for (const auto &comb : access(subject, "Distr_val")) {
    std::vector<std::size_t> combination;
    for (const auto &v : comb["v"]) {
      combination.push_back(std::atoi(to_string(v).c_str()));
    }
    const float val =
        static_cast<float>(std::atof(to_string(access(comb, "D")).c_str()));
    result->setImageRaw(categoric::Combination{std::move(combination)}, val);
  }

  return result;
};

void importPotential(const nlohmann::json &subject,
                     const categoric::VariablesSet &variables,
                     ImportPredicate &importer) {
  auto shape = importFactor(subject, variables);
  const auto *w = try_access(subject, "weight");
  if (nullptr == w) {
    importer.importConst(shape);
    return;
  }

  auto factor = std::make_shared<distribution::FactorExponential>(
      *shape, static_cast<float>(std::atof(to_string(*w).c_str())));
  const auto *tunab = try_access(subject, "tunability");
  if ((nullptr != tunab) && (to_string(*tunab) == "Y")) {
    const auto *share_tag = try_access(subject, "Share");
    if (nullptr == share_tag) {
      importer.importTunable(factor);
    } else {
      auto group = importGroup(*share_tag, variables).getVariablesSet();
      importer.importTunable(factor, group);
    }
    return;
  }
  importer.importConst(factor);
};
} // namespace

std::unordered_map<std::string, std::size_t>
Importer::convert(const AdderPtrs &recipient, const nlohmann::json &source) {
  // import variables
  categoric::VariablesSet variables;
  std::unordered_map<std::string, std::size_t> evidences;
  for (const auto &var : source["Variables"]) {
    const auto name = to_string(access(var, "name"));
    const auto size = to_string(access(var, "Size"));
    auto new_var = categoric::make_variable(std::atoi(size.c_str()), name);
    if (variables.find(new_var) != variables.end()) {
      throw Error{name, " is a multiple times specified variable "};
    }
    variables.emplace(new_var);
    const auto *obs_flag = try_access(var, "evidence");
    if (nullptr != obs_flag) {
      const std::size_t val =
          static_cast<std::size_t>(std::atoi(to_string(*obs_flag).c_str()));
      evidences.emplace(name, val);
    }
  }
  // import potentials
  ImportPredicate importer{recipient};
  for (const auto &factor : source["Potentials"]) {
    importPotential(factor, variables, importer);
  }
  importer.importCumulatedTunable();
  return evidences;
}
} // namespace EFG::io::json

#endif
