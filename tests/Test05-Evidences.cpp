#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <sstream>

#include <ModelLibrary.h>
#include <Utils.h>

#include <EasyFactorGraph/structure/EvidenceManager.h>

namespace EFG::test {
using namespace categoric;
using namespace factor;
using namespace structure;
using namespace test;

TEST_CASE("no evidence set", "[evidence]") {
  EvidenceTestModel model;

  CHECK(model.allHiddenCluserExists());
}

TEST_CASE("testing evidence managing", "[evidence]") {
  EvidenceTestModel model;
  const auto &strct = model.getStructure();

  // Set first evidence
  model.setEvidences(Evidence{model.mVars[1], 0});

  model.clusterExists({model.lVars[0], model.mVars[0], model.uVars[0]});
  model.clusterExists({
      model.lVars[1],
      model.lVars[2],
      model.lVars[3],
      model.uVars[1],
      model.uVars[2],
      model.uVars[3],
      model.mVars[2],
      model.mVars[3],
  });
  CHECK(strct.nodes[model.mVars[1]].evidence == 0);

  // Set second evidence
  model.setEvidences(Evidence{model.mVars[2], 1});

  model.clusterExists({model.lVars[0], model.mVars[0], model.uVars[0]});
  model.clusterExists({model.lVars[1], model.mVars[1], model.uVars[1]});
  model.clusterExists({
      model.lVars[2],
      model.lVars[3],
      model.uVars[2],
      model.uVars[3],
      model.mVars[3],
  });
  CHECK(strct.nodes[model.mVars[1]].evidence == 0);
  CHECK(strct.nodes[model.mVars[2]].evidence == 1);
}

TEST_CASE("evidence individual reset", "[evidence]") {
  EvidenceTestModel model;
  const auto &strct = model.getStructure();

  model.setEvidences(Evidence{model.mVars[1], 0}, Evidence{model.mVars[2], 0});

  // Remove first evidence
  model.removeEvidences(model.mVars[2]);

  model.clusterExists({model.lVars[0], model.mVars[0], model.uVars[0]});
  model.clusterExists({
      model.lVars[1],
      model.lVars[2],
      model.lVars[3],
      model.uVars[1],
      model.uVars[2],
      model.uVars[3],
      model.mVars[2],
      model.mVars[3],
  });
  CHECK(strct.nodes[model.mVars[1]].evidence == 0);
  CHECK(strct.nodes[model.mVars[2]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);

  // Remove second evidence
  model.removeEvidences(model.mVars[1]);

  CHECK(model.allHiddenCluserExists());
  CHECK(strct.nodes[model.mVars[1]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
  CHECK(strct.nodes[model.mVars[2]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
}

TEST_CASE("evidence group reset", "[evidence]") {
  EvidenceTestModel model;
  const auto &strct = model.getStructure();

  model.setEvidences(Evidence{model.mVars[1], 0}, Evidence{model.mVars[2], 0});

  model.removeEvidences(model.mVars[1], model.mVars[2]);

  CHECK(model.allHiddenCluserExists());
  CHECK(strct.nodes[model.mVars[1]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
  CHECK(strct.nodes[model.mVars[2]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
}

TEST_CASE("evidence total reset", "[evidence]") {
  EvidenceTestModel model;
  const auto &strct = model.getStructure();

  model.setEvidences(Evidence{model.mVars[1], 0}, Evidence{model.mVars[2], 0});

  model.removeAllEvidences();

  CHECK(model.allHiddenCluserExists());
  CHECK(strct.nodes[model.mVars[1]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
  CHECK(strct.nodes[model.mVars[2]].evidence ==
        structure::Evidence::NOT_AN_EVIDENCE);
}
} // namespace EFG::test
