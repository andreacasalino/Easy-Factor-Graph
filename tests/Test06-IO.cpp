#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/io/xml/Exporter.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/RandomField.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::strct;
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
  model.addConstFactor(make_test_factor(V3, O));
  model.addTunableFactor(make_test_factor_exp(V0, O, alfa));
  model.addTunableFactor(make_test_factor_exp(V1, V2, beta));
  model.addTunableFactor(make_test_factor_exp(V2, V3, gamma));
  model.addTunableFactor(make_test_factor_exp(V3, V0, gamma),
                         VariablesSet{V2, V3});
  model.setEvidence(O, 1);
  return model;
}
} // namespace

TEST_CASE("xml managing", "[io][xml]") {
  auto model = make_test_model();
  const std::string temp_file = "temp_xml.xml";

  EFG::io::xml::Exporter::exportToFile(
      model, EFG::io::xml::ExportInfo{temp_file, "Model"});

  RandomField model_imported;
  EFG::io::xml::Importer::importFromFile(model_imported, temp_file);

  // chek the 2 models are identical in:
  //  - variables
  //  - factors const
  //  - factors tunable clsuters
  //  - evidences
}

#include <EasyFactorGraph/io/json/Exporter.h>
#include <EasyFactorGraph/io/json/Importer.h>

TEST_CASE("json managing", "[io][json]") {
  auto model = make_test_model();
  const std::string temp_file = "temp_xml.json";

  EFG::io::json::Exporter::exportToFile(model, temp_file);

  RandomField model_imported;
  EFG::io::json::Importer::importFromFile(model_imported, temp_file);

  // chek the 2 models are identical in:
  //  - variables
  //  - factors const
  //  - factors tunable clsuters
  //  - evidences
}
