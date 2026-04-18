#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/Graph.h>

#include <BruteForceQuery.h>
#include <ModelLibrary.h>
#include <Utils.h>

namespace EFG::test {
using namespace model;
using namespace structure;
using namespace categoric;
using namespace factor;

namespace {
template <typename M>
bool checkMarginals(M &model, std::size_t var_index,
                    std::vector<float> expected, float threshold = 0.01f) {
  static thread_local std::vector<float> res;
  res.clear();
  model.getMarginalDistribution(res, var_index);

  auto expected_normalized = make_prob_distr(std::move(expected));

  return almost_equal_it(res, expected_normalized, threshold);
}
} // namespace

TEST_CASE("trivial graph propagation", "[propagation][trivial]") {
  structure::ModelBuilder res;

  std::size_t var_a = res.make_variable(2);
  std::size_t var_b = res.make_variable(2);

  auto w = GENERATE(1.f, 1.5f, 2.f);

  add_corr_expfactor(res, w, var_a, var_b);

  const float exp_w = expf(w);
  model::Graph model{structure::ModelBuilder::build(std::move(res))};

  // B = 0
  model.setEvidences(Evidence{var_b, 0});
  CHECK(checkMarginals(model, var_a, {exp_w, 1.f}));
  // B = 1
  model.setEvidences(Evidence{var_b, 1});
  CHECK(checkMarginals(model, var_a, {1.f, exp_w}));

  model.removeAllEvidences();
  // A = 0
  model.setEvidences(Evidence{var_a, 0});
  CHECK(checkMarginals(model, var_b, {exp_w, 1.f}));
  // A = 1
  model.setEvidences(Evidence{var_a, 1});
  CHECK(checkMarginals(model, var_b, {1.f, exp_w}));
}

TEST_CASE("simple poly tree belief propagation", "[propagation][tree]") {
  SimpleTree model;

  const float a = expf(SimpleTree::alfa);
  const float b = expf(SimpleTree::beta);
  const float g = expf(SimpleTree::gamma);
  const float e = expf(SimpleTree::eps);

  // E=1
  model.setEvidences(Evidence{model.getVar('E'), 1});
  CHECK(checkMarginals(
      model, model.getVar('A'),
      {(a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e))}));
  CHECK(checkMarginals(model, model.getVar('B'), {(g + e), (1 + g * e)}));
  CHECK(checkMarginals(
      model, model.getVar('C'),
      {(b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e))}));
  CHECK(checkMarginals(model, model.getVar('D'), {1.f, e}));

  // D=1
  model.removeAllEvidences();
  model.setEvidences(Evidence{model.getVar('D'), 1});
  CHECK(checkMarginals(model, model.getVar('A'), {a + g, 1.f + a * g}));
  CHECK(checkMarginals(model, model.getVar('B'), {1.f, g}));
  CHECK(checkMarginals(model, model.getVar('C'), {b + g, 1.f + b * g}));
  CHECK(checkMarginals(model, model.getVar('E'), {1.f, e}));
}

namespace {
void add_asymetric_factor(structure::ModelBuilder &model, std::size_t var_a,
                          std::size_t var_b, float w) {
  categoric::VarStateSize var_a_size = model.get_seed().variables_sizes[var_a];
  categoric::VarStateSize var_b_size = model.get_seed().variables_sizes[var_b];
  if (var_b_size < var_a_size) {
    add_asymetric_factor(model, var_b, var_a, w);
    return;
  }

  categoric::Group<2> g{{var_a_size, var_b_size}};
  std::vector<misc::Intervals::IntervalPoint> intervals_values;
  for (categoric::VarStateSize b = 0; b < var_b_size; b += 1) {
    categoric::VarStateSize a =
        b % static_cast<categoric::VarStateSize>(var_a_size);
    auto comb_idx = g.combinationIndex({a, b});
    intervals_values.emplace_back(std::make_pair(comb_idx, 1.f));
  }

  auto intervals =
      misc::Intervals::from_points<true>(std::move(intervals_values));
  factor::FactorExponential<2> factor{{var_a_size, var_b_size},
                                      std::move(intervals)};
  factor.trsfm.setWeight(w);

  model.add_tunable_binary_factor(std::move(factor), var_a, var_b);
}
} // namespace

TEST_CASE("simple poly tree with different sizes vars", "[propagation][tree]") {
  structure::ModelBuilder builder;
  auto A = builder.make_variable(3);
  auto B = builder.make_variable(2);
  auto C = builder.make_variable(2);
  auto D = builder.make_variable(4);
  add_asymetric_factor(builder, A, B, 1.2f);
  add_asymetric_factor(builder, A, C, 1.f);
  add_asymetric_factor(builder, A, D, 1.3f);

  model::RandomField model{structure::ModelBuilder::build(std::move(builder))};

  model.setEvidences(Evidence{B, 1});

  BruteForce bf{model.getStructure()};
  std::vector<float> prob, prob_expected;

  model.getMarginalDistribution(prob, C);
  prob_expected = bf.getMarginals(C);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));

  model.getMarginalDistribution(prob, D);
  prob_expected = bf.getMarginals(D);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));
}

TEST_CASE("complex poly tree belief propagation", "[propagation][tree]") {
  ComplexTree model;

  model.setEvidences(Evidence{0, 1}, Evidence{1, 1}, Evidence{2, 1});

  auto threads = GENERATE(1, 4);
  auto activator = activate_if_needed(model, threads);

  BruteForce bf{model.getStructure()};
  std::vector<float> prob, prob_expected;

  model.getMarginalDistribution(prob, 9);
  prob_expected = bf.getMarginals(9);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));

  model.getMarginalDistribution(prob, 10);
  prob_expected = bf.getMarginals(10);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));

  model.getMarginalDistribution(prob, 12);
  prob_expected = bf.getMarginals(12);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));
}

TEST_CASE("simple loopy graph belief propagation", "[propagation][loopy]") {
  SimpleLoopy model;

  float M = expf(SimpleLoopy::w);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);

  // E=1
  model.setEvidences(Evidence{model.getVar('E'), 1});
  CHECK(checkMarginals(model, model.getVar('D'),
                       {3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2)},
                       0.04f));
  CHECK(checkMarginals(model, model.getVar('C'), {M_alfa, M_beta}, 0.03f));
  CHECK(checkMarginals(model, model.getVar('B'), {M_alfa, M_beta}, 0.03f));
  CHECK(checkMarginals(model, model.getVar('A'),
                       {M * M_alfa + M_beta, M_alfa + M * M_beta}, 0.03f));
}

TEST_CASE("simple loopy graph belief propagation with different sizes vars",
          "[propagation][loopy]") {
  structure::ModelBuilder builder;
  auto A = builder.make_variable(3);
  auto B = builder.make_variable(2);
  auto C = builder.make_variable(2);
  auto D = builder.make_variable(4);
  add_asymetric_factor(builder, A, B, 1.2f);
  add_asymetric_factor(builder, A, C, 1.f);
  add_asymetric_factor(builder, A, D, 1.3f);
  add_asymetric_factor(builder, C, D, 1.f);

  model::RandomField model{structure::ModelBuilder::build(std::move(builder))};

  model.setEvidences(Evidence{B, 1});

  BruteForce bf{model.getStructure()};
  std::vector<float> prob, prob_expected;

  model.getMarginalDistribution(prob, C);
  prob_expected = bf.getMarginals(C);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));

  model.getMarginalDistribution(prob, D);
  prob_expected = bf.getMarginals(D);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));
}

TEST_CASE("complex loopy graph belief propagation", "[propagation][loopy]") {
  ComplexLoopy model;

  model.setEvidences(Evidence{0, 1});

  auto threads = GENERATE(1, 4);
  auto activator = activate_if_needed(model, threads);

  std::vector<float> prob;
  model.getMarginalDistribution(prob, 7);

  auto prob_expected = BruteForce{model.getStructure()}.getMarginals(7);
  CHECK(almost_equal_it(prob, prob_expected, 0.03f));
}

TEST_CASE("big loopy graph", "[propagation][loopy]") {
  MatrixLoopy model{10};

  model.setEvidences(Evidence{0, 1});

  auto threads = GENERATE(1, 4);
  auto activator = activate_if_needed(model, threads);

  auto last_var_index = model.getStructure().nodes.size() - 1;

  std::vector<float> prob;
  model.getMarginalDistribution(prob, last_var_index);

  // comparing with brute force would take too much time ...
  CHECK(prob[0] < prob[1]);
}

TEST_CASE("MAPTest, strong weight between hidden", "[propagation][MAP]") {
  ChainModel model{4, 0.1f, 1.f};

  std::vector<categoric::VarStateSize> map, mapExpected = {1, 1, 1, 1};
  model.getHiddenSetMAP(map);
  CHECK(map == mapExpected);
}

TEST_CASE("MAPTest, strong weight for evidences", "[propagation][MAP]") {
  ChainModel model{4, 1.0f, 0.1f};

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

  CHECK(model.getMAP(0) == 0);
  CHECK(model.getMAP(1) == 1);
  CHECK(model.getMAP(2) == 0);
  CHECK(model.getMAP(3) == 1);
}

TEST_CASE("Sub graph distribution", "[propagation][subgraph]") {
  structure::ModelBuilder res;

  std::size_t var_a = res.make_variable(2);
  std::size_t var_b = res.make_variable(2);
  std::size_t var_c = res.make_variable(2);
  std::size_t var_d = res.make_variable(2);

  float alfa = 0.5f, beta = 1.5f;

  add_corr_expfactor(res, alfa, var_a, var_b);
  add_corr_expfactor(res, alfa, var_b, var_c);
  add_corr_expfactor(res, alfa, var_c, var_d);

  model::Graph model{structure::ModelBuilder::build(std::move(res))};

  // joint distribution of A B C
  {
    auto joint_factor = model.getJointMarginalDistribution<3>({0, 1, 2});
    std::vector<float> prob;
    factor::getProbabilities(joint_factor, prob);
    CHECK(almost_equal_it(
        make_prob_distr({expf(alfa) * expf(beta), expf(alfa), 1.f, expf(beta),
                         expf(beta), 1.f, expf(alfa), expf(alfa) * expf(beta)}),
        prob, 0.15f));
  }

  // joint distribution of A B
  {
    auto joint_factor = model.getJointMarginalDistribution<2>({0, 1});
    std::vector<float> prob;
    factor::getProbabilities(joint_factor, prob);
    CHECK(almost_equal_it(make_prob_distr({expf(alfa), 1.f, 1.f, expf(alfa)}),
                          prob, 0.15f));
  }
}
} // namespace EFG::test
