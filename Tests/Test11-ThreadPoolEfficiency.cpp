#include <gtest/gtest.h>
#include <model/RandomField.h>
#include <sstream>
#include <chrono>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

constexpr std::size_t VAR_SIZE = 4;

enum Task { BeliefProp, Gibbs, Gradient };
class BinaryStructure
    : public RandomField {
public:
    BinaryStructure(const std::size_t levels, bool loopy) {
        std::size_t counter = 0;
        auto varGen = [&counter]() {
            std::stringstream stream;
            stream << "V";
            stream << counter;
            ++counter;
            return categoric::makeVariable(VAR_SIZE, stream.str());
        };

        std::list<categoric::VariablePtr> toExpand, nextExpand;
        toExpand.push_back(varGen());
        for (std::size_t k = 1; k < levels; ++k) {
            for (auto it = toExpand.begin(); it != toExpand.end(); ++it) {
                categoric::VariablePtr varA = varGen();
                categoric::VariablePtr varB = varGen();
                this->insertTunable(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<categoric::VariablePtr>{*it, varA}, true), 1.f));
                this->insertTunable(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<categoric::VariablePtr>{*it, varB}, true), 1.f));
                if (loopy) {
                    this->insertTunable(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<categoric::VariablePtr>{varA, varB}, true), 1.f));
                }
                nextExpand.push_back(varA);
                nextExpand.push_back(varB);
            }
            toExpand = nextExpand;
            nextExpand.clear();
        }
    }

    void profile(const Task& task, const std::size_t trials) {
        std::function<void(void)> t;
        train::TrainSetPtr trainSet;
        switch (task)
        {
        case Task::BeliefProp:
            t = [&]() {
                this->resetEvidences({});
                this->propagateBelief(strct::PropagationKind::Sum);
            };
            break;
        case Task::Gibbs:
            t = [&]() {
                this->getHiddenSetSamples(100, 20);
            };
            break;
        case Task::Gradient:
            trainSet = std::make_shared<train::TrainSet>(this->getHiddenSetSamples(500, 20));
            t = [&]() {
                this->getGradient(trainSet);
            };
            break;
        default:
            break;
        }

        auto profile = [&]() {
            float meanTime = 0.f;
            for (std::size_t k = 0; k < trials; ++k) {
                auto tic = std::chrono::high_resolution_clock::now();
                t();
                meanTime += static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tic).count()) / static_cast<float>(trials);
            }
            return meanTime;
        };
        float averageSerial = profile();
        this->setThreadPoolSize(2);
        float averagePool = profile();
        std::cout << "serial: " << averageSerial << std::endl;
        std::cout << "pool:   " << averagePool << std::endl;
        // EXPECT_LE(averagePool, averageSerial);
    };
};



TEST(Polytree, BeliefPropagation) {
    BinaryStructure model(8, false);
    model.profile(Task::BeliefProp, 50);
}
TEST(Polytree, GibbsSampling) {
    BinaryStructure model(4, false);
    model.profile(Task::Gibbs, 10);
}
TEST(Polytree, GradientComputation) {
    BinaryStructure model(8, false);
    model.profile(Task::Gradient, 30);
}



TEST(LoopyTree, BeliefPropagation) {
    BinaryStructure model(8, true);
    model.profile(Task::BeliefProp, 50);
}
TEST(LoopyTree, GibbsSampling) {
    BinaryStructure model(4, true);
    model.profile(Task::Gibbs, 10);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
