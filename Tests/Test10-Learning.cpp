#include <gtest/gtest.h>
#include <model/RandomField.h>
#include <categoric/Range.h>
#include <distribution/DistributionFinder.h>
#include <distribution/factor/const/Indicator.h>

#include <trainers/GradientDescend.h>
#include <trainers/QuasiNewton.h>
#include <trainers/GradientDescendConjugate.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;


template<typename Trainer>
class TrainerStoryAware : public Trainer {
protected:
    void update() override {
        this->descendStory.push_back(this->model->getWeights());
        this->Trainer::update();
    };

    inline const std::list<Vec>& getDescendStory() const { return this->descendStory; };

    std::list<Vec> descendStory;
};

class LearnableTest
    : public ::testing::Test
    , public model::RandomField {
protected:
    LearnableTest() = default;

    virtual std::unique_ptr<model::RandomField> getReferenceModel() const = 0;

    std::unique_ptr<model::RandomField> referenceModel;
    std::list<DistributionFinder> finders;
    void SetUp() override {
        this->referenceModel = this->getReferenceModel();
        this->absorb(*this->referenceModel, true);
        this->setOnes();
        auto vars = this->getVariables();
        for (auto it = this->factorsAll.begin(); it != this->factorsAll.end(); ++it) {
            this->finders.emplace_back(**it, vars);
        }
    }

    template<typename Trainer>
    void useTrainer(train::TrainSetPtr trainSet) {
        TrainerStoryAware<Trainer> trainer;
        trainer.setMaxIterations(20);
        trainer.train(*this, trainSet);
        // check marginal compuation
        auto vars = this->getVariables();
        this->resetEvidences({ {(*vars.begin())->name(), 0} });
        this->referenceModel->resetEvidences({ {(*vars.begin())->name(), 0} });
        auto distrLearnt = this->getMarginalDistribution((*vars.rbegin())->name());
        auto distrReal = this->referenceModel->getMarginalDistribution((*vars.rbegin())->name());
        throw 0; // compare somehow the 2 distributions
        // check decresing trend
        auto finalWeight = this->getWeights();
        auto story = trainer.getDescendStory();
        auto it = story.begin();
        float lastLkl = this->getLikeliHood(*it, trainSet);
        ++it;
        for (it; it != story.end(); ++it) {
            float lkl = this->getLikeliHood(*it, trainSet);
            EXPECT_GE(lastLkl, lkl);
            lastLkl = lkl;
        }
        this->setWeights(finalWeight);
    };

    void useAllTrainers() {
        std::cout << "sampling train set" << std::endl;
        train::TrainSetPtr trainSet = std::make_shared<train::TrainSet>(this->referenceModel->getHiddenSetSamples(500, 50));
        std::cout << "Gradient descend start";
        this->useTrainer<train::GradientDescend>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "QuasiNewton descend start";
        this->useTrainer<train::QuasiNewton>(trainSet);
        std::cout << " stop" << std::endl;

        std::cout << "GradientDescendConjugate descend start";
        this->useTrainer<train::GradientDescendConjugate>(trainSet);
        std::cout << " stop" << std::endl;
    };

    float getLogActivation(const EFG::categoric::Combination& c) const {
        float res = 0.f;
        for (auto it = this->finders.begin(); it != this->finders.end(); ++it) {
            res += logf(it->find(c).second);
        }
        return res;
    };

    float getLikeliHood(const std::vector<float>& weights, const train::TrainSetPtr& trainSet) {
        this->setWeights(weights);
        float Z = 0.f;
        {
            EFG::categoric::Range group(this->getVariables());
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
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
        model->insert(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
        return std::move(model);
    }
};
TEST_F(SmallModel, smallModelTraining) {
    std::cout << "--------------------------" << std::endl;
    std::cout << "small model" << std::endl;
    this->useAllTrainers();
    throw 0; // check weights value
    std::cout << "--------------------------" << std::endl << std::endl;
}

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
TEST_F(MediumModel, mediumModelTraining) {
    std::cout << "--------------------------" << std::endl;
    std::cout << "medium model" << std::endl;
    this->useAllTrainers();
    throw 0; // check weights value
    std::cout << "--------------------------" << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}