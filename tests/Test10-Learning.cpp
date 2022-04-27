#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/QueryManager.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <EasyFactorGraph/trainable/ModelTrainer.h>

#include <TrainingTools/iterative/solvers/GradientDescend.h>
#include <TrainingTools/iterative/solvers/GradientDescendConjugate.h>
#include <TrainingTools/iterative/solvers/QuasiNewton.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <math.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;
using namespace EFG::train;
using namespace EFG::strct;
using namespace EFG::test;

namespace {
template <typename TrainerT> class StoryAware : public TrainerT {
public:
  std::list<std::vector<float>> getStory() const { return descendStory; }

protected:
  void updateDirection() override {
    Eigen::VectorXd par = this->getParameters();
    std::vector<float> w;
    w.reserve(par.size());
    for (Eigen::Index i = 0; i < par.size(); ++i) {
      w.push_back(par(i));
    }
    this->descendStory.emplace_back(std::move(w));
    this->TrainerT::updateDirection();
  };

  void initDirection() override {
    this->descendStory.clear();
    this->TrainerT::initDirection();
  }

private:
  std::list<std::vector<float>> descendStory;
};

class LikelihoodAware {
public:
  LikelihoodAware(const EFG::strct::ConnectionsManager &model)
      : vars{model.getAllVariables()} {
    const auto &factors = model.getAllFactors();
    finders.reserve(factors.size());
    const auto &all_vars = vars.getVariables();
    for (const auto &factor : factors) {
      finders.emplace_back(factor->makeFinder(all_vars));
    }
  };

  float getLogActivation(const EFG::categoric::Combination &c) const {
    float res = 0.f;
    for (auto it = this->finders.begin(); it != this->finders.end(); ++it) {
      res += logf(it->find(c).value);
    }
    return res;
  };

  float getLogLikeliHood(const EFG::train::TrainSet::Iterator &combinations) {
    float Z = 0.f;
    {
      GroupRange range(Group{vars});
      for_each_combination(range, [this, &Z](const Combination &comb) {
        Z += this->getLogActivation(comb);
      });
    }
    float lkl = 0.f, coeff = 1.f / static_cast<float>(combinations.size());
    combinations.forEachSample([this, &lkl, &coeff](const Combination &comb) {
      lkl += coeff * this->getLogActivation(comb);
    });
    return lkl - Z;
  }

private:
  Group vars;
  std::vector<CombinationFinder> finders;
};

bool has_decreasing_likelihood(FactorsTunableAware &trained_model,
                               const EFG::train::TrainSet &train_set,
                               const std::list<std::vector<float>> &story,
                               const float toll) {
  const auto final_w = trained_model.getWeights();
  LikelihoodAware likelihood_aware(trained_model);
  float prev_likelihood = -std::numeric_limits<float>::max();
  const auto train_set_iter = train_set.makeIterator();
  for (const auto &w : story) {
    trained_model.setWeights(w);
    float att_likelihood = likelihood_aware.getLogLikeliHood(train_set_iter);
    const bool ok = (-toll) < (att_likelihood - prev_likelihood);
    if (!ok) {
      return false;
    }
    prev_likelihood = att_likelihood;
  }
  trained_model.setWeights(final_w);
  return true;
}

template <typename T> T *cast_model(FactorsTunableAware &subject) {
  return dynamic_cast<T *>(&subject);
}

struct Models {
  FactorsTunableAware &reference_model;
  FactorsTunableAware &trained_model;
};

VariablesSoup to_soup(const VariablesSet &subject) {
  return VariablesSoup{subject.begin(), subject.end()};
}

bool almost_equal_marginals(const Models &models, const float toll) {
  auto hidden_vars = to_soup(models.trained_model.getHiddenVariables());
  auto var_getter = [&model = models.reference_model](const std::string &name) {
    return model.findVariable(name);
  };

  cast_model<EvidenceSetter>(models.trained_model)
      ->setEvidence(hidden_vars.back(), 0);
  cast_model<EvidenceSetter>(models.reference_model)
      ->setEvidence(var_getter(hidden_vars.back()->name()), 0);

  for (std::size_t k = 0; k < (hidden_vars.size() - 1); ++k) {
    auto marginals_trained =
        cast_model<QueryManager>(models.trained_model)
            ->getMarginalDistribution(hidden_vars[k]->name());

    auto marginals_reference =
        cast_model<QueryManager>(models.reference_model)
            ->getMarginalDistribution(hidden_vars[k]->name());

    if (!almost_equal(marginals_trained, marginals_reference, toll)) {
      return false;
    }
  }
  return true;
}

struct CheckContext {
  CheckContext(const Models &models, const TrainSet &train_set)
      : models(models), train_set(train_set) {}

  Models models;
  TrainSet train_set;
  std::size_t max_iterations = 30;
  std::optional<float> stoch_percentage = 1.f;
  std::optional<std::function<void(::train::IterativeTrainer &)>>
      trainer_setting;

  std::optional<float> check_weights_toll = 0.5f;
  std::optional<float> check_likelihood_trend_toll = 5.f;
  std::optional<float> check_marginal_info = 0.15f;
};
template <typename TrainerT>
bool check_trainer(const CheckContext &ctxt, const std::size_t threads) {
  set_ones(ctxt.models.trained_model);
  StoryAware<TrainerT> trainer;
  trainer.setMaxIterations(ctxt.max_iterations);
  if (std::nullopt != ctxt.trainer_setting) {
    (*ctxt.trainer_setting)(trainer);
  }
  TrainInfo info;
  info.stochastic_percentage =
      ctxt.stoch_percentage.has_value() ? *ctxt.stoch_percentage : 1.f;
  info.threads = threads;
  train_model(ctxt.models.trained_model, trainer, ctxt.train_set, info);
  // check tuned values
  if (ctxt.check_weights_toll &&
      (!almost_equal(ctxt.models.reference_model.getWeights(),
                     ctxt.models.trained_model.getWeights(),
                     *ctxt.check_weights_toll))) {
    return false;
  }
  // check decresing trend
  if (ctxt.check_likelihood_trend_toll &&
      (!has_decreasing_likelihood(ctxt.models.trained_model, ctxt.train_set,
                                  trainer.getStory(),
                                  *ctxt.check_likelihood_trend_toll))) {
    return false;
  }
  // check marginal computation
  if (ctxt.check_marginal_info &&
      (!almost_equal_marginals(ctxt.models, *ctxt.check_marginal_info))) {
    return false;
  }
  return true;
}
} // namespace

TEST_CASE("Small random field tuning", "[train]") {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(3, "B");
  VariablePtr C = make_variable(3, "C");

  RandomField reference_model;
  reference_model.copyConstFactor(FactorExponential{Indicator{A, 0}, 1.f});
  reference_model.addTunableFactor(make_corr_expfactor2(A, B, 2.f));
  reference_model.addTunableFactor(make_corr_expfactor2(A, C, 0.5f));

  RandomField to_tune(reference_model);

  CheckContext context(
      Models{reference_model, to_tune},
      TrainSet(reference_model.getHiddenSetSamples(
          GibbsSampler::SamplesGenerationContext{500, 20, 0})));

  SECTION("Gradient Descend Fixed") {
    context.trainer_setting = [](::train::IterativeTrainer &trainer) {
      static_cast<::train::GradientDescendFixed &>(trainer).setOptimizationStep(
          0.5f);
    };
    CHECK(check_trainer<::train::GradientDescendFixed>(context, 1));
  }
  SECTION("Gradient Descend Adaptive") {
    CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
        context, 1));
  }
  SECTION("Gradient Descend Conjugate") {
    CHECK(check_trainer<
          ::train::GradientDescendConjugate<::train::YundaSearcher>>(context,
                                                                     1));
  }
  SECTION("Quasi Newton") {
    CHECK(check_trainer<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(context,
                                                                       1));
  }
}

TEST_CASE("Medium random field tuning", "[train]") {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(3, "B");
  VariablePtr C = make_variable(3, "C");
  VariablePtr D = make_variable(3, "D");
  VariablePtr E = make_variable(3, "E");

  RandomField reference_model;
  reference_model.copyConstFactor(FactorExponential{Indicator{A, 0}, 1.f});
  reference_model.addTunableFactor(make_corr_expfactor2(A, B, 2.f));
  reference_model.addTunableFactor(make_corr_expfactor2(A, C, 0.5f));
  reference_model.addTunableFactor(make_corr_expfactor2(A, D, 2.f));
  reference_model.addTunableFactor(make_corr_expfactor2(A, E, 0.5f));

  RandomField to_tune(reference_model);

  SECTION("Full training set") {
    CheckContext context(
        Models{reference_model, to_tune},
        TrainSet(reference_model.getHiddenSetSamples(
            GibbsSampler::SamplesGenerationContext{1000, 20, 0})));

    SECTION("Gradient Descend Fixed") {
      context.trainer_setting = [](::train::IterativeTrainer &trainer) {
        static_cast<::train::GradientDescendFixed &>(trainer)
            .setOptimizationStep(0.5f);
      };
      auto threads = GENERATE(1, 2, 4);
      CHECK(check_trainer<::train::GradientDescendFixed>(context, threads));
    }
    SECTION("Gradient Descend Adaptive") {
      CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
          context, 1));
    }
    SECTION("Gradient Descend Conjugate") {
      CHECK(check_trainer<
            ::train::GradientDescendConjugate<::train::YundaSearcher>>(context,
                                                                       1));
    }
    SECTION("Quasi Newton") {
      CHECK(check_trainer<
            ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
          context, 1));
    }
  }

  SECTION("Stochastic training set") {
    CheckContext context(
        Models{reference_model, to_tune},
        TrainSet(reference_model.getHiddenSetSamples(
            GibbsSampler::SamplesGenerationContext{3000, 20, 0})));
    context.check_likelihood_trend_toll.reset();
    context.stoch_percentage = 0.2f;

    SECTION("Gradient Descend Fixed") {
      context.trainer_setting = [](::train::IterativeTrainer &trainer) {
        static_cast<::train::GradientDescendFixed &>(trainer)
            .setOptimizationStep(0.3f);
      };
      CHECK(check_trainer<::train::GradientDescendFixed>(context, 1));
    }
    SECTION("Gradient Descend Adaptive") {
      CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
          context, 1));
    }
    SECTION("Gradient Descend Conjugate") {
      CHECK(check_trainer<
            ::train::GradientDescendConjugate<::train::YundaSearcher>>(context,
                                                                       1));
    }
    SECTION("Quasi Newton") {
      CHECK(check_trainer<
            ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
          context, 1));
    }
  }
}

TEST_CASE("Small conditional random field tuning", "[train]") {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(3, "B");
  VariablePtr C = make_variable(3, "C");

  RandomField reference_model_temp;
  reference_model_temp.copyConstFactor(FactorExponential{Indicator{A, 0}, 1.f});
  reference_model_temp.addTunableFactor(make_corr_expfactor2(A, B, 2.f));
  reference_model_temp.addTunableFactor(make_corr_expfactor2(A, C, 0.5f));
  reference_model_temp.setEvidence(B, 0);
  reference_model_temp.setEvidence(C, 0);
  ConditionalRandomField reference_model(reference_model_temp, false);

  ConditionalRandomField to_tune(reference_model);
  Models models{reference_model, to_tune};

  CheckContext context(
      Models{reference_model, to_tune},
      TrainSet{reference_model.makeTrainSet(
          GibbsSampler::SamplesGenerationContext{50, 20, 0}, 1.f)});
  throw std::runtime_error{"comment makeTrainSet(...) and check above values"};

  SECTION("Gradient Descend Fixed") {
    context.trainer_setting = [](::train::IterativeTrainer &trainer) {
      static_cast<::train::GradientDescendFixed &>(trainer).setOptimizationStep(
          0.5f);
    };
    CHECK(check_trainer<::train::GradientDescendFixed>(context, 1));
  }
  SECTION("Gradient Descend Adaptive") {
    CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
        context, 1));
  }
  SECTION("Gradient Descend Conjugate") {
    CHECK(check_trainer<
          ::train::GradientDescendConjugate<::train::YundaSearcher>>(context,
                                                                     1));
  }
  SECTION("Quasi Newton") {
    CHECK(check_trainer<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(context,
                                                                       1));
  }
}