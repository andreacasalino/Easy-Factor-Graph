#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/io/xml/Exporter.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>
#include <EasyFactorGraph/model/RandomField.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::strct;
using namespace EFG::train;
using namespace EFG::model;
using namespace EFG::distribution;

namespace {
std::shared_ptr<Factor> make_test_factor(const VariablePtr &first,
                                         const VariablePtr &second) {
  return std::make_shared<Factor>(Group{first, second},
                                  USE_SIMPLE_CORRELATION_TAG);
}

std::shared_ptr<FactorExponential>
make_test_factor_exp(const VariablePtr &first, const VariablePtr &second,
                     const float w) {
  return std::make_shared<FactorExponential>(*make_test_factor(first, second),
                                             w);
}

constexpr float alfa = 0.5f;
constexpr float beta = 0.7f;
constexpr float gamma = 1.2f;

RandomField make_test_model() {
  auto V0 = make_variable(3, "V0");
  auto V1 = make_variable(3, "V1");
  auto V2 = make_variable(3, "V2");
  auto V3 = make_variable(3, "V3");
  auto O = make_variable(3, "O");

  RandomField model;
  model.addConstFactor(make_test_factor(V0, O));
  model.addConstFactor(make_test_factor(V1, O));
  model.addConstFactor(make_test_factor(V2, O));
  model.addConstFactor(make_test_factor(V3, O));
  model.addTunableFactor(make_test_factor_exp(V0, V1, alfa));
  model.addTunableFactor(make_test_factor_exp(V1, V2, beta));
  model.addTunableFactor(make_test_factor_exp(V2, V3, gamma));
  model.addTunableFactor(make_test_factor_exp(V3, V0, gamma),
                         VariablesSet{V2, V3});
  model.setEvidence(O, 1);
  return model;
}

template <typename ContainerT>
bool exist_factor_with_group(const ContainerT &subject, const StateAware &model,
                             const std::string &first,
                             const std::string &second) {
  VariablesSet group;
  group.emplace(model.findVariable(first));
  group.emplace(model.findVariable(second));
  for (const auto &element : subject) {
    const distribution::Distribution &distr = *element;
    if (group == distr.getVariables().getVariablesSet()) {
      return true;
    }
  }
  return false;
}

bool check_imported_variables(const StateAware &model) {
  const auto vars = model.getAllVariables();
  const std::vector<std::string> expected_vars = {"V0", "V1", "V2", "V3", "O"};
  if (expected_vars.size() != vars.size()) {
    return false;
  }
  for (const auto &var_name : expected_vars) {
    auto vars_it = std::find_if(vars.begin(), vars.end(),
                                [&var_name](const VariablePtr &var) {
                                  return var_name == var->name();
                                });
    if (vars_it == vars.end()) {
      return false;
    }
  }
  return true;
}

struct VarPair {
  std::string first;
  std::string second;
};

bool check_imported_const_factors(const FactorsAware &model) {
  const auto const_factors = model.getConstFactors();
  if (const_factors.size() != 4) {
    return false;
  }
  const std::vector<VarPair> expected_groups = {
      {"V0", "O"}, {"V1", "O"}, {"V2", "O"}, {"V3", "O"}};
  for (const auto &group : expected_groups) {
    if (!exist_factor_with_group(const_factors, model, group.first,
                                 group.second)) {
      return false;
    }
  }
  return true;
}

bool check_imported_tunable_factors(const FactorsTunableAware &model) {
  const auto tunable_factors = model.getTunableFactors();
  if (tunable_factors.size() != 4) {
    return false;
  }
  const std::vector<VarPair> expected_groups = {
      {"V0", "V1"}, {"V1", "V2"}, {"V2", "V3"}, {"V3", "V0"}};
  for (const auto &group : expected_groups) {
    if (!exist_factor_with_group(tunable_factors, model, group.first,
                                 group.second)) {
      return false;
    }
  }
  return true;
}

bool check_imported_evidences(const StateAware &model) {
  const auto &evidences = model.getEvidences();
  if (evidences.size() != 1) {
    return false;
  }
  if (evidences.begin()->first != model.findVariable("O")) {
    return false;
  }
  if (evidences.begin()->second != 1) {
    return false;
  }
  return true;
}

bool check_imported_clusters(const RandomField &model) {
  const auto w = model.getWeights();
  if (w.size() != 3) {
    return false;
  }
  std::vector<float> expected_w = {alfa, beta, gamma};
  for (const auto val : expected_w) {
    if (std::find(w.begin(), w.end(), val) == w.end()) {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("xml managing", "[io][xml]") {
  auto model = make_test_model();
  const std::string temp_file = "./temp.xml";

  EFG::io::xml::Exporter::exportToFile(
      model, EFG::io::xml::ExportInfo{temp_file, "Model"});

  SECTION("tunable model") {
    RandomField model_imported;
    EFG::io::xml::Importer::importFromFile(model_imported, temp_file);

    CHECK(check_imported_variables(model_imported));
    CHECK(check_imported_const_factors(model_imported));
    CHECK(check_imported_tunable_factors(model_imported));
    CHECK(check_imported_evidences(model_imported));
    CHECK(check_imported_clusters(model_imported));
  }

  SECTION("constant model") {
    Graph model_imported;
    EFG::io::xml::Importer::importFromFile(model_imported, temp_file);

    CHECK(check_imported_variables(model_imported));
    CHECK(model_imported.getConstFactors().size() == 8);
    CHECK(check_imported_evidences(model_imported));
  }
}

#include <EasyFactorGraph/io/json/Exporter.h>
#include <EasyFactorGraph/io/json/Importer.h>

TEST_CASE("json managing", "[io][json]") {
  auto model = make_test_model();
  const std::string temp_file = "./temp.json";

  EFG::io::json::Exporter::exportToFile(model, temp_file);

  SECTION("tunable model") {
    RandomField model_imported;
    EFG::io::json::Importer::importFromFile(model_imported, temp_file);

    CHECK(check_imported_variables(model_imported));
    CHECK(check_imported_const_factors(model_imported));
    CHECK(check_imported_tunable_factors(model_imported));
    CHECK(check_imported_evidences(model_imported));
    CHECK(check_imported_clusters(model_imported));
  }

  SECTION("constant model") {
    Graph model_imported;
    EFG::io::json::Importer::importFromFile(model_imported, temp_file);

    CHECK(check_imported_variables(model_imported));
    CHECK(model_imported.getConstFactors().size() == 8);
    CHECK(check_imported_evidences(model_imported));
  }
}