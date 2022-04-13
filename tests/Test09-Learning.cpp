#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
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

TrainSet make_train_set(GibbsSampler &subject) {
  return TrainSet{subject.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 50, 0})};
}

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

template <typename TrainerT> bool check_trainer(const Models &models) {
  //   info->trainedModel->setOnes();
  //   // do training
  //   StoryAware<TrainerT> trainer;
  //   trainer.setMaxIterations(30);
  //   {
  //     ::train::GradientDescendFixed *as_fixed =
  //         dynamic_cast<::train::GradientDescendFixed *>(&trainer);
  //     if (nullptr != as_fixed) {
  //       as_fixed->setOptimizationStep(0.5);
  //     }
  //   }
  //   if (nullptr != train_set_percentage) {
  //     info->trainedModel->train(trainer, info->trainSet,
  //     *train_set_percentage);
  //   } else {
  //     info->trainedModel->train(trainer, info->trainSet);
  //   }
  //   std::cout << "Iterations done " << trainer.getStory().size() <<
  //   std::endl;
  //   // check tuned values
  //   this->checkWeights();
  //   // check marginal computation
  //   this->checkMarginals();
  //   // check decresing trend
  //   if (this->checkLkl) {
  //     this->checkLikelihood(trainer);
  //   }
}

struct Info {
  float stoch_percentage = 1.f;
  bool use_adaptive_descend = true;
  bool use_conjugate = true;
  bool use_newton = true;
  // TODO tollerances
};
bool check_trainers(const Models &models, const TrainSet &train_set,
                    const Info &info) {
  // use SECTION to refactor this, calling check_trainer every time

  //   this->checkGradient();
  // this->checkTrainer<::train::GradientDescendFixed>();
  // if (use_adaptive_descend) {
  //   this->checkTrainer<::train::GradientDescend<::train::YundaSearcher>>();
  // }
  // if (use_conjugate) {
  //   this->checkTrainer<::train::GradientDescendConjugate<
  //       ::train::YundaSearcher, ::train::FletcherReeves>>();
  // }
  // if (use_newton) {
  //   this->checkTrainer<
  //       ::train::QuasiNewton<::train::YundaSearcher, ::train::BFGS>>();
  // }
}

class TrainTest : public ::testing::Test {
protected:
  struct Info {
    std::unique_ptr<EFG::train::Trainable> referenceModel;
    std::unique_ptr<EFG::train::Trainable> trainedModel;
    EFG::train::TrainSetPtr trainSet;
    std::unique_ptr<LikelihoodAware> evaluator;
  };
  std::unique_ptr<Info> info;
  std::unique_ptr<float> train_set_percentage;
  void SetUp() override {
    info = std::make_unique<Info>();
    info->referenceModel = this->getModel();
    info->trainedModel = this->getModel();
    info->evaluator =
        std::make_unique<LikelihoodAware>(*info->trainedModel.get());
    info->trainSet = this->getTrainSet();
    std::cout << "train set sampled " << std::endl;
  }

  bool checkLkl = true;
  template <typename TrainerT>
  void checkLikelihood(const StoryAware<TrainerT> &trainer) {
    auto finalWeight = info->trainedModel->getWeights();
    auto story = trainer.getStory();
    auto it = story.begin();
    info->trainedModel->setWeights(*it);
    float lastLkl = info->evaluator->getLogLikeliHood(info->trainSet);
    ++it;
    for (it; it != story.end(); ++it) {
      info->trainedModel->setWeights(*it);
      float lkl = info->evaluator->getLogLikeliHood(info->trainSet);
      EXPECT_GE(lastLkl, lkl);
      lastLkl = lkl;
    }
    info->trainedModel->setWeights(finalWeight);
  }
};
} // namespace

// class SmallRandomField : public TrainTest {
// public:
//   SmallRandomField() = default;

// protected:
//   std::unique_ptr<EFG::train::Trainable> getModel() const override {
//     VariablePtr A = std::make_shared<Variable>(3, "A");
//     VariablePtr B = std::make_shared<Variable>(3, "B");
//     VariablePtr C = std::make_shared<Variable>(3, "C");

//     std::unique_ptr<model::RandomField> model =
//         std::make_unique<model::RandomField>();
//     model->insert(std::make_shared<factor::cnst::FactorExponential>(
//         factor::cnst::IndicatorFactor(A, 0), 1.f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
//     return std::move(model);
//   }
// };

// class MediumRandomField : public TrainTest {
// public:
//   MediumRandomField() = default;

// protected:
//   std::unique_ptr<EFG::train::Trainable> getModel() const override {
//     VariablePtr A = std::make_shared<Variable>(3, "A");
//     VariablePtr B = std::make_shared<Variable>(3, "B");
//     VariablePtr C = std::make_shared<Variable>(3, "C");
//     VariablePtr D = std::make_shared<Variable>(3, "D");
//     VariablePtr E = std::make_shared<Variable>(3, "E");

//     std::unique_ptr<model::RandomField> model =
//         std::make_unique<model::RandomField>();
//     model->insert(std::make_shared<factor::cnst::FactorExponential>(
//         factor::cnst::IndicatorFactor(A, 0), 1.f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, D}, true), 2.f));
//     model->insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, E}, true), 0.5f));
//     return std::move(model);
//   }
// };

// class MediumRandomFieldStoch : public MediumRandomField {
// public:
//   MediumRandomFieldStoch() = default;

// protected:
//   EFG::train::TrainSetPtr getTrainSet() override {
//     EFG::strct::GibbsSampler *sampler =
//         dynamic_cast<EFG::strct::GibbsSampler *>(info->referenceModel.get());
//     return std::make_unique<EFG::train::TrainSet>(
//         sampler->getHiddenSetSamples(3000, 50));
//   };

//   void SetUp() override {
//     this->MediumRandomField::SetUp();
//     this->checkLkl = false;
//     this->wErrToll = 0.37f;
//     this->train_set_percentage = std::make_unique<float>(0.1f);

//     this->use_adaptive_descend = false;
//     this->use_conjugate = false;
//     this->use_newton = false;
//   }
// };

// class MediumRandomFieldThreadPool : public MediumRandomField {
// public:
//   MediumRandomFieldThreadPool() = default;

// protected:
//   void SetUp() override {
//     this->MediumRandomField::SetUp();
//     dynamic_cast<strct::ThreadPoolAware *>(info->trainedModel.get())
//         ->setThreadPoolSize(3);
//   }
// };

// class SmallConditionalRandomField : public TrainTest {
// public:
//   SmallConditionalRandomField() = default;

// protected:
//   std::unique_ptr<EFG::train::Trainable> getModel() const override {
//     VariablePtr A = std::make_shared<Variable>(3, "A");
//     VariablePtr B = std::make_shared<Variable>(3, "B");
//     VariablePtr C = std::make_shared<Variable>(3, "C");

//     model::RandomField model;
//     model.insert(std::make_shared<factor::cnst::FactorExponential>(
//         factor::cnst::IndicatorFactor(A, 0), 1.f));
//     model.insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
//     model.insertTunable(std::make_shared<factor::modif::FactorExponential>(
//         factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
//     model.resetEvidences({{"B", 0}, {"C", 0}});
//     return std::make_unique<ConditionalRandomField>(model);
//   }

//   EFG::train::TrainSetPtr getTrainSet() override {
//     std::size_t iter = 50;
//     std::vector<categoric::Combination> rawSamples;
//     EFG::strct::GibbsSampler *sampler =
//         dynamic_cast<EFG::strct::GibbsSampler *>(info->referenceModel.get());
//     EFG::strct::NodesAware *nodes =
//         dynamic_cast<EFG::strct::NodesAware *>(info->referenceModel.get());
//     EFG::strct::EvidencesSetter *setter =
//         dynamic_cast<EFG::strct::EvidencesSetter
//         *>(info->referenceModel.get());
//     std::set<categoric::VariablePtr> evidences = {nodes->findVariable("B"),
//                                                   nodes->findVariable("C")};
//     rawSamples.reserve(iter * categoric::Group(evidences).size());
//     categoric::Range range(evidences);
//     iterator::forEach(range, [&](const categoric::Range &r) {
//       std::vector<std::size_t> ev = {r.get().data()[0], r.get().data()[1]};
//       setter->setEvidences(ev);
//       auto samples = sampler->getHiddenSetSamples(iter, 50);
//       for (auto it = samples.begin(); it != samples.end(); ++it) {
//         rawSamples.emplace_back(3);
//         rawSamples.back().data()[0] = it->data()[0];
//         rawSamples.back().data()[1] = ev[0];
//         rawSamples.back().data()[2] = ev[1];
//       }
//     });
//     return std::make_shared<EFG::train::TrainSet>(rawSamples);
//   };

//   void SetUp() override {
//     this->TrainTest::SetUp();
//     this->wErrToll = 0.35f;
//   }
// };

// TEST_F(SmallRandomField, check) { this->CHECK(); }

// TEST_F(MediumRandomField, check) { this->CHECK(); }

// TEST_F(MediumRandomFieldStoch, check) { this->CHECK(); }

// TEST_F(SmallConditionalRandomField, check) { this->CHECK(); }

// TEST_F(MediumRandomFieldThreadPool, check) { this->CHECK(); }
