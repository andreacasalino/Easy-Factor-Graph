#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>

#include <BruteForceQuery.h>
#include <ModelLibrary.h>
#include <Utils.h>

namespace EFG::test {
using namespace categoric;
using namespace structure;
using namespace model;

namespace {
bool check_gradient_at(model::RandomField &model, const BruteForceGradient &bf,
                       const std::vector<float> &w) {
  model.setTunableWeights(w);

  std::vector<float> gradient;
  model.gradient(bf.getSamples()).get(gradient);

  auto gradient_expected = bf.getGradientAt();

  return almost_equal_it(gradient, gradient_expected, 0.01f);
}
} // namespace

TEST_CASE("Gradient evaluation on a single factor model", "[gradient][rf]") {
  LineModel model{2, 1.f};
  BruteForceGradient bf{model};

  auto w_shifted = GENERATE(1.f, 0.5f, 2.f);

  CHECK(check_gradient_at(model, bf, {w_shifted}));
}

TEST_CASE("Gradient evaluation on a dual factors model", "[gradient][rf]") {
  const float alfa = 1.f;
  const float beta = 0.5f;

  LineModel model{2, {alfa, beta}};
  BruteForceGradient bf{model};

  auto w_shifted =
      GENERATE(std::vector<float>{1.f, 0.5f}, std::vector<float>{1.f, 1.f},
               std::vector<float>{0.5f, 0.5f}, std::vector<float>{2.f, 2.f});

  CHECK(check_gradient_at(model, bf, w_shifted));
}

TEST_CASE("Gradient evaluation on a trial factors model", "[gradient][rf]") {
  const float alfa = 1.f;
  const float beta = 0.5f;
  const float gamma = 2.f;

  LineModel model{2, {alfa, beta, gamma}};
  BruteForceGradient bf{model};

  auto w_shifted = GENERATE(
      std::vector<float>{1.f, 0.5f, 2.f}, std::vector<float>{1.f, 1.f, 1.f},
      std::vector<float>{0.5f, 0.5f, 0.5f}, std::vector<float>{2.f, 2.f, 2.f});

  CHECK(check_gradient_at(model, bf, w_shifted));
}

namespace {
std::vector<float> make_shifted(const std::vector<float> &src, float delta) {
  auto res = src;
  for (auto &v : res) {
    v += delta;
  }
  return res;
}
} // namespace

TEST_CASE("Gradient evaluation on a simple tree", "[gradient][rf]") {
  SimpleTree model;
  BruteForceGradient bf{model};

  std::vector<float> w_origin{SimpleTree::alfa, SimpleTree::beta,
                              SimpleTree::gamma, SimpleTree::eps};

  auto w_shifted = make_shifted(w_origin, 0.8f);

  CHECK(check_gradient_at(model, bf, w_shifted));
}

TEST_CASE("Gradient evaluation on a simple loopy model", "[gradient][rf]") {
  SimpleLoopy model;
  BruteForceGradient bf{model};

  std::vector<float> w_origin;
  w_origin.resize(5, SimpleLoopy::w);

  auto w_shifted = make_shifted(w_origin, 0.8f);

  CHECK(check_gradient_at(model, bf, w_shifted));
}

/* TODO
TEST_CASE("Gradient evaluation on a simple conditional random field",
          "[gradient][crf]") {
  structure::ModelBuilder builder;

  auto make_level_ = [&](std::size_t len) {
    std::vector<std::size_t> res;
    for (std::size_t i = 0; i < len; ++i) {
      res.push_back(builder.make_variable(3));
    }
    return res;
  };

  std::vector<std::size_t> o = make_level_(4);
  std::vector<std::size_t> h0 = make_level_(3);
  std::vector<std::size_t> h1 = make_level_(2);

  std::vector<float> w_origin;

  for (std::size_t i = 0; i < h0.size(); ++i) {
    add_corr_expfactor(builder, w_origin.emplace_back(1.f), o[i], h0[i]);
    add_corr_expfactor(builder, w_origin.emplace_back(0.5f), o[i + 1], h0[i]);
  }
  for (std::size_t i = 0; i < h1.size(); ++i) {
    add_corr_expfactor(builder, w_origin.emplace_back(1.f), h0[i], h1[i]);
    add_corr_expfactor(builder, w_origin.emplace_back(0.5f), h0[i + 1], h1[i]);
  }

  ConditionalRandomField model{
      structure::ModelBuilder::build(std::move(builder))};

  auto w_shifted = make_shifted(w_origin, 0.8f);

  std::vector<float> gradient = get_gradient(model, 1000, w_shifted);

  CHECK(check_gradient(w_origin, w_shifted, gradient));
}
*/
} // namespace EFG::test
