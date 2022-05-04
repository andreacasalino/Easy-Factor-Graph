#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "ModelLibrary.h"
#include "Utils.h"
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG;
using namespace EFG::model;
using namespace EFG::strct;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::test;
using namespace EFG::test::library;

namespace {
template <typename ModelT> class TestModels : public ModelT {
public:
  TestModels() = default;

  // check all messages were computed after propagation
  bool areAllMessagesComputed() const {
    for (const auto &cluster : this->getState().clusters) {
      for (const auto *node : cluster.nodes) {
        for (const auto &[connected_node, connection] :
             node->active_connections) {
          if (connection->message == nullptr) {
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
    const auto var = this->findVariable(var_name);
    return almost_equal(*ProbDistribution{expected},
                        this->getMarginalDistribution(var_name), threshold);
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
  for (const auto &a_element : a) {
    if (std::find_if(b.begin(), b.end(),
                     [&a_element](const ClusterInfo &b_element) {
                       return are_equal(a_element, b_element);
                     }) == b.end()) {
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

TEST_CASE("trivial graph propagation", "[propagation]") {
  TestModels<Graph> model;

  const float w = 1.5f;
  const float exp_w = expf(w);

  model.addConstFactor(
      make_corr_expfactor2(make_variable(2, "A"), make_variable(2, "B"), w));

  // B = 0
  model.setEvidence(model.findVariable("B"), 0);
  CHECK(model.checkMarginals("A", {exp_w, 1.f}));
  // B = 1
  model.setEvidence(model.findVariable("B"), 1);
  CHECK(model.checkMarginals("A", {1.f, exp_w}));

  model.removeAllEvidences();
  // A = 0
  model.setEvidence(model.findVariable("A"), 0);
  CHECK(model.checkMarginals("B", {exp_w, 1.f}));
  // A = 1
  model.setEvidence(model.findVariable("A"), 1);
  CHECK(model.checkMarginals("B", {1.f, exp_w}));
}

TEST_CASE("simple poly tree belief propagation", "[propagation]") {
  TestModels<SimpleTree> model;

  REQUIRE_FALSE(model.hasPropagationResult());

  const float a = expf(SimpleTree::alfa);
  const float b = expf(SimpleTree::beta);
  const float g = expf(SimpleTree::gamma);
  const float e = expf(SimpleTree::eps);

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
  REQUIRE(model.areAllMessagesComputed());
  CHECK(model.checkMarginals("B", {(g + e), (1 + g * e)}));
  CHECK(model.checkMarginals(
      "C", {(b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e))}));
  CHECK(model.checkMarginals("D", {1.f, e}));

  model.removeAllEvidences();
  REQUIRE_FALSE(model.hasPropagationResult());

  // D=1
  model.setEvidence(model.findVariable("D"), 1);
  CHECK(model.checkMarginals("A", {a + g, 1.f + a * g}));
  {
    const auto &propagation_result = model.getLastPropagationResult();
    strct::PropagationResult propagation_expected;
    propagation_expected.propagation_kind_done = PropagationKind::SUM;
    propagation_expected.was_completed = true;
    propagation_expected.structure_found =
        std::vector<ClusterInfo>{ClusterInfo{true, 3}, ClusterInfo{true, 1}};
    REQUIRE(are_equal(propagation_expected, propagation_result));
  }
  REQUIRE(model.areAllMessagesComputed());
  CHECK(model.checkMarginals("B", {1.f, g}));
  CHECK(model.checkMarginals("C", {b + g, 1.f + b * g}));
  CHECK(model.checkMarginals("E", {1.f, e}));
}

TEST_CASE("complex poly tree belief propagation", "[propagation]") {
  TestModels<ComplexTree> model;
  model.setEvidence(model.findVariable("v1"), 1);
  model.setEvidence(model.findVariable("v2"), 1);
  model.setEvidence(model.findVariable("v3"), 1);

  auto threads = GENERATE(1, 2, 4);

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

  CHECK(model.areAllMessagesComputed());
}

TEST_CASE("simple loopy graph belief propagation", "[propagation]") {
  TestModels<SimpleLoopy> model;

  float M = expf(SimpleLoopy::w);
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
  REQUIRE(model.areAllMessagesComputed());
  CHECK(model.checkMarginals("C", {M_alfa, M_beta}, 0.045f));
  CHECK(model.checkMarginals("B", {M_alfa, M_beta}, 0.045f));
  CHECK(model.checkMarginals("A", {M * M_alfa + M_beta, M_alfa + M * M_beta},
                             0.045f));
}

TEST_CASE("complex loopy graph belief propagation", "[propagation]") {
  TestModels<ComplexLoopy> model;

  model.setEvidence(model.findVariable("v1"), 1);

  auto threads = GENERATE(1, 2, 4);

  auto prob = model.getMarginalDistribution("v8", threads);
  CHECK(prob[0] < prob[1]);
  CHECK(model.areAllMessagesComputed());
}

#include <sstream>

TEST_CASE("big loopy graph", "[propagation]") {
  std::vector<std::vector<VariablePtr>> vars;

  auto make_name = [](const std::size_t r, const std::size_t c) {
    std::stringstream stream;
    stream << "V_" << std::to_string(r) << std::to_string(c);
    return stream.str();
  };

  const std::size_t size = 10;
  vars.reserve(size);
  for (std::size_t r = 0; r < size; ++r) {
    auto &row = vars.emplace_back();
    row.reserve(size);
    for (std::size_t c = 0; c < size; ++c) {
      row.push_back(make_variable(2, make_name(r, c)));
    }
  }

  Graph model;

  using Coord = std::pair<std::size_t, std::size_t>;
  auto add_factor = [&](const Coord &first, const Coord &second) {
    model.addConstFactor(make_corr_expfactor2(vars[first.first][first.second],
                                              vars[second.first][second.second],
                                              0.1f));
  };

  for (std::size_t r = 0; r < size; ++r) {
    for (std::size_t c = 0; c < size; ++c) {
      if (0 < r) {
        add_factor(Coord{r, c}, Coord{r - 1, c});
      }
      if (0 < c) {
        add_factor(Coord{r, c}, Coord{r, c - 1});
      }
      if ((0 < r) && (0 < c)) {
        add_factor(Coord{r, c}, Coord{r - 1, c - 1});
      }
    }
  }

  auto threads = GENERATE(1, 2, 4);
  model.getMarginalDistribution(make_name(0, 0), threads);
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
    model.addConstFactor(make_corr_expfactor2(a, b, w));
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

    auto get_expected_MAP_val = [](const std::string &var_name) {
      if (var_name == "Y0") {
        return 0;
      }
      if (var_name == "Y1") {
        return 1;
      }
      if (var_name == "Y2") {
        return 0;
      }
      return 1;
    };
    std::vector<std::size_t> mapExpected;
    mapExpected.reserve(4);
    for (const auto &var : model.getHiddenVariables()) {
      mapExpected.push_back(get_expected_MAP_val(var->name()));
    }
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
  graph.addConstFactor(make_corr_expfactor2(A, B, alfa));
  graph.addConstFactor(make_corr_expfactor2(B, C, alfa));
  graph.addConstFactor(make_corr_expfactor2(C, D, alfa));

  // joint distribution of A B C
  CHECK(almost_equal(
      *ProbDistribution{{expf(alfa) * expf(beta), expf(alfa), 1.f, expf(beta),
                         expf(beta), 1.f, expf(alfa), expf(alfa) * expf(beta)}},
      graph.getJointMarginalDistribution({"A", "B", "C"}).getProbabilities(),
      0.15f));

  // joint distribution of A B
  CHECK(almost_equal(
      *ProbDistribution{{expf(alfa), 1.f, 1.f, expf(alfa)}},
      graph.getJointMarginalDistribution({"A", "B"}).getProbabilities(),
      0.01f));
}

TEST_CASE("Belief propagation with Pool efficiency", "[propagation]") {
  auto depth = GENERATE(8, 10);
  auto loopy = GENERATE(false, true);

  ScalableModel model(depth, 3, loopy);

  auto measure_time =
      [&](const std::size_t threads) -> std::chrono::nanoseconds {
    model.removeAllEvidences();
    model.setEvidence(model.root(), 0);
    return test::measure_time(
        [&]() { model.getMarginalDistribution(model.nonRoot(), threads); });
  };

  auto single_thread_time = measure_time(1);
  auto multi_thread_time = measure_time(2);

  CHECK(static_cast<double>(multi_thread_time.count()) <
        static_cast<double>(single_thread_time.count()));
}
