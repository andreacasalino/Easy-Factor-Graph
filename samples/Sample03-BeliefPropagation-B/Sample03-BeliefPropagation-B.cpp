/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::strct;

// just a bunch of utilities needed by the sample
#include <Frequencies.h>
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

std::string merge(const std::string &first, const std::string &second);

int main() {
  SAMPLE_SECTION("Simple polytree belief propagation", "4.3.1", [] {
    Graph graph;
    // import the graph from an xml file
    EFG::io::xml::Importer::importFromFile(graph,
                                           merge(SAMPLE_FOLDER, "graph_1.xml"));
    float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);

    // set E=1 as an evidence
    graph.setEvidence("E", 1);
    cout << endl << endl;
    cout << "E=1\n";

    // compute the marginals distributions of the other variables and
    // compare it
    cout << "P(A|E)\n";
    cout << make_distribution(
                {(a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e))})
         << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("A") << "  computed values" << endl
         << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({(g + e), (1 + g * e)}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("B") << "  computed values" << endl
         << endl;

    cout << "P(C|E)\n";
    cout << make_distribution(
                {(b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e))})
         << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("C") << "  computed values" << endl
         << endl;

    cout << "P(D|E)\n";
    cout << make_distribution({1.f, e}) << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("D") << "  computed values" << endl
         << endl;

    // set E=0 as an evidence and recompute the marginals
    graph.setEvidence("E", 0);
    cout << endl << endl;
    cout << "E=0\n";

    cout << "P(A|E)\n";
    cout << make_distribution(
                {(g + e) + a * (1 + g * e), a * (g + e) + (1 + g * e)})
         << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("A") << "  computed values" << endl
         << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({1 + g * e, g + e}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("B") << "  computed values" << endl
         << endl;

    cout << "P(C|E)\n";
    cout << make_distribution(
                {(g + e) + b * (1 + g * e), b * (g + e) + (1 + g * e)})
         << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("C") << "  computed values" << endl
         << endl;

    cout << "P(D|E)\n";
    cout << make_distribution({e, 1.f}) << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("D") << "  computed values" << endl
         << endl;

    // set D=1 as an evidence and recompute the marginals of the hidden
    // variables (including E)
    graph.removeAllEvidences();
    graph.setEvidence("D", 1);
    cout << endl << endl;
    cout << "D=1\n";

    cout << "P(A|D)\n";
    cout << make_distribution({a + g, 1.f + a * g}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("A") << "  computed values" << endl
         << endl;

    cout << "P(B|D)\n";
    cout << make_distribution({1.f, g}) << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("B") << "  computed values" << endl
         << endl;

    cout << "P(C|D)\n";
    cout << make_distribution({b + g, 1.f + b * g}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("C") << "  computed values" << endl
         << endl;

    cout << "P(E|D)\n";
    cout << make_distribution({1.f, e}) << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution("E") << "  computed values" << endl
         << endl;
  });

  SAMPLE_SECTION("Complex polytree belief propagation", "4.3.2", [] {
    Graph politree;
    // import the graph an existing xml file
    EFG::io::xml::Importer::importFromFile(politree,
                                           merge(SAMPLE_FOLDER, "graph_2.xml"));

    GibbsSampler::SamplesGenerationContext context_for_saples_generation =
        GibbsSampler::SamplesGenerationContext{
            1500, // number of desired samples
            50, 0};
    // use internal thread pool to fasten various computations below
    std::size_t threads = 3;

    // set v1,v2,v3 as observations and use Gibbs sampling
    // to produce samples for the joint conditioned (to the observations)
    // distribution of the hidden variables
    politree.setEvidence("v1", 1);
    politree.setEvidence("v2", 1);
    politree.setEvidence("v3", 1);
    auto samples = politree.makeSamples(context_for_saples_generation, threads);

    auto hidden_set = politree.getHiddenVariables();

    // compare the computed marginals with the ones coming from the samples
    // obtained by the Gibbs sampler
    cout << "P(v10 | Observations): \n";
    cout << getEmpiricalMarginals(politree.findVariable("v10"), samples,
                                  politree.getAllVariables())
         << "  empirical values from Gibbs sampling" << endl;
    cout << politree.getMarginalDistribution("v10") << "  computed values"
         << endl
         << endl;

    cout << "P(v11 | Observations): \n";
    cout << getEmpiricalMarginals(politree.findVariable("v11"), samples,
                                  politree.getAllVariables())
         << "  empirical values from Gibbs sampling" << endl;
    cout << politree.getMarginalDistribution("v11") << "  computed values"
         << endl
         << endl;

    cout << "P(v12 | Observations): \n";
    cout << getEmpiricalMarginals(politree.findVariable("v12"), samples,
                                  politree.getAllVariables())
         << "  empirical values from Gibbs sampling" << endl;
    cout << politree.getMarginalDistribution("v12") << "  computed values"
         << endl
         << endl;
  });

  SAMPLE_SECTION("Simple loopy model belief propagation", "4.3.3", [] {
    Graph loop;
    // import the graph an existing xml file
    EFG::io::xml::Importer::importFromFile(loop,
                                           merge(SAMPLE_FOLDER, "graph_3.xml"));

    // set the observation
    loop.setEvidence("E", 1);
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
    cout << loop.getMarginalDistribution("D") << "  computed values" << endl
         << endl;

    cout << "P(C|E)\n";
    cout << make_distribution({M_alfa, M_beta}) << "  theoretical values"
         << endl;
    cout << loop.getMarginalDistribution("C") << "  computed values" << endl
         << endl;

    cout << "P(B|E)\n";
    cout << make_distribution({M_alfa, M_beta}) << "  theoretical values"
         << endl;
    cout << loop.getMarginalDistribution("B") << "  computed values" << endl
         << endl;

    cout << "P(A|E)\n";
    cout << make_distribution({M * M_alfa + M_beta, M_alfa + M * M_beta})
         << "  theoretical values" << endl;
    cout << loop.getMarginalDistribution("A") << "  computed values" << endl
         << endl;
  });

  SAMPLE_SECTION("Complex loopy model belief propagation", "4.3.4", [] {
    Graph loop;
    // import the graph an existing xml file
    EFG::io::xml::Importer::importFromFile(loop,
                                           merge(SAMPLE_FOLDER, "graph_4.xml"));

    GibbsSampler::SamplesGenerationContext context_for_saples_generation =
        GibbsSampler::SamplesGenerationContext{500, // number of desired samples
                                               50, 0};
    // use internal thread pool to fasten various computations below
    std::size_t threads = 3;

    // set v1=1 as an evidence and use a Gibbs sampler
    // to produce samples for the joint conditioned (to the observations)
    // distribution of the hidden variables
    loop.setEvidence("v1", 1);
    auto samples = loop.makeSamples(context_for_saples_generation, threads);

    auto hidden_set = loop.getHiddenVariables();

    // compare the computed marginals with the ones coming from the samples
    // obtained by the Gibbs sampler
    cout << "P(v8 | Observations): \n";
    cout << getEmpiricalMarginals(loop.findVariable("v8"), samples,
                                  loop.getAllVariables())
         << "  empirical values from Gibbs sampling" << endl;
    cout << loop.getMarginalDistribution("v8", threads) << "  computed values"
         << endl
         << endl; // use the interal thread pool to also fasten the compuations
                  // of the amrginal distribution
  });

  return EXIT_SUCCESS;
}

std::string merge(const std::string &first, const std::string &second) {
  std::stringstream stream;
  stream << first << second;
  return stream.str();
}
