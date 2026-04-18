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

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Graph with a single potential", "4.2.1", [] {
    float teta = 1.5f;

    ModelBuilder builder;
    builder.make_variable(2);
    builder.make_variable(2);
    auto factor = FactorExponential<2>::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    factor.trsfm.setWeight(teta);
    builder.add_binary_factor(std::move(factor), 0, 1);

    Graph graph{ModelBuilder::build(std::move(builder))};
    std::vector<float> prob;

    // make a new belief propagation setting B=0 as observation
    graph.setEvidences(Evidence{1, 0});

    // compute the marginal probabilities
    cout << "P(A|B=0)" << endl;
    cout << make_distribution({expf(teta), 1.f}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, 0);
    cout << prob << "  computed values" << endl << endl;

    // make a new belief propagation setting B1=1 as observation
    graph.setEvidences(Evidence{1, 1});

    cout << "P(A|B=1)" << endl;
    cout << make_distribution({1.f, expf(teta)}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, 0);
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION("Graph with two potentials and 3 variables", "4.2.2", [] {
    float alfa = 0.5f, beta = 1.f;

    ModelBuilder builder;
    auto A_idx = builder.make_variable(2);
    auto B_idx = builder.make_variable(2);
    auto C_idx = builder.make_variable(2);
    auto factor_alfa = FactorExponential<2>::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    factor_alfa.trsfm.setWeight(alfa);
    builder.add_binary_factor(std::move(factor_alfa), B_idx, C_idx);
    auto factor_beta = FactorExponential<2>::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    factor_beta.trsfm.setWeight(beta);
    builder.add_binary_factor(std::move(factor_beta), A_idx, B_idx);

    Graph graph{ModelBuilder::build(std::move(builder))};
    std::vector<float> prob;

    // make a new belief propagation setting C=1 as observation
    graph.setEvidences(Evidence{C_idx, 1});

    // compute the marginals of A,B and then compare results with the
    // theoretical ones, see documentation
    cout << "P(B|C=1)\n";
    cout << make_distribution({1.f, expf(alfa)}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, B_idx);
    cout << prob << "  computed values" << endl << endl;

    cout << "P(A|C=1)\n";
    cout << make_distribution(
                {expf(alfa) + expf(beta), 1.f + expf(alfa) * expf(beta)})
         << "  theoretical values" << endl;
    graph.getMarginalDistribution(prob, A_idx);
    cout << prob << "  computed values" << endl << endl;

    // make a new belief propagation setting B=1 as unique observation
    graph.removeAllEvidences();
    graph.setEvidences(Evidence{B_idx, 1});

    cout << "P(A|B=1)\n";
    cout << make_distribution({1.f, expf(beta)}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, A_idx);
    cout << prob << "  computed values" << endl << endl;

    cout << "P(C|B=1)\n";
    cout << make_distribution({1.f, expf(alfa)}) << "  theoretical values"
         << endl;
    graph.getMarginalDistribution(prob, C_idx);
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION("Belief degradation on a chain of variables", "4.2.3", [] {
    const std::size_t domain_size = 5;
    const float weight = 2.5f;
    for (int k = 2; k <= 10; k++) {
      const auto chain_size = k;

      ModelBuilder builder;
      for (size_t k = 0; k < chain_size; ++k) {
        builder.make_variable(domain_size);
      }
      for (size_t k = 1; k < chain_size; ++k) {
        auto factor = FactorExponential<2>::from_sparse_domain_gen(
            {domain_size, domain_size},
            SimplyCorrelatedDomainGen<2>{domain_size});
        factor.trsfm.setWeight(weight);
        builder.add_binary_factor(std::move(factor), k - 1, k);
      }

      Graph graph{ModelBuilder::build(std::move(builder))};
      std::vector<float> prob;

      // set Y_0 as an observations and compute the marginals of the last
      // variable in the chain
      graph.setEvidences(Evidence{0, 0});
      cout << "chain size equal to " << chain_size << ", marginals of Y_n: ";
      graph.getMarginalDistribution(prob, chain_size - 1);
      cout << prob << endl;
      cout << endl;
    }
  });

  return EXIT_SUCCESS;
}
