#include <gtest/gtest.h>
#include <math.h>
#include <model/RandomField.h>
#include <model/ConditionalRandomField.h>
#include <categoric/Range.h>
#include <distribution/DistributionFinder.h>
#include <distribution/factor/const/Indicator.h>

#include <trainers/GradientDescendFixed.h>
#include <trainers/GradientDescend.h>
#include <trainers/QuasiNewton.h>
#include <trainers/GradientDescendConjugate.h>
#include <trainers/strategies/StochasticTrainSet.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;


template<typename TrainerT>
class TrainerStoryAware : public TrainerT {
public:
    inline const std::list<train::Vect>& getDescendStory() const { return this->descendStory; };

protected:
    void update() override {
        this->descendStory.push_back(this->model->getWeights());
        this->TrainerT::update();
    };

    void reset() override {
        this->descendStory.clear();
        this->TrainerT::reset();
    }

    std::list<train::Vect> descendStory;
};

class LikelihoodAware {
public:
    LikelihoodAware(const train::Trainable& model) {
        this->vars = model.getVariables();
        const EFG::strct::StructureAware* str = dynamic_cast<const EFG::strct::StructureAware*>(&model);
        for (auto it = str->getAllFactors().begin(); it != str->getAllFactors().end(); ++it) {
            this->finders.emplace_back(**it, vars);
        }
    };

    float getLogActivation(const EFG::categoric::Combination& c) const {
        float res = 0.f;
        for (auto it = this->finders.begin(); it != this->finders.end(); ++it) {
            res += logf(it->find(c).second);
        }
        return res;
    };

    float getLogLikeliHood(const train::TrainSetPtr& trainSet) {
        float Z = 0.f;
        {
            EFG::categoric::Range group(this->vars);
            EFG::iterator::forEach(group, [this, &Z](const EFG::categoric::Range& r) {
                Z += this->getLogActivation(r.get());
                });
        }
        float lkl = 0.f, coeff = 1.f / static_cast<float>(trainSet->getSet().size());
        for (auto it = trainSet->getSet().begin(); it != trainSet->getSet().end(); ++it) {
            lkl += coeff * this->getLogActivation(**it);
        }
        return lkl - Z;
    }

private:
    std::set<categoric::VariablePtr> vars;
    std::list<DistributionFinder> finders;
};

class TrainTest
    : public ::testing::Test {
protected:
    TrainTest() = default;

    virtual std::unique_ptr<train::Trainable> getModel() const = 0;

    virtual train::TrainSetPtr getTrainSet() {
        EFG::strct::GibbsSampler* sampler = dynamic_cast<EFG::strct::GibbsSampler*>(info->referenceModel.get());
        return std::make_unique<train::TrainSet>(sampler->getHiddenSetSamples(500, 50));
    };

    struct Info {
        std::unique_ptr<train::Trainable> referenceModel;
        std::unique_ptr<train::Trainable> trainedModel;
        train::TrainSetPtr                trainSet;
        std::unique_ptr<LikelihoodAware>  evaluator;
    };
    static std::unique_ptr<Info> info;

    float wErrToll = 0.3f;
    void checkWeights() const {
        // check tuned values
        auto finalWeight = info->trainedModel->getWeights();
        {
            auto referenceWeight = info->referenceModel->getWeights();
            for (std::size_t k = 0; k < finalWeight.size(); ++k) {
                EXPECT_LE(fabs(finalWeight[k] - referenceWeight[k]), this->wErrToll);
            }
        }
    }
    void checkMarginals() {
        auto vars = info->referenceModel->getVariables();
        std::vector<float> distrLearnt, distrReal;
        EFG::strct::EvidencesChanger* referenceSetter = dynamic_cast<EFG::strct::EvidencesChanger*>(info->referenceModel.get());
        if (nullptr != referenceSetter) {
            // random field
            referenceSetter->resetEvidences({ {(*vars.rbegin())->name(), 0} });
            dynamic_cast<EFG::strct::EvidencesChanger*>(info->trainedModel.get())->resetEvidences({ {(*vars.rbegin())->name(), 0} });
        }
        distrReal = dynamic_cast<EFG::strct::QueryHandler*>(info->referenceModel.get())->getMarginalDistribution((*vars.rbegin())->name());
        distrLearnt = dynamic_cast<EFG::strct::QueryHandler*>(info->trainedModel.get())->getMarginalDistribution((*vars.rbegin())->name());

        EXPECT_LE(fabs(distrLearnt.front() - distrReal.front()), 0.1f);
        EXPECT_LE(fabs(distrLearnt.back() - distrReal.back()), 0.1f);
    }

    bool checkLkl = true;
    template<typename TrainerT>
    void checkLikelihood(const TrainerStoryAware<TrainerT>& trainer) {
        auto finalWeight = info->trainedModel->getWeights();
        auto story = trainer.getDescendStory();
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

    virtual void resetInfo() {
        info = std::make_unique<Info>();
        info->referenceModel = this->getModel();
        info->trainedModel = this->getModel();
        info->evaluator = std::make_unique<LikelihoodAware>(*info->trainedModel.get());
        info->trainSet = this->getTrainSet();
        std::cout << "train set sampled " << std::endl;
    }

    template<typename TrainerT>
    void useTrainer(bool updateInfo = false) {
        if (updateInfo) {
            this->resetInfo();
        }
        info->trainedModel->setOnes();
        // do training
        TrainerStoryAware<TrainerT> trainer;
        trainer.setMaxIterations(30);
        trainer.train(*info->trainedModel, info->trainSet);
        std::cout << "Iterations done " << trainer.getDescendStory().size() << std::endl;
        // check tuned values
        this->checkWeights();
        // check marginal computation
        this->checkMarginals();
        // check decresing trend
        if (this->checkLkl) {
            this->checkLikelihood(trainer);
        }
    }
};
std::unique_ptr<TrainTest::Info> TrainTest::info = nullptr;



#define TEST_TRAINERS(Model, TrainSetT) \
    TEST_F(Model, GradientDescendFixed) { \
        this->useTrainer<train::GradientDescendFixed<TrainSetT>>(true); \
    } \
    TEST_F(Model, GradientDescendAdaptive) { \
            this->useTrainer<train::GradientDescend<TrainSetT, train::YundaSearcher>>(); \
    } \
    TEST_F(Model, GradientDescendConjugate) { \
            this->useTrainer<train::GradientDescendConjugate<TrainSetT, train::YundaSearcher, train::FletcherReeves>>(); \
    } \
    TEST_F(Model, QuasiNewton) { \
            this->useTrainer<train::QuasiNewton<TrainSetT, train::YundaSearcher, train::BFGS>>(); \
    } 


class SmallRandomField : public TrainTest {
public:
    SmallRandomField() = default;
protected:
    std::unique_ptr<train::Trainable> getModel() const override {
        VariablePtr A = std::make_shared<Variable>(3, "A");
        VariablePtr B = std::make_shared<Variable>(3, "B");
        VariablePtr C = std::make_shared<Variable>(3, "C");

        std::unique_ptr<model::RandomField> model = std::make_unique<model::RandomField>();
        model->insert(std::make_shared < factor::cnst::FactorExponential>(factor::cnst::IndicatorFactor(A, 0), 1.f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
        return std::move(model);
    }
};

class MediumRandomField : public TrainTest {
public:
    MediumRandomField() = default;
protected:
    std::unique_ptr<train::Trainable> getModel() const override {
        VariablePtr A = std::make_shared<Variable>(3, "A");
        VariablePtr B = std::make_shared<Variable>(3, "B");
        VariablePtr C = std::make_shared<Variable>(3, "C");
        VariablePtr D = std::make_shared<Variable>(3, "D");
        VariablePtr E = std::make_shared<Variable>(3, "E");

        std::unique_ptr<model::RandomField> model = std::make_unique<model::RandomField>();
        model->insert(std::make_shared < factor::cnst::FactorExponential>(factor::cnst::IndicatorFactor(A, 0), 1.f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, D}, true), 2.f));
        model->insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, E}, true), 0.5f));
        return std::move(model);
    }
};

class MediumRandomFieldStoch : public MediumRandomField {
public:
    MediumRandomFieldStoch() = default;
protected:
    train::TrainSetPtr getTrainSet() override {
        EFG::strct::GibbsSampler* sampler = dynamic_cast<EFG::strct::GibbsSampler*>(info->referenceModel.get());
        return std::make_unique<train::TrainSet>(sampler->getHiddenSetSamples(3000, 50));
    };

    void SetUp() override {
        this->checkLkl = false;
		this->wErrToll = 0.37f;
    }
};

class MediumRandomFieldThreadPool : public MediumRandomField {
public:
    MediumRandomFieldThreadPool() = default;
protected:
    void resetInfo() override {
        this->MediumRandomField::resetInfo();
        dynamic_cast<strct::ThreadPoolAware*>(info->trainedModel.get())->setThreadPoolSize(3);
    }
};

class SmallConditionalRandomField : public TrainTest {
public:
    SmallConditionalRandomField() = default;
protected:
    std::unique_ptr<train::Trainable> getModel() const override {
        VariablePtr A = std::make_shared<Variable>(3, "A");
        VariablePtr B = std::make_shared<Variable>(3, "B");
        VariablePtr C = std::make_shared<Variable>(3, "C");

        model::RandomField model;
        model.insert(std::make_shared < factor::cnst::FactorExponential>(factor::cnst::IndicatorFactor(A, 0), 1.f));
        model.insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
        model.insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
        model.resetEvidences({ {"B", 0},{"C", 0} });
        return std::make_unique<ConditionalRandomField>(model);
    }

    train::TrainSetPtr getTrainSet() override {
        std::size_t iter = 50;
        std::vector<categoric::Combination> rawSamples;
        EFG::strct::GibbsSampler* sampler = dynamic_cast<EFG::strct::GibbsSampler*>(info->referenceModel.get());
        EFG::strct::NodesAware* nodes = dynamic_cast<EFG::strct::NodesAware*>(info->referenceModel.get());
        EFG::strct::EvidencesSetter* setter = dynamic_cast<EFG::strct::EvidencesSetter*>(info->referenceModel.get());
        std::set<categoric::VariablePtr> evidences = { nodes->findVariable("B"), nodes->findVariable("C") };
        rawSamples.reserve(iter * categoric::Group(evidences).size());
        categoric::Range range(evidences);
        iterator::forEach(range, [&](const categoric::Range& r) {
            std::vector<std::size_t> ev = { r.get().data()[0] , r.get().data()[1] };
            setter->setEvidences(ev);
            auto samples = sampler->getHiddenSetSamples(iter, 50);
            for (auto it = samples.begin(); it != samples.end(); ++it) {
                rawSamples.emplace_back(3);
                rawSamples.back().data()[0] = it->data()[0];
                rawSamples.back().data()[1] = ev[0];
                rawSamples.back().data()[2] = ev[1];
            }
        });
        return std::make_shared<train::TrainSet>(rawSamples);
    };
	
	void SetUp() override {
		this->wErrToll = 0.35f;
	}
};

TEST_TRAINERS(SmallRandomField, train::BasicTrainSet);

TEST_TRAINERS(MediumRandomField, train::BasicTrainSet);

TEST_TRAINERS(MediumRandomFieldStoch, train::StochasticTrainSet);

TEST_TRAINERS(SmallConditionalRandomField, train::BasicTrainSet);

TEST_TRAINERS(MediumRandomFieldThreadPool, train::BasicTrainSet);

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}