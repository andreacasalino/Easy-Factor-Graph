#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

#include <algorithm>
#include <math.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;
using namespace EFG::strct;
using namespace EFG::io;

namespace {
std::string make_file_path(const std::string &file_name) {
  std::stringstream stream;
  stream << SAMPLE_FOLDER << "Sample03-BeliefPropagation-B/" << file_name;
  return stream.str();
}

bool are_samples_valid(const std::vector<Combination> &samples,
                       const VariablesSet &group) {
  if (samples.empty()) {
    return false;
  }
  auto samples_it = std::find_if(samples.begin(), samples.end(),
                                 [&group](const Combination &comb) {
                                   return comb.data().size() != group.size();
                                 });
  return samples_it == samples.end();
}

// frequency of var = 1 is returned
float getFrequency1(const std::vector<Combination> &samples,
                    const VariablesSet &group, const VariablePtr &var) {
  std::size_t varPos = std::distance(group.begin(), group.find(var));
  std::size_t instances = 0;
  for (const auto &sample : samples) {
    if (1 == sample.data()[varPos]) {
      ++instances;
    }
  }
  return static_cast<float>(instances) / static_cast<float>(samples.size());
};

bool almost_equal(float probTheory0, float probTheory1, float frequency1,
                  const float threshold = 0.05f) {
  float probTheory = probTheory1 / (probTheory0 + probTheory1);
  return fabs(probTheory - frequency1) < threshold;
}
} // namespace

TEST_CASE("binary factor gibbs sampling", "[gibbs_sampling]") {
  float w = 2.f;
  FactorExponential factor(
      Factor(Group{make_variable(2, "A"), make_variable(2, "B")},
             USE_SIMPLE_CORRELATION_TAG),
      w);

  Graph model;
  model.copyConstFactor(factor);

  model.setEvidence(model.findVariable("A"), 1);
  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 50, 0});
  REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));
  CHECK(almost_equal(1.f, expf(w),
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("B"))));
}

TEST_CASE("polyTree gibbs sampling", "[gibbs_sampling]") {
  float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
  Graph model;
  xml::Importer::importFromFile(model, make_file_path("graph_1.xml"));

  auto threads = GENERATE(1, 4);

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 20, 0}, threads);
  REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));

  CHECK(almost_equal((a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)),
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("A"))));
  CHECK(almost_equal((g + e), (1 + g * e),
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("B"))));
  CHECK(almost_equal((b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)),
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("C"))));
  CHECK(almost_equal(1.f, e,
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("D"))));
}

TEST_CASE("loopy model gibbs sampling", "[gibbs_sampling]") {
  float M = expf(1.f);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);
  Graph model;
  xml::Importer::importFromFile(model, make_file_path("graph_3.xml"));

  auto threads = GENERATE(1, 4);

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 20}, threads);
  REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));
  CHECK(almost_equal(3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2),
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("D")),
                     0.06));
  CHECK(almost_equal(M_alfa, M_beta,
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("C")),
                     0.06));
  CHECK(almost_equal(M_alfa, M_beta,
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("B")),
                     0.06));
  CHECK(almost_equal(M * M_alfa + M_beta, M_alfa + M * M_beta,
                     getFrequency1(samples, model.getHiddenVariables(),
                                   model.findVariable("A")),
                     0.06));
}
