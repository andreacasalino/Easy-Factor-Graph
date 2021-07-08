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

class LearnableTest
    : public ::testing::Test {
protected:
    LearnableTest() = default;

    virtual std::unique_ptr<train::Trainable> getReferenceModel() const = 0;

    virtual train::TrainSetPtr getTrainSet() {
        EFG::strct::GibbsSampler* sampler = dynamic_cast<EFG::strct::GibbsSampler*>(this->referenceModel.get());
        return std::make_unique<train::TrainSet>(sampler->getHiddenSetSamples(500, 50));
    };

    std::unique_ptr<train::Trainable> referenceModel;
    std::unique_ptr<train::Trainable> trainedModel;
    train::TrainSetPtr trainSet;
    std::list<DistributionFinder> finders;

    void SetUp() override {
        this->referenceModel = this->getReferenceModel();
        this->trainedModel = this->getReferenceModel();
        this->trainSet = this->getTrainSet();
        auto vars = this->trainedModel->getVariables();
        EFG::strct::StructureAware* structure = dynamic_cast<EFG::strct::StructureAware*>(this->trainedModel.get());
        for (auto it = structure->getAllFactors().begin(); it != structure->getAllFactors().end(); ++it) {
            this->finders.emplace_back(**it, vars);
        }
    }

    template<typename Trainer>
    void useTrainer() {
        this->trainedModel->setOnes();
        TrainerStoryAware<Trainer> trainer;
        trainer.setMaxIterations(30);
        trainer.train(*this->trainedModel, trainSet);
        std::cout << "Iterations done " << trainer.getDescendStory().size() << std::endl;
        // check marginal computation
        auto vars = this->referenceModel->getVariables();
        std::vector<float> distrLearnt, distrReal;
        EFG::strct::EvidencesChanger* referenceSetter = dynamic_cast<EFG::strct::EvidencesChanger*>(this->referenceModel.get());
        if (nullptr != referenceSetter) {
            // random field
            referenceSetter->resetEvidences({ {(*vars.rbegin())->name(), 0} });
            dynamic_cast<EFG::strct::EvidencesChanger*>(this->trainedModel.get())->resetEvidences({ {(*vars.rbegin())->name(), 0} });
        }
        distrReal = dynamic_cast<EFG::strct::QueryHandler*>(this->referenceModel.get())->getMarginalDistribution((*vars.rbegin())->name());
        distrLearnt = dynamic_cast<EFG::strct::QueryHandler*>(this->trainedModel.get())->getMarginalDistribution((*vars.rbegin())->name());

        EXPECT_LE(fabs(distrLearnt.front() - distrReal.front()), 0.1f);
        EXPECT_LE(fabs(distrLearnt.back() - distrReal.back()), 0.1f);
        // check tuned values
        auto finalWeight = this->trainedModel->getWeights();
        {
            auto referenceWeight = this->referenceModel->getWeights();
            for (std::size_t k = 0; k < finalWeight.size(); ++k) {
                EXPECT_LE(fabs(finalWeight[k] - referenceWeight[k]) , 0.3f);
            }
        }
        // check decresing trend
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
};

#define TEST_TRAINERS(Model) \
    TEST_F(Model, GradientDescendFixed) { \
        this->useTrainer<train::GradientDescendFixed<train::BasicTrainSet>>(); \
    } \
    TEST_F(Model, GradientDescendAdaptive) { \
            this->useTrainer<train::GradientDescend<train::BasicTrainSet, train::YundaSearcher>>(); \
    } \
    TEST_F(Model, GradientDescendConjugate) { \
            this->useTrainer<train::GradientDescendConjugate<train::BasicTrainSet, train::YundaSearcher, train::FletcherReeves>>(); \
    } \
    TEST_F(Model, QuasiNewton) { \
            this->useTrainer<train::QuasiNewton<train::BasicTrainSet, train::YundaSearcher, train::BFGS>>(); \
    } 


class SmallRandomField : public LearnableTest {
public:
    SmallRandomField() = default;
protected:
    std::unique_ptr<train::Trainable> getReferenceModel() const override {
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

class MediumRandomField : public LearnableTest {
public:
    MediumRandomField() = default;
protected:
    std::unique_ptr<train::Trainable> getReferenceModel() const override {
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

//class SmallConditionalRandomField : public LearnableTest {
//public:
//    SmallConditionalRandomField() = default;
//protected:
//    std::unique_ptr<train::Trainable> getReferenceModel() const override {
//        VariablePtr A = std::make_shared<Variable>(3, "A");
//        VariablePtr B = std::make_shared<Variable>(3, "B");
//        VariablePtr C = std::make_shared<Variable>(3, "C");
//
//        model::RandomField model;
//        model.insert(std::make_shared < factor::cnst::FactorExponential>(factor::cnst::IndicatorFactor(A, 0), 1.f));
//        model.insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, B}, true), 2.f));
//        model.insertTunable(std::make_shared < factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{A, C}, true), 0.5f));
//        model.resetEvidences({ {"B", 0},{"C", 0} });
//        return std::make_unique<ConditionalRandomField>(model);
//    }
//
//    train::TrainSetPtr getTrainSet(const std::size_t iter) {
//        EFG::strct::GibbsSampler* sampler = dynamic_cast<EFG::strct::GibbsSampler*>(this->referenceModel.get());
//        EFG::strct::EvidencesSetter* evHndl = dynamic_cast<EFG::strct::EvidencesSetter*>(this->referenceModel.get());
//        categoric::Range range(sampler->getHiddenVariables());
//        std::vector<categoric::Combination> samples;
//        samples.reserve(categoric::Group(sampler->getHiddenVariables()).size() * iter);
//        iterator::forEach(range, [&](const categoric::Range& r) {
//            evHndl->setEvidences(r.get());
//            auto temp = sampler->getHiddenSetSamples(iter, 50);
//            for (auto it = temp.begin(); it != temp.end(); ++it) {
//                samples.push_back({it->data()[0] , r.get().data()[0], r.get().data()[1] });
//            }
//        });
//        return std::make_unique<train::TrainSet>(samples);
//    };
//};

TEST_TRAINERS(SmallRandomField);

TEST_TRAINERS(MediumRandomField);

//TEST_TRAINERS(SmallConditionalRandomField);

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}