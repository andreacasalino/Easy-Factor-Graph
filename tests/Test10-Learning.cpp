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
template <typename Trainer> class TrainerWithStory : public Trainer {
public:
  using Trainer::Trainer;

  struct StoryIterator {
    StoryIterator(const TrainerWithStory &src)
        : len_{src.len_}, rest_{src.wStory_.begin(), src.wStory_.end()} {}

    std::optional<std::span<const float>> next() {
      if (rest_.empty()) {
        return std::nullopt;
      }
      std::span<const float> res{rest_.begin(), rest_.begin() + len_};
      rest_ = {rest_.begin() + len_, rest_.end()};
      return res;
    }

  private:
    std::size_t len_;
    std::span<const float> rest_;
  };
  StoryIterator make_iter() const { return {*this}; }

protected:
  void updateDirection() override {
    Eigen::VectorXd par = this->getParameters();
    len_ = static_cast<std::size_t>(par.size());
    for (auto val : par) {
      wStory_.push_back(static_cast<float>(val));
    }
    this->Trainer::updateDirection();
  };

private:
  std::size_t len_{0};
  std::vector<float> wStory_;
};

#define CONTEX_FIELD(type, name, default_value)                                \
  type name{default_value};                                                    \
  ContextBuilder &set_##name(type val) {                                       \
    name = val;                                                                \
    return *this;                                                              \
  }

struct ContextBuilder {
  CONTEX_FIELD(std::size_t, samples, 500) // TODO use me!
  CONTEX_FIELD(std::size_t, max_iterations, 30)
  CONTEX_FIELD(std::size_t, threads, 1)

  CONTEX_FIELD(float, check_weights_toll, 0.5f)
  CONTEX_FIELD(float, check_likelihood_trend_toll, 5.f)
  CONTEX_FIELD(float, check_marginal_toll, 0.15f)
};

template <typename ModelT, typename Trainer> struct LearningTest {
  LearningTest(ModelT &model, TrainerWithStory<Trainer> &driver,
               ContextBuilder ctxt)
      : model_{model}, driver_{driver}, ctxt_{std::move(ctxt)},
        bf_{model_, ctxt_.samples} {
    model_.getTunableWeights(initial_w_);
    initial_marginals_ = compute_marginals();
    auto ones = initial_w_;
    for (auto &val : ones) {
      val = 1.f;
    }
    model_.setTunableWeights(ones);
  }

  bool train() {
    driver_.setMaxIterations(ctxt_.max_iterations);
    auto guard = activate_if_needed(model_, ctxt_.threads);
    auto samples = bf_.getSamples();
    structure::train_model(model_, driver_, samples);

    /////////////////// checks ///////////////////
    return almost_equal_it(initial_w_, getW(), ctxt_.check_weights_toll) &&
           checkLikelihoodTrend() && checkMarginals();
  }

private:
  std::vector<float> getW() const {
    std::vector<float> w;
    model_.getTunableWeights(w);
    return w;
  }

  bool checkLikelihoodTrend() {
    if (ctxt_.check_likelihood_trend_toll ==
        std::numeric_limits<float>::max()) {
      return true;
    }
    const auto final_w = getW();
    auto story_it = driver_.make_iter();
    auto prev_likelihood = -std::numeric_limits<float>::max();
    while (true) {
      auto next = story_it.next();
      if (!next) {
        break;
      }
      model_.setTunableWeights(*next);
      float att_likelihood = bf_.getLogLikelihoodAt();
      const bool ok = (-ctxt_.check_likelihood_trend_toll) <
                      (att_likelihood - prev_likelihood);
      if (!ok) {
        return false;
      }
      prev_likelihood = att_likelihood;
    }
    model_.setTunableWeights(final_w);
    return true;
  }

  bool checkMarginals() {
    auto marginals_now = compute_marginals();
    for (std::size_t k = 0; k < marginals_now.size(); ++k) {
      if (!almost_equal_it(marginals_now[k], initial_marginals_[k],
                           ctxt_.check_marginal_toll)) {
        return false;
      }
    }
    return true;
  }

  ModelT &model_;
  TrainerWithStory<Trainer> &driver_;
  ContextBuilder ctxt_;
  BruteForceGradient bf_;

  std::vector<float> initial_w_;

  std::vector<std::vector<float>> compute_marginals() {
    if (ctxt_.check_marginal_toll == std::numeric_limits<float>::max()) {
      return {};
    }

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

template <typename Trainer, typename ModelT, typename TrainerInitPred>
bool train_test_with_init(ModelT &model, ContextBuilder ctxt,
                          TrainerInitPred &&pred) {
  TrainerWithStory<Trainer> driver;
  pred(driver);
  LearningTest<ModelT, Trainer> test{model, driver, ctxt};
  return test.train();
}

template <typename Trainer, typename ModelT>
bool train_test(ModelT &model, ContextBuilder ctxt) {
  return train_test_with_init<Trainer, ModelT>(model, ctxt, [](auto &_) {});
}

TEST_CASE("Small random field tuning", "[train]") {
  structure::ModelBuilder builder;
  auto varA = builder.make_variable(3);
  auto varB = builder.make_variable(3);
  auto varC = builder.make_variable(3);
  add_indicator_expfactor(builder, 1.f, varA);
  add_corr_expfactor(builder, 2.f, varA, varB);
  add_corr_expfactor(builder, 0.5f, varA, varC);

  model::RandomField model{structure::ModelBuilder::build(std::move(builder))};
  ContextBuilder ctxt;

  SECTION("Gradient Descend Fixed") {
    bool ok = train_test_with_init<::train::GradientDescendFixed>(
        model, ctxt, [](auto &trainer) { trainer.setOptimizationStep(0.5f); });
    CHECK(ok);
  }

  ctxt.set_check_likelihood_trend_toll(std::numeric_limits<float>::max());

  SECTION("Gradient Descend Adaptive") {
    bool ok = train_test<::train::GradientDescend<::train::YundaSearcher>>(
        model, ctxt);
    CHECK(ok);
  }
  SECTION("Gradient Descend Conjugate") {
    bool ok =
        train_test<::train::GradientDescendConjugate<::train::YundaSearcher>>(
            model, ctxt);
    CHECK(ok);
  }
  SECTION("Quasi Newton") {
    bool ok =
        train_test<::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
            model, ctxt);
    CHECK(ok);
  }
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
  ContextBuilder ctxt = ContextBuilder{}.set_samples(1000);

  SECTION("Gradient Descend Fixed") {
    auto threads = GENERATE(1, 2, 4);
    ctxt.set_threads(threads);
    bool ok = train_test_with_init<::train::GradientDescendFixed>(
        model, ctxt, [](auto &trainer) { trainer.setOptimizationStep(0.2f); });
    CHECK(ok);
  }

  ctxt.set_check_likelihood_trend_toll(std::numeric_limits<float>::max());

  SECTION("Gradient Descend Adaptive") {
    bool ok = train_test<::train::GradientDescend<::train::YundaSearcher>>(
        model, ctxt);
    CHECK(ok);
  }
  SECTION("Gradient Descend Conjugate") {
    bool ok =
        train_test<::train::GradientDescendConjugate<::train::YundaSearcher>>(
            model, ctxt);
    CHECK(ok);
  }
  SECTION("Quasi Newton") {
    bool ok =
        train_test<::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
            model, ctxt);
    CHECK(ok);
  }
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
