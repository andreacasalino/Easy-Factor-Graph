#include <gtest/gtest.h>
#include <ModelTest.h>
#include <model/Graph.h>
#include <math.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

// frequency of var = 1 is returned
float getFrequency1(const std::vector<Combination>& samples, const std::set<VariablePtr>& group, const VariablePtr& var) {
    std::size_t varPos = std::distance(group.begin(), group.find(var));
    for(auto it = samples.begin(); it!=samples.end(); ++it) {
        EXPECT_EQ(it->size(), group.size());
    }
    std::size_t instances = 0;
    for(auto it = samples.begin(); it!=samples.end(); ++it) {
        if(1 == it->data()[varPos]) {
            ++instances;
        }
    }
    return static_cast<float>(instances) / static_cast<float>(samples.size());
};

void compare(float probTheory0, float probTheory1, float frequency1, const float threshold = 0.05f) {
    float probTheory = probTheory1 / (probTheory0 + probTheory1);/*
    std::cout << "prob theory " << probTheory << "   ;   prob sampler " << frequency1 << std::endl;*/
    EXPECT_LE(fabs(probTheory - frequency1) , threshold);
}

TEST(GibbsSampling, trivialBinary) {
    float w = 2.f;
    Graph model;
    model.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({ makeVariable(2, "A"), makeVariable(2, "B") }, true), w));
    
    model.resetEvidences({ {"A", 1} });
    auto samples = model.getHiddenSetSamples(500, 50);
    compare(1.f, expf(w), getFrequency1(samples, model.getHiddenVariables(), model.findVariable("B")));
}

TEST(GibbsSampling, polyTree) {
    float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
    Graph model;
    io::xml::Importer::importFromXml(model, test::getModelPath("graph_1.xml"));

    // E=1
    model.resetEvidences({ {"E", 1} });
    auto samples = model.getHiddenSetSamples(500, 20);
    compare((a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)), getFrequency1(samples, model.getHiddenVariables(), model.findVariable("A")));
    compare((g + e) ,(1 + g * e), getFrequency1(samples, model.getHiddenVariables(), model.findVariable("B")));
    compare((b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)), getFrequency1(samples, model.getHiddenVariables(), model.findVariable("C")));
    compare(1.f, e , getFrequency1(samples, model.getHiddenVariables(), model.findVariable("D")));
}

TEST(GibbsSampling, loopy) {
    float M = expf(1.f);
    float M_alfa = powf(M, 3) + M + 2.f*powf(M, 2);
    float M_beta = powf(M, 4) + 2.f*M + powf(M, 2);
    Graph model;
    io::xml::Importer::importFromXml(model, test::getModelPath("graph_3.xml"));

    // E=1
    model.resetEvidences({ {"E", 1} });
    auto samples = model.getHiddenSetSamples(500, 20);
    compare(3.f * M + powf(M,3), powf(M,4) + 3.f * powf(M,2), getFrequency1(samples, model.getHiddenVariables(), model.findVariable("D")), 0.06);
    compare(M_alfa, M_beta, getFrequency1(samples, model.getHiddenVariables(), model.findVariable("C")), 0.06);
    compare(M_alfa, M_beta, getFrequency1(samples, model.getHiddenVariables(), model.findVariable("B")), 0.06);
    compare(M * M_alfa + M_beta, M_alfa + M * M_beta , getFrequency1(samples, model.getHiddenVariables(), model.findVariable("A")), 0.06);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
