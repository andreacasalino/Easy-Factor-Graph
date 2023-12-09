#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/model/Graph.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>

#include <EasyFactorGraph/io/json/Exporter.h>
#include <EasyFactorGraph/io/json/Importer.h>
#include <EasyFactorGraph/io/xml/Exporter.h>
#include <EasyFactorGraph/io/xml/Importer.h>

#include <algorithm>
#include <set>

namespace EFG::test {
using namespace categoric;
using namespace strct;
using namespace train;
using namespace model;
using namespace factor;
using namespace test;

namespace {
using VarNames = std::set<std::string>;

VarNames vars_names(const Group &subject) {
  VarNames res;
  for (const auto &var : subject.getVariables()) {
    res.emplace(var->name());
  }
  return res;
}

void sort_set(std::vector<VarNames> &subject) {
  auto convert = [](const VarNames &s) {
    return *s.begin() + " " + *s.rbegin();
  };

  std::sort(subject.begin(), subject.end(),
            [&convert](const VarNames &a, const VarNames &b) {
              return convert(a) < convert(b);
            });
}

template <typename ModelT> class TestModelBase : public ModelT {
public:
  TestModelBase() = default;

  bool checkVariables() {
    std::vector<std::string> vars;
    for (const auto &var : this->ModelT::getAllVariables()) {
      vars.push_back(var->name());
    }
    std::sort(vars.begin(), vars.end());
    std::vector<std::string> expected_vars = {"V0", "V1", "V2", "V3", "O"};
    std::sort(expected_vars.begin(), expected_vars.end());
    return vars == expected_vars;
  }

  bool checkEvidences() {
    std::map<std::string, std::size_t> ev, expected;
    for (const auto &[var, val] : this->ModelT::state().evidences) {
      ev[var->name()] = val;
    }
    expected.emplace("O", 1);
    return ev == expected;
  }
};

class TestModel : public TestModelBase<RandomField> {
public:
  static const inline float alfa = 0.5f;
  static const inline float beta = 0.7f;
  static const inline float gamma = 1.2f;

  TestModel() = default;

  struct FillTag {};
  TestModel(FillTag tag) {
    auto V0 = make_variable(3, "V0");
    auto V1 = make_variable(3, "V1");
    auto V2 = make_variable(3, "V2");
    auto V3 = make_variable(3, "V3");
    auto O = make_variable(3, "O");

    addConstFactor(make_corr_factor_ptr(V0, O));
    addConstFactor(make_corr_factor_ptr(V1, O));
    addConstFactor(make_corr_factor_ptr(V2, O));
    addConstFactor(make_corr_factor_ptr(V3, O));
    addTunableFactor(make_corr_expfactor_ptr(V0, V1, alfa));
    addTunableFactor(make_corr_expfactor_ptr(V1, V2, beta));
    addTunableFactor(make_corr_expfactor_ptr(V2, V3, gamma));
    addTunableFactor(make_corr_expfactor_ptr(V3, V0, gamma),
                     VariablesSet{V2, V3});
    setEvidence(O, 1);
  }

  VariablesSet makeVars(const std::string &first,
                        const std::string &second) const {
    return VariablesSet{findVariable(first), findVariable(second)};
  }

  bool checkConstFactors() {
    std::vector<std::set<std::string>> groups, expected;
    for (const auto &factor : const_factors) {
      groups.emplace_back(vars_names(factor->function().vars()));
    }
    expected = std::vector<VarNames>{
        {"V0", "O"}, {"V1", "O"}, {"V2", "O"}, {"V3", "O"}};

    sort_set(groups);
    sort_set(expected);
    return groups == expected;
  }

  bool checkTunablefactors() {
    auto sort_map = [](std::map<float, std::vector<VarNames>> &subject) {
      for (auto &[_, group] : subject) {
        sort_set(group);
      }
    };

    std::map<float, std::vector<VarNames>> groups, expected;

    for (const auto &tuner : tuners) {
      train::visitTuner(
          tuner.get(),
          [&groups](const train::BaseTuner &b) {
            groups[b.getWeight()].emplace_back(
                vars_names(b.getFactor().function().vars()));
          },
          [&groups](const train::CompositeTuner &c) {
            auto &collection = groups[c.getWeight()];
            for (const auto &f : c.getElements()) {
              collection.emplace_back(
                  vars_names(static_cast<const train::BaseTuner &>(*f)
                                 .getFactor()
                                 .function()
                                 .vars()));
            }
          });
    }

    expected = std::map<float, std::vector<VarNames>>{
        {alfa, std::vector<VarNames>{{"V0", "V1"}}},
        {beta, std::vector<VarNames>{{"V1", "V2"}}},
        {gamma, std::vector<VarNames>{{"V2", "V3"}, {"V3", "V0"}}}};

    sort_map(groups);
    sort_map(expected);
    return groups == expected;
  }
};
} // namespace

TEST_CASE("xml managing", "[io][xml]") {
  TestModel model{TestModel::FillTag{}};
  const std::string temp_file = "./temp.xml";

  io::xml::Exporter::exportToFile(model,
                                  EFG::io::xml::ExportInfo{temp_file, "Model"});

  SECTION("tunable model") {
    TestModel model_imported;
    EFG::io::xml::Importer::importFromFile(model_imported, temp_file);

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkEvidences());
    CHECK(model_imported.checkConstFactors());
    CHECK(model_imported.checkTunablefactors());
  }

  SECTION("constant model") {
    TestModelBase<Graph> model_imported;
    EFG::io::xml::Importer::importFromFile(model_imported, temp_file);

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkEvidences());
    CHECK(model_imported.getConstFactors().size() == 8);
    CHECK(model_imported.getAllFactors().size() == 8);
  }
}

TEST_CASE("json managing", "[io][json]") {
  TestModel model{TestModel::FillTag{}};
  const std::string temp_file = "./temp.json";

  EFG::io::json::Exporter::exportToFile(model, temp_file);

  SECTION("tunable model") {
    TestModel model_imported;
    EFG::io::json::Importer::importFromFile(model_imported, temp_file);

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkEvidences());
    CHECK(model_imported.checkConstFactors());
    CHECK(model_imported.checkTunablefactors());
  }

  SECTION("constant model") {
    TestModelBase<Graph> model_imported;
    EFG::io::json::Importer::importFromFile(model_imported, temp_file);

    CHECK(model_imported.checkVariables());
    CHECK(model_imported.checkEvidences());
    CHECK(model_imported.getConstFactors().size() == 8);
    CHECK(model_imported.getAllFactors().size() == 8);
  }
}
} // namespace EFG::test
