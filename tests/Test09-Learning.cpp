#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <EasyFactorGraph/trainable/ModelTrainer.h>

#include <TrainingTools/iterative/solvers/GradientDescend.h>
#include <TrainingTools/iterative/solvers/GradientDescendConjugate.h>
#include <TrainingTools/iterative/solvers/QuasiNewton.h>

#include <algorithm>
#include <limits>
#include <math.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;
using namespace EFG::train;
using namespace EFG::strct;

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

bool are_similar(const std::vector<float> &a, const std::vector<float> &b,
                 const float toll = 0.2f) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (abs(a[k] - b[k]) > toll) {
      return false;
    }
  }
  return true;
}

VariablePtr get_second_hidden(const ConnectionsManager &subject) {
  auto vars = subject.getHiddenVariables();
  auto it = vars.begin();
  ++it;
  return *it;
}

struct Models {
  FactorsTunableAware &reference_model;
  FactorsTunableAware &trained_model;
};
bool is_gradient_in_right_direction(const Models &subject,
                                    const TrainSet::Iterator &train_set) {
  auto gradient = subject.trained_model.getWeightsGradient(train_set);
  auto w_a = subject.reference_model.getWeights();
  auto w_b = subject.trained_model.getWeights();
  if (gradient.size() != w_b.size()) {
    return false;
  }
  auto same_sign = [](const float a, const float b) {
    if (a > 0) {
      return b > 0;
    }
    return b < 0;
  };
  for (std::size_t k = 0; k < gradient.size(); ++k) {
    if (!same_sign(gradient[k], w_a[k] - w_b[k])) {
      return false;
    }
  }
  return true;
}

bool are_marginals_similar(const Models &subject) {
  auto *reference_as_random_field =
      dynamic_cast<RandomField *>(&subject.reference_model);
  if (nullptr == reference_as_random_field) {
    reference_as_random_field->setEvidence(
        get_second_hidden(*reference_as_random_field), 0);
    auto *trained_as_random_field =
        dynamic_cast<RandomField *>(&subject.trained_model);
    trained_as_random_field->setEvidence(
        get_second_hidden(*trained_as_random_field), 0);
  }

  std::string variable_to_query =
      subject.reference_model.getEvidences().begin()->first->name();

  auto *reference_query_handler =
      dynamic_cast<QueryManager *>(&subject.reference_model);
  auto *trained_query_handler =
      dynamic_cast<QueryManager *>(&subject.trained_model);
  return are_similar(
      reference_query_handler->getMarginalDistribution(variable_to_query),
      trained_query_handler->getMarginalDistribution(variable_to_query), 0.1f);
}

bool has_likelihood_decreasing_trend(
    FactorsTunableAware &trained_model,
    const EFG::train::TrainSet::Iterator &combinations,
    const std::list<std::vector<float>> &story, const float toll = 0.1f) {
  const auto final_w = trained_model.getWeights();
  LikelihoodAware likelihood_aware(trained_model);
  float prev_likelihood = std::numeric_limits<float>::min();
  for (const auto &w : story) {
    trained_model.setWeights(w);
    float att_likelihood = likelihood_aware.getLogLikeliHood(combinations);
    const bool ok = (-toll) < (att_likelihood - prev_likelihood);
    if (!ok) {
      return false;
    }
    prev_likelihood - att_likelihood;
  }
  trained_model.setWeights(final_w);
  return true;
}

template <typename TrainerT>
bool check_trainer(const Models &models, const TrainSet &train_set,
                   const float stoch_percentage, const bool check_Lkl = true,
                   const std::size_t threads = 1) {
  set_ones(models.trained_model);
  StoryAware<TrainerT> trainer;
  trainer.setMaxIterations(30);
  {
    ::train::GradientDescendFixed *as_fixed =
        dynamic_cast<::train::GradientDescendFixed *>(&trainer);
    if (nullptr != as_fixed) {
      as_fixed->setOptimizationStep(0.5);
    }
  }
  TrainInfo info;
  info.stochastic_percentage = stoch_percentage;
  info.threads = threads;
  train_model(models.trained_model, trainer, train_set, info);
  // check tuned values
  if (!are_similar(models.reference_model.getWeights(),
                   models.trained_model.getWeights())) {
    return false;
  }
  // check marginal computation
  if (!are_marginals_similar(models)) {
    return false;
  }
  // check decresing trend
  if (check_Lkl && (!has_likelihood_decreasing_trend(models.trained_model,
                                                     train_set.makeIterator(),
                                                     trainer.getStory()))) {
    return false;
  }
  return true;
}

std::shared_ptr<FactorExponential>
make_corr_factor(const VariablePtr &first, const VariablePtr &second,
                 const float w, const bool correlating_nature = true) {
  std::unique_ptr<Factor> factor;
  if (correlating_nature) {
    factor = std::make_unique<Factor>(Group{first, second},
                                      USE_SIMPLE_CORRELATION_TAG);
  } else {
    factor = std::make_unique<Factor>(Group{first, second},
                                      USE_SIMPLE_ANTI_CORRELATION_TAG);
  }
  return std::make_shared<FactorExponential>(*factor, w);
}
} // namespace

TEST_CASE("Small random field tuning", "[train]") {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(3, "B");
  VariablePtr C = make_variable(3, "C");

  RandomField reference_model;
  reference_model.copyConstFactor(FactorExponential{Indicator{A, 0}, 1.f});
  reference_model.addTunableFactor(make_corr_factor(A, B, 2.f, true));
  reference_model.addTunableFactor(make_corr_factor(A, C, 0.5f, true));

  RandomField to_tune(reference_model);
  Models models{reference_model, to_tune};

  TrainSet train_set(reference_model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 20, 0}));

  REQUIRE(is_gradient_in_right_direction(models, train_set.makeIterator()));

  SECTION("Gradient Descend Fixed") {
    CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f));
  }
  SECTION("Gradient Descend Adaptive") {
    CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
        models, train_set, 1.f));
  }
  SECTION("Gradient Descend Conjugate") {
    CHECK(check_trainer<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
        models, train_set, 1.f));
  }
  SECTION("Quasi Newton") {
    CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f));
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
  reference_model.addTunableFactor(make_corr_factor(A, B, 2.f, true));
  reference_model.addTunableFactor(make_corr_factor(A, C, 0.5f, true));
  reference_model.addTunableFactor(make_corr_factor(A, D, 2.f, true));
  reference_model.addTunableFactor(make_corr_factor(A, E, 0.5f, true));

  RandomField to_tune(reference_model);
  Models models{reference_model, to_tune};

  SECTION("Full training set") {
    TrainSet train_set(reference_model.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{100, 10, 0}));

    REQUIRE(is_gradient_in_right_direction(models, train_set.makeIterator()));

    SECTION("Gradient Descend Fixed") {
      auto threads = GENERATE(1, 3);
      CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f,
                                                         true, threads));
    }
    SECTION("Gradient Descend Adaptive") {
      CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
          models, train_set, 1.f));
    }
    SECTION("Gradient Descend Conjugate") {
      CHECK(check_trainer<
            ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
          models, train_set, 1.f));
    }
    SECTION("Quasi Newton") {
      CHECK(
          check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f));
    }
  }

  SECTION("Stochastic training set") {
    TrainSet train_set(reference_model.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{1000, 50, 0}));

    const float percentage = 0.1f;

    SECTION("Gradient Descend Fixed") {
      CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set,
                                                         percentage, false));
    }
    SECTION("Gradient Descend Adaptive") {
      CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
          models, train_set, percentage, false));
    }
    SECTION("Gradient Descend Conjugate") {
      CHECK(check_trainer<
            ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
          models, train_set, percentage, false));
    }
    SECTION("Quasi Newton") {
      CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set,
                                                         percentage, false));
    }
  }
}

TEST_CASE("Small conditional random field tuning", "[train]") {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(3, "B");
  VariablePtr C = make_variable(3, "C");

  RandomField reference_model_temp;
  reference_model_temp.copyConstFactor(FactorExponential{Indicator{A, 0}, 1.f});
  reference_model_temp.addTunableFactor(make_corr_factor(A, B, 2.f, true));
  reference_model_temp.addTunableFactor(make_corr_factor(A, C, 0.5f, true));
  reference_model_temp.setEvidence(B, 0);
  reference_model_temp.setEvidence(C, 0);
  ConditionalRandomField reference_model(reference_model_temp, false);

  ConditionalRandomField to_tune(reference_model);
  Models models{reference_model, to_tune};

  TrainSet train_set(reference_model.makeTrainSet(
      GibbsSampler::SamplesGenerationContext{100, 10, 0}, 0.5f));
  throw std::runtime_error{"Compute train set of conditional random in field "
                           "in a funciton inside library"};

  REQUIRE(is_gradient_in_right_direction(models, train_set.makeIterator()));

  SECTION("Gradient Descend Fixed") {
    CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f));
  }
  SECTION("Gradient Descend Adaptive") {
    CHECK(check_trainer<::train::GradientDescend<::train::YundaSearcher>>(
        models, train_set, 1.f));
  }
  SECTION("Gradient Descend Conjugate") {
    CHECK(check_trainer<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>(
        models, train_set, 1.f));
  }
  SECTION("Quasi Newton") {
    CHECK(check_trainer<::train::GradientDescendFixed>(models, train_set, 1.f));
  }
}
