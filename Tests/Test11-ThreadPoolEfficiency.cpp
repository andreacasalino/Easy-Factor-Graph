#include <gtest/gtest.h>
#include <ModelTest.h>
#include <model/RandomField.h>
#include <chrono>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

class GraphTest
    : virtual public model::RandomField {
protected:
    enum Task { BeliefProp, Gibbs, Gradient };
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
            trainSet = std::make_shared<train::TrainSet>(this->getHiddenSetSamples(1500, 100));
            t = [&]() {
                this->getGradient(trainSet);
            };
            break;
        default:
            break;
        }

        std::vector<float> averages;
        averages.reserve(4);
        for (std::size_t th = 0; th < 4; ++th) {
            this->setThreadPoolSize(th + 1);
            averages.push_back(0.f);
            for (std::size_t k = 0; k < trials; ++k) {
                auto tic = std::chrono::high_resolution_clock::now();
                t();
                averages.back() +=  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tic).count() / static_cast<float>(trials);
            }
        }
        // check that the averages are monotonically decreasing
        for (std::size_t k = 1; k < averages.size(); ++k) {
            EXPECT_LE(averages[k], averages[k - 1]);
        }
    };
};

class ComplexPolyTree 
    : public test::ModelTest<RandomField>
    , public GraphTest {
public:
    ComplexPolyTree() = default;
protected:
    std::string getName() const final { return "graph_3.xml"; };
    std::string getFolder() const final { return "Sample06-Learning-A/"; };
};

TEST_F(ComplexPolyTree, BeliefPropagation) {
    this->profile(Task::BeliefProp, 100);
}
TEST_F(ComplexPolyTree, GibbsSampling) {
    this->profile(Task::Gibbs, 10);
}
TEST_F(ComplexPolyTree, GradientComputation) {
    this->profile(Task::Gradient, 20);
}



class ComplexLoopy
    : public test::ModelTest<RandomField>
    , public GraphTest {
public:
    ComplexLoopy() = default;
protected:
    std::string getName() const final { return "graph_4.xml"; };
};

TEST_F(ComplexLoopy, BeliefPropagation) {
    this->profile(Task::BeliefProp, 100);
}
TEST_F(ComplexLoopy, GibbsSampling) {
    this->profile(Task::Gibbs, 10);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
