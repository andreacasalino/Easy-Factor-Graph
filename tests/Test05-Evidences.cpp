#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <sstream>

#include "Utils.h"
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsConstManager.h>

namespace EFG::test {
using namespace categoric;
using namespace factor;
using namespace strct;
using namespace test;

namespace {
class EvidenceTest : public EvidenceSetter,
                     public EvidenceRemover,
                     protected FactorsConstInserter {
public:
  VariablesSoup uVars;
  VariablesSoup mVars;
  VariablesSoup lVars;

  EvidenceTest() {
    auto connect = [this](const VariablePtr &a, const VariablePtr &b) {
      addConstFactor(make_corr_factor_ptr(a, b));
    };

    auto createVariable = [](const std::string &name, std::size_t id) {
      std::stringstream s;
      s << name << id;
      return make_variable(2, s.str());
    };

    this->uVars.reserve(3);
    this->mVars.reserve(4);
    this->lVars.reserve(3);

    this->uVars.push_back(createVariable("A", 0));
    this->mVars.push_back(createVariable("M", 0));
    this->lVars.push_back(createVariable("L", 0));
    connect(this->uVars.back(), this->mVars.back());
    connect(this->lVars.back(), this->mVars.back());

    for (std::size_t k = 1; k < 3; ++k) {
      std::size_t s = this->mVars.size();
      this->uVars.push_back(createVariable("A", k));
      this->mVars.push_back(createVariable("M", k));
      this->lVars.push_back(createVariable("L", k));
      connect(this->uVars.back(), this->mVars.back());
      connect(this->lVars.back(), this->mVars.back());
      connect(this->uVars[s - 1], this->mVars.back());
      connect(this->lVars[s - 1], this->mVars.back());
    }
  };

  void clusterExists(const VariablesSet &vars) {
    auto convert = [](const std::unordered_set<Node *> &nodes) {
      VariablesSet res;
      for (auto *node : nodes) {
        res.emplace(node->variable);
      }
      return res;
    };
    const auto &clusters = state().clusters;
    auto it =
        std::find_if(clusters.begin(), clusters.end(),
                     [&](const auto &cl) { return convert(cl.nodes) == vars; });
    if (it == clusters.end()) {
      std::stringstream stream;
      stream << "Hidden cluster: <";
      for (const auto &var : vars) {
        stream << ' ' << var->name();
      }
      stream << '>';
      stream << " was not found";
      throw Error{stream.str()};
    }
  };
};
} // namespace

TEST_CASE("testing evidence managing", "[evidence]") {
  EvidenceTest model;
  model.clusterExists(VariablesSet{model.getAllVariables().begin(),
                                   model.getAllVariables().end()});
  CHECK(model.getEvidences().empty());

  model.setEvidence(model.mVars[1], 0);
  model.clusterExists(
      VariablesSet{model.uVars[0], model.mVars[0], model.lVars[0]});
  model.clusterExists(VariablesSet{model.uVars[2], model.mVars[2],
                                   model.lVars[2], model.uVars[1],
                                   model.lVars[1]});
  {
    Evidences expected;
    expected.emplace(model.mVars[1], 0);
    CHECK(model.getEvidences() == expected);
  }

  model.setEvidence(model.mVars[2], 1);
  model.clusterExists(
      VariablesSet{model.uVars[0], model.mVars[0], model.lVars[0]});
  model.clusterExists(VariablesSet{model.uVars[1]});
  model.clusterExists(VariablesSet{model.uVars[2]});
  model.clusterExists(VariablesSet{model.lVars[1]});
  model.clusterExists(VariablesSet{model.lVars[2]});
  {
    Evidences expected;
    expected.emplace(model.mVars[1], 0);
    expected.emplace(model.mVars[2], 1);
    CHECK(model.getEvidences() == expected);
  }
}

TEST_CASE("evidence individual reset", "[evidence]") {
  EvidenceTest model;

  model.setEvidence(model.mVars[1], 0);
  model.setEvidence(model.mVars[2], 0);

  model.removeEvidence(model.mVars[2]);
  model.clusterExists(
      VariablesSet{model.uVars[0], model.mVars[0], model.lVars[0]});
  model.clusterExists(VariablesSet{model.uVars[1], model.uVars[2],
                                   model.lVars[1], model.lVars[2],
                                   model.mVars[2]});
  {
    Evidences expected;
    expected.emplace(model.mVars[1], 0);
    CHECK(model.getEvidences() == expected);
  }

  model.removeEvidence(model.mVars[1]);
  model.clusterExists(VariablesSet{model.getAllVariables().begin(),
                                   model.getAllVariables().end()});
  CHECK(model.getEvidences().empty());
}

TEST_CASE("evidence group reset", "[evidence]") {
  EvidenceTest model;

  model.setEvidence(model.mVars[1], 0);
  model.setEvidence(model.mVars[2], 0);

  model.removeEvidences(VariablesSet{model.mVars[1], model.mVars[2]});
  model.clusterExists(VariablesSet{model.getAllVariables().begin(),
                                   model.getAllVariables().end()});
  CHECK(model.getEvidences().empty());
}

TEST_CASE("evidence total reset", "[evidence]") {
  EvidenceTest model;

  model.setEvidence(model.mVars[1], 0);
  model.setEvidence(model.mVars[2], 0);

  model.removeAllEvidences();
  model.clusterExists(VariablesSet{model.getAllVariables().begin(),
                                   model.getAllVariables().end()});
  CHECK(model.getEvidences().empty());
  CHECK_THROWS_AS(model.removeEvidence(model.mVars[2]), Error);
}

} // namespace EFG::test
