#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/QueryManager.h>

#include <BruteForceQuery.h>
#include <Utils.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <math.h>
#include <ranges>
#include <span>

#include "ModelLibrary.h"

namespace EFG::test {
#define CONTEX_FIELD(type, name, default_value)                                \
  type name{default_value};                                                    \
  TrainTestContext &set_##name(type val) {                                     \
    name = val;                                                                \
    return *this;                                                              \
  }

struct TrainTestContext {
  CONTEX_FIELD(std::size_t, samples, 500)
  CONTEX_FIELD(std::size_t, max_iterations, 50)
  CONTEX_FIELD(std::size_t, threads, 1)

  CONTEX_FIELD(float, check_marginal_toll, 0.15f)
};

template <typename ModelT> struct LearningTest {
  LearningTest(ModelT &model, TrainTestContext ctxt = TrainTestContext{})
      : model_{model}, ctxt_{std::move(ctxt)}, bf_{model_, ctxt_.samples} {
    initial_marginals_ = compute_marginals();
    std::vector<float> ones;
    ones.resize(model.getStructure().tunability.order.size(), 1.f);
    model_.setTunableWeights(ones);
  }

  bool train() {
    auto guard = activate_if_needed(model_, ctxt_.threads);
    auto samples = bf_.getSamples();
    structure::Trainer{}
        .max_iterations(ctxt_.max_iterations)
        .train_model(model_, samples);

    /////////////////// check marginals ///////////////////
    auto marginals_now = compute_marginals();
    for (std::size_t k = 0; k < marginals_now.size(); ++k) {
      if (!almost_equal_it(marginals_now[k], initial_marginals_[k],
                           ctxt_.check_marginal_toll)) {
        return false;
      }
    }
    return true;
  }

private:
  ModelT &model_;
  TrainTestContext ctxt_;
  BruteForceGradient bf_;

  std::vector<std::vector<float>> compute_marginals() {
    const std::vector<structure::Node> &nodes = model_.getStructure().nodes;
    if constexpr (std::is_same_v<ModelT, model::RandomField>) {
      model::RandomField &model = model_;
      // set last var as evidence with value = 0
      model.setEvidences(structure::Evidence{nodes.size() - 1, 0});
    } else {
      // set evidences to zeros
      std::vector<std::size_t> ev;
      for (std::size_t k = 0; k < nodes.size(); ++k) {
        if (nodes[k].evidence != structure::Evidence::NOT_AN_EVIDENCE) {
          ev.push_back(0);
        }
      }
      model_.setEvidences(ev);
    }

    std::vector<std::vector<float>> res;
    for (std::size_t k = 0; k < nodes.size(); ++k) {
      if (nodes[k].evidence != structure::Evidence::NOT_AN_EVIDENCE) {
        continue;
      }
      model_.getMarginalDistribution(res.emplace_back(), k);
    }
    return res;
  }
  std::vector<std::vector<float>> initial_marginals_;
};

TEST_CASE("Small random field tuning", "[train]") {
  structure::ModelBuilder builder;
  auto varA = builder.make_variable(3);
  auto varB = builder.make_variable(3);
  auto varC = builder.make_variable(3);
  add_indicator_expfactor(builder, 1.f, varA);
  add_corr_expfactor(builder, 2.f, varA, varB);
  add_corr_expfactor(builder, 0.5f, varA, varC);

  model::RandomField model{structure::ModelBuilder::build(std::move(builder))};

  bool ok = LearningTest<model::RandomField>{model}.train();
  CHECK(ok);
}

TEST_CASE("Medium random field tuning", "[train]") {
  structure::ModelBuilder builder;
  auto varA = builder.make_variable(3);
  auto varB = builder.make_variable(3);
  auto varC = builder.make_variable(3);
  auto varD = builder.make_variable(3);
  auto varE = builder.make_variable(3);
  add_indicator_expfactor(builder, 1.f, varA);
  add_corr_expfactor(builder, 2.f, varA, varB);
  add_corr_expfactor(builder, 0.5f, varA, varC);
  add_corr_expfactor(builder, 2.f, varA, varD);
  add_corr_expfactor(builder, 0.5f, varA, varE);

  model::RandomField model{structure::ModelBuilder::build(std::move(builder))};

  auto threads = GENERATE(1, 2, 4);
  bool ok =
      LearningTest<model::RandomField>{
          model, TrainTestContext{}.set_threads(threads).set_samples(1000)}
          .train();
  CHECK(ok);
}

/*
TEST_CASE("Small conditional random field tuning", "[train]") {
  LearningTest<ConditionalRandomField> info([]() {
    VariablePtr A = make_variable(3, "A");
    VariablePtr B = make_variable(3, "B");
    VariablePtr C = make_variable(3, "C");

    RandomField reference_model_temp;
    reference_model_temp.copyConstFactor(
        factor::FactorExponential{factor::Indicator{A, 0}, 1.f});
    reference_model_temp.addTunableFactor(make_corr_expfactor_ptr(A, B, 2.f));
    reference_model_temp.addTunableFactor(make_corr_expfactor_ptr(A, C,
0.5f)); reference_model_temp.setEvidence(B, 0);
    reference_model_temp.setEvidence(C, 0);
    return std::make_unique<ConditionalRandomField>(reference_model_temp,
                                                    false);
  });
  info.samplesMaker([](ConditionalRandomField &reference) {
        return make_good_trainset(reference, 1000);
      })
      .checkMarginalsToll(std::numeric_limits<float>::max());

  SECTION("Gradient Descend Fixed") {
    info.trainerInitialization([](::train::IterativeTrainer &trainer) {
      static_cast<::train::GradientDescendFixed
&>(trainer).setOptimizationStep( 0.2f);
    });
    CHECK(info.train<::train::GradientDescendFixed>());
  }
  info.checkLikelihoodToll(std::numeric_limits<float>::max());
  SECTION("Gradient Descend Adaptive") {
    CHECK(info.train<::train::GradientDescend<::train::YundaSearcher>>());
  }
  SECTION("Gradient Descend Conjugate") {
    CHECK(info.train<
          ::train::GradientDescendConjugate<::train::YundaSearcher>>());
  }
  SECTION("Quasi Newton") {
    CHECK(info.train<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>());
  }
}
*/
} // namespace EFG::test
