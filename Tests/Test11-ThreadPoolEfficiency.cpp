#include <gtest/gtest.h>
#include <model/RandomField.h>
#include <io/xml/Importer.h>
#include <chrono>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

class GraphTest
    : public ::testing::Test
    , public model::RandomField {
protected:
    GraphTest() = default;

    void SetUp() override {
        io::xml::Importer::importFromXml(*this, io::FilePath(std::string(SAMPLE_FOLDER) + std::string("Sample03-BeliefPropagation-B/"), this->getFileName()));
    }

    virtual std::string getFileName() = 0;

    enum Task { BeliefProp, Gibbs, Gradient };
    void profile(const Task& task, const std::size_t trials) {
        std::function<void(void)> t;
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
            t = [&]() {
                this->getGradient();
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

class ComplexPolyTree : public GraphTest {
public:
    ComplexPolyTree() = default;
protected:
    std::string getFileName() final { return "graph_2.xml"; };
};

class ComplexLoopy : public GraphTest {
public:
    ComplexLoopy() = default;
protected:
    std::string getFileName() final { return "graph_4.xml"; };
};

#define PROFILE(MODEL) \
TEST_F(MODEL, BeliefPropagation) { \
    this->profile(Task::BeliefProp); \
} \
TEST_F(MODEL, GibbsSampling) { \
    this->profile(Task::Gibbs); \
} \
TEST_F(MODEL, GradientComputation) { \
    this->profile(Task::Gradient); \
}

PROFILE(ComplexPolyTree)

PROFILE(ComplexPolyTree)
