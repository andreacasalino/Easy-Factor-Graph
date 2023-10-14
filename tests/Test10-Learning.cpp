#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/ImageFinder.h>
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

#include "ModelLibrary.h"

namespace EFG::test {
using namespace categoric;
using namespace model;
using namespace train;
using namespace strct;
using namespace library;

namespace {
template <typename ModelT> class LearningTest {
public:
  template <typename MakeModelPred> LearningTest(MakeModelPred &&pred) {
    to_train = pred();
    reference = pred();
  }

  LearningTest &threads(std::size_t threads) {
    threads_ = threads;
    return *this;
  }
  template <typename MakeSamplesPred>
  LearningTest &samplesMaker(MakeSamplesPred &&pred) {
    samplesMaker_ = std::forward<std::function<TrainSet(ModelT &)>>(pred);
    return *this;
  }
  template <typename InitTrainerPred>
  LearningTest &trainerInitialization(InitTrainerPred &&init) {
    initTrainer =
        std::forward<std::function<void(::train::IterativeTrainer &)>>(init);
    return *this;
  }
  LearningTest &maxIterations(std::size_t iter) {
    max_iterations = iter;
    return *this;
  }
  LearningTest &stochGradPecentage(float val) {
    stoch_percentage = val;
    return *this;
  }
  LearningTest &checkWeightToll(float val) {
    check_weights_toll = val;
    return *this;
  }
  LearningTest &checkLikelihoodToll(float val) {
    check_likelihood_trend_toll = val;
    return *this;
  }
  LearningTest &checkMarginalsToll(float val) {
    check_marginal_toll = val;
    return *this;
  }

  template <typename TrainerT> bool train() {
    set_ones(*to_train);
    std::vector<std::vector<float>> wStory;
    wStory.reserve(max_iterations);
    TrainerWithStory<TrainerT> trainer{wStory};
    trainer.setMaxIterations(max_iterations);
    if (initTrainer) {
      initTrainer(trainer);
    }
    TrainInfo info;
    info.stochastic_percentage = stoch_percentage;
    info.threads = threads_;
    TrainSet samples = samplesMaker_(*reference);
    train_model(*to_train, trainer, samples, info);
    /////////////////// checks ///////////////////
    return almost_equal_it(reference->getWeights(), to_train->getWeights(),
                           check_weights_toll) &&
           checkLikelihoodTrend(wStory, samples.makeIterator()) &&
           checkMarginals();
  }

protected:
  bool checkLikelihoodTrend(const std::vector<std::vector<float>> &wStory,
                            const TrainSet::Iterator &train_set) {
    if (check_likelihood_trend_toll == std::numeric_limits<float>::max()) {
      return true;
    }
    const auto final_w = to_train->getWeights();
    LikelihoodGetter likelihood_aware(*to_train);
    float prev_likelihood = -std::numeric_limits<float>::max();
    for (const auto &w : wStory) {
      to_train->setWeights(w);
      float att_likelihood = likelihood_aware.getLogLikeliHood(train_set);
      const bool ok =
          (-check_likelihood_trend_toll) < (att_likelihood - prev_likelihood);
      if (!ok) {
        return false;
      }
      prev_likelihood = att_likelihood;
    }
    to_train->setWeights(final_w);
    return true;
  }

  bool checkMarginals() {
    if constexpr (!std::is_same<ConditionalRandomField, ModelT>::value) {
      if (check_marginal_toll == std::numeric_limits<float>::max()) {
        return true;
      }

      VariablesSoup hidden_vars;
      {
        const auto &hidden_set = to_train->getHiddenVariables();
        hidden_vars = VariablesSoup{hidden_set.begin(), hidden_set.end()};
      }
      auto var_getter = [&reference =
                             *this->reference](const std::string &name) {
        return reference.findVariable(name);
      };

      dynamic_cast<EvidenceSetter *>(to_train.get())
          ->setEvidence(hidden_vars.back(), 0);
      dynamic_cast<EvidenceSetter *>(reference.get())
          ->setEvidence(var_getter(hidden_vars.back()->name()), 0);

      for (std::size_t k = 0; k < (hidden_vars.size() - 1); ++k) {
        auto marginals_trained =
            dynamic_cast<QueryManager *>(to_train.get())
                ->getMarginalDistribution(hidden_vars[k]->name());

        auto marginals_reference =
            dynamic_cast<QueryManager *>(reference.get())
                ->getMarginalDistribution(hidden_vars[k]->name());

        if (!almost_equal_it(marginals_trained, marginals_reference,
                             check_marginal_toll)) {
          return false;
        }
      }
    }
    return true;
  }

  std::unique_ptr<ModelT> reference;
  std::unique_ptr<ModelT> to_train;

  std::size_t threads_ = 1;
  std::function<TrainSet(ModelT &)> samplesMaker_;
  std::size_t max_iterations = 30;
  std::function<void(::train::IterativeTrainer &)> initTrainer;
  float stoch_percentage = 1.f;

  float check_weights_toll = 0.5f;
  float check_likelihood_trend_toll = 5.f;
  float check_marginal_toll = 0.15f;

  template <typename TrainerBase> class TrainerWithStory : public TrainerBase {
  public:
    TrainerWithStory(std::vector<std::vector<float>> &wStory)
        : wStory{wStory} {};

  protected:
    void updateDirection() override {
      Eigen::VectorXd par = this->getParameters();
      std::vector<float> w;
      w.reserve(par.size());
      for (Eigen::Index i = 0; i < par.size(); ++i) {
        w.push_back(par(i));
      }
      this->wStory.emplace_back(std::move(w));
      this->TrainerBase::updateDirection();
    };

  private:
    std::vector<std::vector<float>> &wStory;
  };
};
} // namespace

TEST_CASE("Small random field tuning", "[train]") {
  LearningTest<RandomField> info([]() {
    VariablePtr A = make_variable(3, "A");
    VariablePtr B = make_variable(3, "B");
    VariablePtr C = make_variable(3, "C");
    std::unique_ptr<RandomField> subject = std::make_unique<RandomField>();
    subject->copyConstFactor(
        factor::FactorExponential{factor::Indicator{A, 0}, 1.f});
    subject->addTunableFactor(make_corr_expfactor_ptr(A, B, 2.f));
    subject->addTunableFactor(make_corr_expfactor_ptr(A, C, 0.5f));
    return subject;
  });
  info.samplesMaker([](RandomField &reference) {
    return make_good_trainset(reference, 500);
  });

  SECTION("Gradient Descend Fixed") {
    info.trainerInitialization([](::train::IterativeTrainer &trainer) {
      static_cast<::train::GradientDescendFixed &>(trainer).setOptimizationStep(
          0.5f);
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

TEST_CASE("Medium random field tuning", "[train]") {
  LearningTest<RandomField> info([]() {
    VariablePtr A = make_variable(3, "A");
    VariablePtr B = make_variable(3, "B");
    VariablePtr C = make_variable(3, "C");
    VariablePtr D = make_variable(3, "D");
    VariablePtr E = make_variable(3, "E");

    std::unique_ptr<RandomField> subject = std::make_unique<RandomField>();
    subject->copyConstFactor(
        factor::FactorExponential{factor::Indicator{A, 0}, 1.f});
    subject->addTunableFactor(make_corr_expfactor_ptr(A, B, 2.f));
    subject->addTunableFactor(make_corr_expfactor_ptr(A, C, 0.5f));
    subject->addTunableFactor(make_corr_expfactor_ptr(A, D, 2.f));
    subject->addTunableFactor(make_corr_expfactor_ptr(A, E, 0.5f));
    return subject;
  });

  SECTION("Full training set") {
    info.samplesMaker([](RandomField &reference) {
      return make_good_trainset(reference, 1000);
    });
    SECTION("Gradient Descend Fixed") {
      info.trainerInitialization([](::train::IterativeTrainer &trainer) {
        static_cast<::train::GradientDescendFixed &>(trainer)
            .setOptimizationStep(0.2f);
      });
      auto threads = GENERATE(1, 2, 4);
      info.threads(threads);
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

  SECTION("Stochastic training set") {
    info.samplesMaker([](RandomField &reference) {
          return make_good_trainset(reference, 5000);
        })
        .checkLikelihoodToll(std::numeric_limits<float>::max())
        .stochGradPecentage(0.2f);

    SECTION("Gradient Descend Fixed") {
      info.trainerInitialization([](::train::IterativeTrainer &trainer) {
        static_cast<::train::GradientDescendFixed &>(trainer)
            .setOptimizationStep(0.2f);
      });

      CHECK(info.train<::train::GradientDescendFixed>());
    }
    ///////////////// temporarely disabled /////////////////
    // SECTION("Gradient Descend Adaptive") {
    //   CHECK(info.train<::train::GradientDescend<::train::YundaSearcher>>());
    // }
    // SECTION("Gradient Descend Conjugate") {
    //   CHECK(info.train<
    //         ::train::GradientDescendConjugate<::train::YundaSearcher>>());
    // }
    // SECTION("Quasi Newton") {
    //   CHECK(info.train<
    //         ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>());
    // }
  }
}

TEST_CASE("Small conditional random field tuning", "[train]") {
  LearningTest<ConditionalRandomField> info([]() {
    VariablePtr A = make_variable(3, "A");
    VariablePtr B = make_variable(3, "B");
    VariablePtr C = make_variable(3, "C");

    RandomField reference_model_temp;
    reference_model_temp.copyConstFactor(
        factor::FactorExponential{factor::Indicator{A, 0}, 1.f});
    reference_model_temp.addTunableFactor(make_corr_expfactor_ptr(A, B, 2.f));
    reference_model_temp.addTunableFactor(make_corr_expfactor_ptr(A, C, 0.5f));
    reference_model_temp.setEvidence(B, 0);
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
      static_cast<::train::GradientDescendFixed &>(trainer).setOptimizationStep(
          0.2f);
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

#include <sstream>

namespace {
void make_shared_w_chain(RandomField &model, std::size_t size, float alfa,
                         float beta) {
  auto make_var_name = [](char name, std::size_t pos) {
    std::stringstream stream;
    stream << name << std::to_string(pos);
    return stream.str();
  };

  auto make_vars_set = [&model](const std::vector<std::string> &names) {
    VariablesSet result;
    for (const auto &name : names) {
      result.emplace(model.findVariable(name));
    }
    return result;
  };

  for (std::size_t k = 0; k < size; ++k) {
    auto Y = make_variable(3, make_var_name('Y', k));
    auto X = make_variable(3, make_var_name('X', k));
    auto pot_XY = make_corr_expfactor_ptr(X, Y, beta);
    if (0 == k) {
      model.addTunableFactor(pot_XY);
    } else {
      model.addTunableFactor(pot_XY, make_vars_set({"X0", "Y0"}));
      auto pot_YY = make_corr_expfactor_ptr(
          Y, model.findVariable(make_var_name('Y', k - 1)), alfa);
      if (1 == k) {
        model.addTunableFactor(pot_YY);
      } else {
        model.addTunableFactor(pot_YY, make_vars_set({"Y0", "Y1"}));
      }
    }
  }
}
} // namespace

TEST_CASE("Shared weights tuning", "[train]") {
  auto chain_size = GENERATE(3, 6);

  LearningTest<RandomField> info([size = chain_size]() {
    auto subject = std::make_unique<RandomField>();
    make_shared_w_chain(*subject, size, 0.7f, 1.2f);
    return subject;
  });
  info.samplesMaker([](RandomField &reference) {
        return make_good_trainset(reference, 1000);
      })
      .checkLikelihoodToll(std::numeric_limits<float>::max())
      .checkMarginalsToll(std::numeric_limits<float>::max());

  SECTION("Gradient Descend Fixed") {
    info.trainerInitialization([](::train::IterativeTrainer &trainer) {
      static_cast<::train::GradientDescendFixed &>(trainer).setOptimizationStep(
          0.5f);
    });
    CHECK(info.train<::train::GradientDescendFixed>());
  }
  SECTION("Quasi Newton") {
    CHECK(info.train<
          ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>());
  }
}

/*
namespace {
std::vector<std::size_t> make_ones(std::size_t size) {
  std::vector<std::size_t> result;
  result.resize(size);
  for (auto &val : result) {
    val = 0;
  }
  return result;
}
} // namespace

TEST_CASE("Train with Pool efficiency", "[train][!mayfail]") {
  auto depth = GENERATE(8, 10);

  ScalableModel model{static_cast<std::size_t>(depth), 3, false};

  std::unique_ptr<TrainSet> train_set;
  {
    const std::size_t train_set_size = 500;
    std::vector<std::vector<std::size_t>> samples;
    samples.reserve(train_set_size);
    for (std::size_t k = 0; k < train_set_size; ++k) {
      samples.emplace_back(make_ones(model.getAllVariables().size()));
    }
    train_set = std::make_unique<TrainSet>(samples);
  }
  auto measure_time =
      [&](std::size_t threads,
          FactorsTunableGetter &subject) -> std::chrono::nanoseconds {
    ::train::GradientDescendFixed trainer;
    trainer.setMaxIterations(20);
    trainer.setOptimizationStep(static_cast<float>(1e-5));
    return test::measure_time([&]() {
      train_model(subject, trainer, *train_set,
                  EFG::train::TrainInfo{threads, 1.f});
    });
  };

  auto single_thread_time = measure_time(1, model);
  auto multi_thread_time = measure_time(2, model);

  CHECK(static_cast<double>(multi_thread_time.count()) <
        static_cast<double>(single_thread_time.count()));

  SECTION("conditional model") {
    model.setEvidence(model.root(), 0);
    ConditionalRandomField as_conditional_random_field(model, false);

    single_thread_time = measure_time(1, as_conditional_random_field);
    multi_thread_time = measure_time(2, as_conditional_random_field);

    CHECK(static_cast<double>(multi_thread_time.count()) <
          static_cast<double>(single_thread_time.count()));
  }
}
*/

} // namespace EFG::test
