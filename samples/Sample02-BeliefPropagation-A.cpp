/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;

// just a bunch of utilities needed by the sample
#include <Frequencies.h>
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Graph with a single potential", "4.2.1", [] {
    // create a simple graph with two nodes
    Graph graph;

    float teta = 1.5f;
    FactorExponential factor(
        Factor{VariablesSoup{make_variable(2, "A"), make_variable(2, "B")},
               Factor::SimplyCorrelatedTag{}},
        teta);
    graph.copyConstFactor(factor);

    // make a new belief propagation setting B=0 as observation
    graph.setEvidence("B", 0);

    // compute the marginal probabilities
    cout << "P(A|B=0)" << endl;
    cout << make_distribution({expf(teta), 1.f}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("A") << "  computed values" << endl
         << endl;

    // make a new belief propagation setting B1=1 as observation
    graph.setEvidence("B", 1);

    cout << "P(A|B=1)" << endl;
    cout << make_distribution({1.f, expf(teta)}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution("A") << "  computed values" << endl
         << endl;
  });

  SAMPLE_SECTION("Graph with two potentials and 3 variables", "4.2.2", [] {
    Graph graph;

    auto A = make_variable(2, "A");
    auto B = make_variable(2, "B");
    auto C = make_variable(2, "C");

    float alfa = 0.5f, beta = 1.f;
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{B, C}, Factor::SimplyCorrelatedTag{}}, alfa));
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, B}, Factor::SimplyCorrelatedTag{}}, beta));

    // make a new belief propagation setting C=1 as observation
    graph.setEvidence(C, 1);

    // compute the marginals of A,B and then compare results with the
    // theoretical ones, see documentation
    cout << "P(B|C=1)\n";
    cout << make_distribution({1.f, expf(alfa)}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution(B) << "  computed values" << endl
         << endl;

    cout << "P(A|C=1)\n";
    cout << make_distribution(
                {expf(alfa) + expf(beta), 1.f + expf(alfa) * expf(beta)})
         << "  theoretical values" << endl;
    cout << graph.getMarginalDistribution(A) << "  computed values" << endl
         << endl;

    // make a new belief propagation setting B=1 as unique observation
    graph.removeAllEvidences();
    graph.setEvidence(B, 1);

    cout << "P(A|B=1)\n";
    cout << make_distribution({1.f, expf(beta)}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution(A) << "  computed values" << endl
         << endl;

    cout << "P(C|B=1)\n";
    cout << make_distribution({1.f, expf(alfa)}) << "  theoretical values"
         << endl;
    cout << graph.getMarginalDistribution(C) << "  computed values" << endl
         << endl;
  });

  SAMPLE_SECTION("Belief degradation on a chain of variables", "4.2.3", [] {
    const std::size_t domain_size = 5;
    const float weight = 2.5f;
    for (int k = 2; k <= 10; k++) {
      const auto chain_size = k;
      // build the set of variables in the chain
      vector<VariablePtr> Y;
      Y.reserve(chain_size);
      for (size_t k = 0; k < chain_size; ++k) {
        Y.push_back(make_variable(domain_size, "Y_" + to_string(k)));
      }
      Graph graph;
      // build the correlating potentials and add it to the chain
      for (size_t k = 1; k < chain_size; ++k) {
        graph.addConstFactor(std::make_shared<FactorExponential>(
            Factor(VariablesSoup{Y[k - 1], Y[k]},
                   Factor::SimplyCorrelatedTag{}),
            weight));
      }
      // set Y_0 as an observations and compute the marginals of the last
      // variable in the chain
      graph.setEvidence(Y.front(), 0);
      cout << "chain size equal to " << chain_size << ", marginals of Y_n: ";
      cout << graph.getMarginalDistribution(Y.back()->name()) << endl;
      cout << endl;
    }
  });

  return EXIT_SUCCESS;
}
