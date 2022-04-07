#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

#include <math.h>
#include <sstream>

using namespace EFG;
using namespace EFG::model;
using namespace EFG::strct;
using namespace EFG::io;
using namespace EFG::categoric;

namespace {
std::string make_file_path(const std::string &file_name) {
  std::stringstream stream;
  stream << SAMPLE_FOLDER << "Sample03-BeliefPropagation-B/" << file_name;
  return stream.str();
}

std::vector<float>
normalize_distribution(const std::vector<float> &distribution) {
  float sum = 0;
  for (const auto &val : distribution) {
    sum += val;
  }
  if (sum < 0.001f) {
    return distribution;
  }
  std::vector<float> result;
  result.reserve(distribution.size());
  for (const auto &val : distribution) {
    result.push_back(val / sum);
  }
  return result;
}

bool almost_equal_distributions(const std::vector<float> &a,
                                const std::vector<float> &b,
                                const float threshold) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (abs(a[k] - b[k]) > threshold) {
      return false;
    }
  }
  return true;
}

class TestModels : public Graph {
public:
  TestModels(const std::string &file_name) {
    xml::Importer::importFromXml(*this, file_name);
  }

  // check all messages were computed after propagation
  bool araAllMessagesComputed() const {
    for (const auto &cluster : getState().clusters) {
      for (const auto *node : cluster.nodes) {
        for (const auto &[connected_node, connection] :
             node->active_connections) {
          if (connection.message == nullptr) {
            return false;
          }
        }
      }
    }
    return true;
  };

  bool checkMarginals(const std::string &var_name,
                      const std::vector<float> &expected,
                      const float threshold = 0.01f) {
    const auto var = findVariable(var_name);
    return almost_equal_distributions(normalize_distribution(expected),
                                      getMarginalDistribution(var_name),
                                      threshold);
  }
};

bool are_equal(const ClusterInfo &a, const ClusterInfo &b) {
  return (a.size == b.size) && (a.tree_or_loopy_graph == b.tree_or_loopy_graph);
}

bool are_equal(const std::vector<ClusterInfo> &a,
               const std::vector<ClusterInfo> &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (!are_equal(a[k], b[k])) {
      return false;
    }
  }
  return true;
}

bool are_equal(const PropagationResult &a, const PropagationResult &b) {
  return (a.propagation_kind_done == b.propagation_kind_done) &&
         (a.was_completed == b.was_completed) &&
         are_equal(a.structure_found, b.structure_found);
}
} // namespace

TEST_CASE("simple poly tree", "[propagation]") {
  TestModels model(make_file_path("graph_1.xml"));

  float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);

  REQUIRE_FALSE(model.hasPropagationResult());

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  CHECK(model.checkMarginals(
      "A", {(a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e))}));
  REQUIRE(model.hasPropagationResult());
  {
    const auto &propagation_result = model.getLastPropagationResult();
    strct::PropagationResult propagation_expected;
    propagation_expected.propagation_kind_done = PropagationKind::SUM;
    propagation_expected.was_completed = true;
    propagation_expected.structure_found =
        std::vector<ClusterInfo>{ClusterInfo{true, 4}};
    REQUIRE(are_equal(propagation_expected, propagation_result));
  }
  REQUIRE(model.araAllMessagesComputed());
  CHECK(model.checkMarginals("B", {(g + e), (1 + g * e)}));
  CHECK(model.checkMarginals(
      "C", {(b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e))}));
  CHECK(model.checkMarginals("D", {1.f, e}));

  // D=1
  model.setEvidence(model.findVariable("D"), 1);
  REQUIRE(model.hasPropagationResult());
  {
    const auto &propagation_result = model.getLastPropagationResult();
    strct::PropagationResult propagation_expected;
    propagation_expected.propagation_kind_done = PropagationKind::SUM;
    propagation_expected.was_completed = true;
    propagation_expected.structure_found =
        std::vector<ClusterInfo>{ClusterInfo{true, 3}, ClusterInfo{true, 1}};
    REQUIRE(are_equal(propagation_expected, propagation_result));
  }
  REQUIRE(model.araAllMessagesComputed());
  CHECK(model.checkMarginals("A", {a + g, 1.f + a * g}));
  CHECK(model.checkMarginals("B", {1.f, g}));
  CHECK(model.checkMarginals("C", {b + g, 1.f + b * g}));
  CHECK(model.checkMarginals("E", {1.f, e}));
}

TEST_CASE("complex poly tree", "[propagation]") {
  TestModels model(make_file_path("graph_2.xml"));
  model.setEvidence(model.findVariable("v1"), 1);
  model.setEvidence(model.findVariable("v2"), 1);
  model.setEvidence(model.findVariable("v3"), 1);

  auto threads = GENERATE(1, 4);

  {
    auto prob = model.getMarginalDistribution("v10", threads);
    CHECK(prob[0] < prob[1]);
  }
  {
    auto prob = model.getMarginalDistribution("v11", threads);
    CHECK(prob[0] < prob[1]);
  }
  {
    auto prob = model.getMarginalDistribution("v13", threads);
    CHECK(prob[0] < prob[1]);
  }

  CHECK(model.araAllMessagesComputed());
}

TEST_CASE("simple loopy tree", "[propagation]") {
  TestModels model(make_file_path("graph_3.xml"));

  float M = expf(1.f);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  CHECK(model.checkMarginals(
      "D", {3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2)}, 0.045f));
  REQUIRE(model.hasPropagationResult());
  {
    const auto &propagation_result = model.getLastPropagationResult();
    strct::PropagationResult propagation_expected;
    propagation_expected.propagation_kind_done = PropagationKind::SUM;
    propagation_expected.was_completed = true;
    propagation_expected.structure_found =
        std::vector<ClusterInfo>{ClusterInfo{false, 4}};
    REQUIRE(are_equal(propagation_expected, propagation_result));
  }
  REQUIRE(model.araAllMessagesComputed());
  CHECK(model.checkMarginals("C", {M_alfa, M_beta}, 0.045f));
  CHECK(model.checkMarginals("B", {M_alfa, M_beta}, 0.045f));
  CHECK(model.checkMarginals("A", {M * M_alfa + M_beta, M_alfa + M * M_beta},
                             0.045f));
}

TEST_CASE("complex loopy tree", "[propagation]") {
  TestModels model(make_file_path("graph_4.xml"));

  model.setEvidence(model.findVariable("v1"), 1);

  auto threads = GENERATE(1, 4);

  auto prob = model.getMarginalDistribution("v8", threads);
  CHECK(prob[0] < prob[1]);
  CHECK(model.araAllMessagesComputed());
}

#include <EasyFactorGraph/structure/SpecialFactors.h>

namespace {
model::Graph make_chain_model(const float wXY, const float wYY) {
  categoric::VariablesSoup Y = {make_variable(2, "Y0"), make_variable(2, "Y1"),
                                make_variable(2, "Y2"), make_variable(2, "Y3")};
  categoric::VariablesSoup X = {make_variable(2, "X0"), make_variable(2, "X1"),
                                make_variable(2, "X2"), make_variable(2, "X3")};

  model::Graph model;
  auto connect = [&model](const VariablePtr &a, const VariablePtr &b,
                          const float w) {
    distribution::Factor factor(Group{VariablesSoup{a, b}},
                                distribution::USE_SIMPLE_CORRELATION_TAG);
    model.copyConstFactor(distribution::FactorExponential(factor, w));
  };

  connect(X[0], Y[0], wXY);
  connect(X[1], Y[1], wXY);
  connect(X[2], Y[2], wXY);
  connect(X[3], Y[3], wXY);

  connect(Y[0], Y[1], wYY);
  connect(Y[1], Y[2], wYY);
  connect(Y[2], Y[3], wYY);

  model.copyConstFactor(
      distribution::FactorExponential(distribution::Indicator{Y[0], 1}, wYY));

  model.setEvidence(X[0], 0);
  model.setEvidence(X[1], 1);
  model.setEvidence(X[2], 0);
  model.setEvidence(X[3], 1);

  return model;
};
} // namespace

TEST_CASE("MAPTest", "[propagation]") {
  SECTION("strong weight between hidden") {
    auto model = make_chain_model(0.1f, 1.f);
    std::vector<std::size_t> mapExpected = {1, 1, 1, 1};
    CHECK(mapExpected == model.getHiddenSetMAP());
    CHECK(model.getLastPropagationResult().propagation_kind_done ==
          PropagationKind::MAP);
  }

  SECTION("strong weight for evidences") {
    auto model = make_chain_model(1.0f, 0.1f);
    std::vector<std::size_t> mapExpected = {0, 1, 0, 1};
    CHECK(mapExpected == model.getHiddenSetMAP());
  }
}

TEST_CASE("Sub graph distribution", "[propagation]") {
  VariablePtr A = make_variable(2, "A");
  VariablePtr B = make_variable(2, "B");
  VariablePtr C = make_variable(2, "C");
  VariablePtr D = make_variable(2, "D");
  float alfa = 0.5f, beta = 1.5f;
  // build the chain
  model::Graph graph;
  graph.addConstFactor(std::make_shared<distribution::FactorExponential>(
      distribution::Factor(Group{{A, B}},
                           distribution::USE_SIMPLE_CORRELATION_TAG),
      alfa));
  graph.addConstFactor(std::make_shared<distribution::FactorExponential>(
      distribution::Factor(Group{{B, C}},
                           distribution::USE_SIMPLE_CORRELATION_TAG),
      alfa));
  graph.addConstFactor(std::make_shared<distribution::FactorExponential>(
      distribution::Factor(Group{{C, D}},
                           distribution::USE_SIMPLE_CORRELATION_TAG),
      alfa));

  // joint distribution of A B C
  CHECK(almost_equal_distributions(
      normalize_distribution({expf(alfa) * expf(beta), expf(alfa), 1.f,
                              expf(beta), expf(beta), 1.f, expf(alfa),
                              expf(alfa) * expf(beta)}),
      graph.getJointMarginalDistribution({"A", "B", "C"})->getProbabilities(),
      0.01f));

  // joint distribution of A B
  CHECK(almost_equal_distributions(
      normalize_distribution({expf(alfa), 1.f, 1.f, expf(alfa)}),
      graph.getJointMarginalDistribution({"A", "B"})->getProbabilities(),
      0.01f));
}
