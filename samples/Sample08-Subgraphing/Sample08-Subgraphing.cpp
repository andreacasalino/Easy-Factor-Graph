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

template <typename Transform, std::size_t N>
float compute_images_sum(const FactorT<N, Transform> &distribution) {
  float res{0};
  distribution.template forEachCombination<true>(
      [&res](const auto &_, auto val) { res += val; });
  return res;
}

int main() {
  SAMPLE_SECTION("Joint distribution of a subgroup of variables ", "4.8.1", [] {
    // build the model
    ModelBuilder builder;

    auto A = builder.make_variable(2);
    auto B = builder.make_variable(2);
    auto C = builder.make_variable(2);
    auto D = builder.make_variable(2);

    float alfa = 0.5f, beta = 1.5f;

    auto factor_alfa = FactorExponential<2>::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    factor_alfa.trsfm.setWeight(alfa);
    builder.add_binary_factor(std::move(factor_alfa), A, B);

    auto factor_beta = FactorExponential<2>::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    factor_beta.trsfm.setWeight(beta);
    builder.add_binary_factor(std::move(factor_alfa), B, C);

    builder.add_binary_factor(FactorExponential<2>::from_sparse_domain_gen(
                                  {2, 2}, SimplyCorrelatedDomainGen<2>{2}),
                              C, D);

    Graph graph{ModelBuilder::build(std::move(builder))};

    std::vector<float> prob;

    // get the joint marginal probabilities of group ABC
    cout << "P(A,B,C)" << endl;
    cout << make_distribution({expf(alfa) * expf(beta), expf(alfa), 1.f,
                               expf(beta), expf(beta), 1.f, expf(alfa),
                               expf(alfa) * expf(beta)})
         << "  theoretical values" << endl;
    graph.getJointMarginalDistributionProbabilities(prob, A, B, C);
    cout << prob << "  computed values" << endl << endl;

    // get the joint marginal probabilities of group AB
    cout << "P(A,B)" << endl;
    cout << make_distribution({expf(alfa), 1.f, 1.f, expf(alfa)})
         << "  theoretical values" << endl;
    graph.getJointMarginalDistributionProbabilities(prob, {A, B});
    cout << prob << "  computed values" << endl << endl;
  });

  SAMPLE_SECTION(
      "Joint distribution of a subgroup of variables inside a complex model ",
      "4.8.2", [] {
        auto path = std::filesystem::path{SAMPLE_FOLDER} / "graph.json";
        Graph graph{std::move(from_file(path))};

        std::vector<float> prob;

        auto X1 = graph.getStructure().named_vars_table.at("X1");
        auto X2 = graph.getStructure().named_vars_table.at("X2");
        auto A1 = graph.getStructure().named_vars_table.at("A1");
        auto A2 = graph.getStructure().named_vars_table.at("A2");
        auto A3 = graph.getStructure().named_vars_table.at("A3");
        auto A4 = graph.getStructure().named_vars_table.at("A4");
        auto B1 = graph.getStructure().named_vars_table.at("B1");
        auto B2 = graph.getStructure().named_vars_table.at("B2");
        auto B3 = graph.getStructure().named_vars_table.at("B3");

        // set the evidences
        graph.setEvidences(Evidence{X1, 0}, Evidence{X2, 0});

        // produce a list of samples for the hidden variables, conditioned by
        // the observed values for the other ones

        auto samples = graph.makeSamples(
            GibbsSampler::SamplesGenerationContext{1000, 50, 0, 1000});
        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // A1,2,3,4)
          auto marginal_A_1234 = graph.getJointMarginalDistribution(
              std::array<std::size_t, 4>{A1, A2, A3, A4});

          float images_sum = compute_images_sum(marginal_A_1234);

          cout << endl << "Prob(A1=0, A2=0, A3=0, A4=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{A1, 0}, {A2, 0}, {A3, 0}, {A4, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.get<true>({0, 0, 0, 0}) / images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(A1=1, A2=1, A3=0, A4=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{A1, 1}, {A2, 1}, {A3, 0}, {A4, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.get<true>({1, 1, 0, 0}) / images_sum
               << "  computed values" << endl;
        }

        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // B1,2,3)
          auto marginal_B_123 = graph.getJointMarginalDistribution(
              std::array<std::size_t, 3>{B1, B2, B3});

          float images_sum = compute_images_sum(marginal_B_123);

          cout << endl << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{B1, 0}, {B2, 0}, {B3, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_B_123.get<true>({0, 0, 0}) / images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{B1, 1}, {B2, 1}, {B3, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_B_123.get<true>({1, 1, 0}) / images_sum
               << "  computed values" << endl;
        }

        // set different evidences
        graph.setEvidences(Evidence{X1, 1}, Evidence{X2, 1});
        // produce a list of samples for the hidden variables, conditioned by
        // the novel evidences
        samples = graph.makeSamples(
            GibbsSampler::SamplesGenerationContext{1000, 50, 2, 1000});
        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // A1,2,3,4)
          auto marginal_A_1234 = graph.getJointMarginalDistribution(
              std::array<std::size_t, 4>{A1, A2, A3, A4});

          float images_sum = compute_images_sum(marginal_A_1234);

          cout << endl << "Prob(A1=0, A2=0, A3=0, A4=0 | X1=1,X2=1)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{A1, 0}, {A2, 0}, {A3, 0}, {A4, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.get<true>({0, 0, 0, 0}) / images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(A1=1, A2=1, A3=0, A4=0 | X1=1,X2=1)" << endl;
          cout << getEmpiricalProbabilityInsideHidden(
                      {{A1, 1}, {A2, 1}, {A3, 0}, {A4, 0}}, samples,
                      graph.getStructure())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.get<true>({1, 1, 0, 0}) / images_sum
               << "  computed values" << endl;
        }
      });

  return EXIT_SUCCESS;
}
