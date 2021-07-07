#include <gtest/gtest.h>
#include <model/RandomField.h>
#include <categoric/Range.h>
#include <distribution/DistributionFinder.h>
#include <distribution/factor/const/Indicator.h>

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

class LearnableTest
    : public ::testing::Test {
protected:
    LearnableTest() = default;

    virtual std::unique_ptr<model::RandomField> getReferenceModel() const = 0;

    std::unique_ptr<model::RandomField> referenceModel;

    std::unique_ptr<model::RandomField> trainedModel;
    std::list<DistributionFinder> finders;

    void SetUp() override {
        this->referenceModel = this->getReferenceModel();
        this->trainedModel = std::make_unique<model::RandomField>();
        this->trainedModel->absorbModel(*this->referenceModel, true);
        auto vars = this->trainedModel->getVariables();
        for (auto it = this->trainedModel->getAllFactors().begin(); it != this->trainedModel->getAllFactors().end(); ++it) {
            this->finders.emplace_back(**it, vars);
        }
    }

    template<typename Trainer>
    void useTrainer(train::TrainSetPtr trainSet) {
        this->trainedModel->setOnes();
        TrainerStoryAware<Trainer> trainer;
        trainer.setMaxIterations(20);
        trainer.train(*this->trainedModel, trainSet);
        // check marginal compuation
        auto vars = this->referenceModel->getVariables();
        this->trainedModel->resetEvidences({ {(*vars.begin())->name(), 0} });
        this->referenceModel->resetEvidences({ {(*vars.begin())->name(), 0} });
        auto distrLearnt = this->trainedModel->getMarginalDistribution((*vars.rbegin())->name());
        auto distrReal = this->referenceModel->getMarginalDistribution((*vars.rbegin())->name());
        EXPECT_LE(fabsf(distrLearnt.front() - distrReal.front()), 0.1f);
        EXPECT_LE(fabsf(distrLearnt.back()  - distrReal.back()), 0.1f);
        // check decresing trend
        auto finalWeight = this->trainedModel->getWeights();
        auto story = trainer.getDescendStory();
        auto it = story.begin();
        this->trainedModel->setWeights(*it);
        float lastLkl = this->getLikeliHood(trainSet);
        ++it;
        for (it; it != story.end(); ++it) {
            this->trainedModel->setWeights(*it);
            float lkl = this->getLikeliHood(trainSet);
            EXPECT_GE(lastLkl, lkl);
            lastLkl = lkl;
        }
        this->trainedModel->setWeights(finalWeight);
    };

    float getLogActivation(const EFG::categoric::Combination& c) const {
        float res = 0.f;
        for (auto it = this->finders.begin(); it != this->finders.end(); ++it) {
            res += logf(it->find(c).second);
        }
        return res;
    };

    float getLikeliHood(const train::TrainSetPtr& trainSet) {
        float Z = 0.f;
        {
            EFG::categoric::Range group(this->trainedModel->getVariables());
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

    void useAllTrainers() {
        std::cout << "sampling train set" << std::endl;
        train::TrainSetPtr trainSet = std::make_shared<train::TrainSet>(this->referenceModel->getHiddenSetSamples(500, 50));

        std::cout << "Gradient descend start";
        this->useTrainer<train::GradientDescend<train::BasicTrainSet, train::YundaSearcher>>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "QuasiNewton descend start";
        this->useTrainer<train::QuasiNewton<train::BasicTrainSet, train::YundaSearcher, train::BFGS>>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "GradientDescendConjugate descend start";
        this->useTrainer<train::GradientDescendConjugate<train::BasicTrainSet, train::YundaSearcher, train::FletcherReeves>>(trainSet);
        std::cout << " stop" << std::endl;
    };

    void useAllTrainersStoch() {
        std::cout << "sampling train set" << std::endl;
        train::TrainSetPtr trainSet = std::make_shared<train::TrainSet>(this->referenceModel->getHiddenSetSamples(2000, 50));

        std::cout << "Gradient descend start";
        this->useTrainer<train::GradientDescend<train::StochasticTrainSet, train::YundaSearcher>>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "QuasiNewton descend start";
        this->useTrainer<train::QuasiNewton<train::StochasticTrainSet, train::YundaSearcher, train::BFGS>>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "GradientDescendConjugate descend start";
        this->useTrainer<train::GradientDescendConjugate<train::StochasticTrainSet, train::YundaSearcher, train::FletcherReeves>>(trainSet);
        std::cout << " stop" << std::endl;
    };
};

class SmallModel : public LearnableTest {
public:
    SmallModel() = default;
protected:
    std::unique_ptr<model::RandomField> getReferenceModel() const override {
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

class MediumModel : public LearnableTest {
public:
    MediumModel() = default;
protected:
    std::unique_ptr<model::RandomField> getReferenceModel() const override {
        VariablePtr A = std::make_shared<Variable>(3, "A");
        VariablePtr B = std::make_shared<Variable>(3, "B");
        VariablePtr C = std::make_shared<Variable>(3, "C");
        VariablePtr D = std::make_shared<Variable>(3, "D");
        VariablePtr E = std::make_shared<Variable>(3, "E");

        std::unique_ptr<model::RandomField> model = std::make_unique<model::RandomField>();
        model->insert(std::make_shared < factor::cnst::FactorExponential>(factor::cnst::IndicatorFactor(A, 0), 1.f));
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, D}, true), 2.f));
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, E}, true), 0.5f));
        return std::move(model);
    }
};

TEST_F(SmallModel, smallModelCompleteTrainSet) {
    std::cout << "--------------------------" << std::endl;
    this->useAllTrainers();
    EXPECT_LE(fabsf(this->referenceModel->getWeights().front() - this->trainedModel->getWeights().front()), 0.2f);
    EXPECT_LE(fabsf(this->referenceModel->getWeights().back()  - this->trainedModel->getWeights().back()), 0.2f);
    std::cout << "--------------------------" << std::endl << std::endl;
}

TEST_F(MediumModel, mediumModelCompleteTrainSet) {
    std::cout << "--------------------------" << std::endl;
    this->useAllTrainers();
    std::cout << "--------------------------" << std::endl << std::endl;
}

TEST_F(MediumModel, mediumModelStochTrainSet) {
    std::cout << "--------------------------" << std::endl;
    this->useAllTrainersStoch();
    std::cout << "--------------------------" << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}