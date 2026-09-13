/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::structure;

// just a bunch of utilities needed by the sample
#include <Frequencies.h>
#include <Printing.h>
#include <SampleSection.h>

#include <filesystem>
#include <iostream>
#include <string_view>
using namespace std;

std::filesystem::path make_path(std::string_view filename);

int main() {
  SAMPLE_SECTION("Simple polytree belief propagation", "4.3.1", [] {
    // import the graph from file
    Graph graph{std::move(from_file(make_path("graph_1.json")))};
    float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
    std::vector<float> prob;
    const auto &vars_names = graph.getStructure().named_vars_table;

    // set E=1 as an evidence
    graph.setEvidences(Evidence{vars_names.at("E"), 1});
    cout << endl << endl;
    cout << "E=1\n";

    // compute the marginals distributions of the other variables and
    // compare it
    cout << "P(A|E)\n";
    cout << make_distribution(
                {(a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e))})
         << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("A"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({(g + e), (1 + g * e)}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, vars_names.at("B"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(C|E)\n";
    cout << make_distribution(
                {(b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e))})
         << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("C"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(D|E)\n";
    cout << make_distribution({1.f, e}) << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("D"));
    cout << prob << "  computed values" << endl << endl;

    // set E=0 as an evidence and recompute the marginals
    graph.setEvidences(Evidence{vars_names.at("E"), 0});
    cout << endl << endl;
    cout << "E=0\n";

    cout << "P(A|E)\n";
    cout << make_distribution(
                {(g + e) + a * (1 + g * e), a * (g + e) + (1 + g * e)})
         << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("A"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({1 + g * e, g + e}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, vars_names.at("B"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(C|E)\n";
    cout << make_distribution(
                {(g + e) + b * (1 + g * e), b * (g + e) + (1 + g * e)})
         << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("C"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(D|E)\n";
    cout << make_distribution({e, 1.f}) << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("D"));
    cout << prob << "  computed values" << endl << endl;

    // set D=1 as an evidence and recompute the marginals of the hidden
    // variables (including E)
    graph.removeAllEvidences();
    graph.setEvidences(Evidence{vars_names.at("D"), 1});
    cout << endl << endl;
    cout << "D=1\n";

    cout << "P(A|D)\n";
    cout << make_distribution({a + g, 1.f + a * g}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, vars_names.at("A"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(B|D)\n";
    cout << make_distribution({1.f, g}) << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("B"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(C|D)\n";
    cout << make_distribution({b + g, 1.f + b * g}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, vars_names.at("C"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(E|D)\n";
    cout << make_distribution({1.f, e}) << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, vars_names.at("E"));
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION("Complex polytree belief propagation", "4.3.2", [] {
    // import the graph from file
    Graph politree{std::move(from_file(make_path("graph_2.json")))};
    std::vector<float> prob;
    const auto &vars_names = politree.getStructure().named_vars_table;

    GibbsSampler::SamplesGenerationContext context_for_saples_generation =
        GibbsSampler::SamplesGenerationContext{
            1500, // number of desired samples
            50, 0};

    // set v1,v2,v3 as observations and use Gibbs sampling
    // to produce samples for the joint conditioned (to the observations)
    // distribution of the hidden variables
    politree.setEvidences(Evidence{vars_names.at("v1"), 1},
                          Evidence{vars_names.at("v2"), 1},
                          Evidence{vars_names.at("v3"), 1});
    std::optional<EFG::misc::Samples> samples;
    {
      // use internal thread pool to fasten various computations below
      std::size_t threads = 4;
      politree.setWorkersPoolSize(threads);
      auto activated_threads_guard =
          politree.activatePool(); // scoped activation
      samples.emplace(politree.makeSamples(context_for_saples_generation));
    }

    std::pair<std::size_t, VarStateSize> idx_and_size;
    auto get_var_and_size_ = [&](std::string var_name) {
      auto var_idx = vars_names.at(var_name);
      auto var_size = politree.getStructure().nodes[var_idx].var_size;
      idx_and_size = std::make_pair(var_idx, var_size);
    };

    // compare the computed marginals with the ones coming from the samples
    // obtained by the Gibbs sampler
    cout << "P(v10 | Observations): \n";
    get_var_and_size_("v10");
    cout << getEmpiricalMarginals(idx_and_size.first, idx_and_size.second,
                                  samples.value())
         << "  empirical values from Gibbs sampling" << endl;
    politree.getMarginalDistribution(prob, idx_and_size.first);
    cout << prob << "  computed values" << endl << endl;

    cout << "P(v11 | Observations): \n";
    get_var_and_size_("v11");
    cout << getEmpiricalMarginals(idx_and_size.first, idx_and_size.second,
                                  samples.value())
         << "  empirical values from Gibbs sampling" << endl;
    politree.getMarginalDistribution(prob, idx_and_size.first);
    cout << prob << "  computed values" << endl << endl;

    cout << "P(v12 | Observations): \n";
    get_var_and_size_("v12");
    cout << getEmpiricalMarginals(idx_and_size.first, idx_and_size.second,
                                  samples.value())
         << "  empirical values from Gibbs sampling" << endl;
    politree.getMarginalDistribution(prob, idx_and_size.first);
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION("Simple loopy model belief propagation", "4.3.3", [] {
    // import the graph from file
    Graph loop{std::move(from_file(make_path("graph_3.json")))};
    std::vector<float> prob;
    const auto &vars_names = loop.getStructure().named_vars_table;

    // set the observation
    loop.setEvidences(Evidence{vars_names.at("E"), 1});
    cout << endl << endl;
    cout << "E=1\n";

    // compute the marginals distributions of the hidden variables and
    // compare it
    float M = expf(1.f);
    float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
    float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);

    cout << "P(D|E)\n";
    cout << make_distribution(
                {3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2)})
         << "  theoretical values" << endl;
    loop.getMarginalDistribution(prob, vars_names.at("D"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(C|E)\n";
    cout << make_distribution({M_alfa, M_beta}) << "  theoretical values"
         << endl;
    loop.getMarginalDistribution(prob, vars_names.at("C"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({M_alfa, M_beta}) << "  theoretical values"
         << endl;
    loop.getMarginalDistribution(prob, vars_names.at("B"));
    cout << prob << "  computed values" << endl << endl;

    cout << "P(A|E)\n";
    cout << make_distribution({M * M_alfa + M_beta, M_alfa + M * M_beta})
         << "  theoretical values" << endl;
    loop.getMarginalDistribution(prob, vars_names.at("A"));
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION("Complex loopy model belief propagation", "4.3.4", [] {
    // import the graph from file
    Graph loop{std::move(from_file(make_path("graph_4.json")))};
    std::vector<float> prob;
    const auto &vars_names = loop.getStructure().named_vars_table;

    GibbsSampler::SamplesGenerationContext context_for_saples_generation =
        GibbsSampler::SamplesGenerationContext{500, // number of desired samples
                                               50, 0};
    // use internal thread pool to fasten various computations below
    std::size_t threads = 3;

    // set v1=1 as an evidence and use a Gibbs sampler
    // to produce samples for the joint conditioned (to the observations)
    // distribution of the hidden variables
    loop.setEvidences(Evidence{vars_names.at("v1"), 1});
    loop.setWorkersPoolSize(threads);
    auto activated_threads_guard = loop.activatePool(); // scoped activation
    auto samples = loop.makeSamples(context_for_saples_generation);

    auto var_idx = vars_names.at("v8");
    auto var_size = loop.getStructure().nodes[var_idx].var_size;

    // compare the computed marginals with the ones coming from the samples
    // obtained by the Gibbs sampler
    cout << "P(v8 | Observations): \n";
    cout << getEmpiricalMarginals(var_idx, var_size, samples)
         << "  empirical values from Gibbs sampling" << endl;
    loop.getMarginalDistribution(prob, var_idx);
    cout << prob << "  computed values" << endl
         << endl; // use the interal thread pool to also fasten the
                  // compuations of the amrginal distribution
  });

  return EXIT_SUCCESS;
}

std::filesystem::path make_path(std::string_view filename) {
  return std::filesystem::path{SAMPLE_FOLDER} / filename;
}
