/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::strct;
using namespace EFG::io;

// just a bunch of utilities needed by the sample
#include <Frequencies.h>
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

float compute_images_sum(const Function &distribution);

int main() {
  SAMPLE_SECTION("Joint distribution of a subgroup of variables ", "4.8.1", [] {
    Graph graph;

    // build the model
    VariablePtr A = make_variable(2, "A");
    VariablePtr B = make_variable(2, "B");
    VariablePtr C = make_variable(2, "C");
    VariablePtr D = make_variable(2, "D");
    float alfa = 0.5f, beta = 1.5f;
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, B}, Factor::SimplyCorrelatedTag{}}, alfa));
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{B, C}, Factor::SimplyCorrelatedTag{}}, beta));
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{C, D}, Factor::SimplyCorrelatedTag{}}, 1.f));

    // get the joint marginal probabilities of group ABC
    cout << "P(A,B,C)" << endl;
    cout << make_distribution({expf(alfa) * expf(beta), expf(alfa), 1.f,
                               expf(beta), expf(beta), 1.f, expf(alfa),
                               expf(alfa) * expf(beta)})
         << "  theoretical values" << endl;
    cout << graph.getJointMarginalDistribution({"A", "B", "C"})
                .getProbabilities()
         << "  computed values" << endl
         << endl;

    // get the joint marginal probabilities of group AB
    cout << "P(A,B)" << endl;
    cout << make_distribution({expf(alfa), 1.f, 1.f, expf(alfa)})
         << "  theoretical values" << endl;
    cout << graph.getJointMarginalDistribution({"A", "B"}).getProbabilities()
         << "  computed values" << endl
         << endl;
  });

  SAMPLE_SECTION(
      "Joint distribution of a subgroup of variables inside a complex model ",
      "4.8.2", [] {
        Graph graph;
        xml::Importer::importFromFile(graph,
                                      SAMPLE_FOLDER + std::string{"graph.xml"});
        // set the evidences
        graph.setEvidence("X1", 0);
        graph.setEvidence("X2", 0);

        // produce a list of samples for the hidden variables, conditioned by
        // the observed values for the other ones

        auto samples = graph.makeSamples(
            GibbsSampler::SamplesGenerationContext{1000, 50, 0, 1000});
        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // A1,2,3,4)
          vector<vector<size_t>> comb_raw = {{0, 0, 0, 0}, {1, 1, 0, 0}};

          auto marginal_A_1234 =
              graph.getJointMarginalDistribution({"A1", "A2", "A3", "A4"});

          float images_sum = compute_images_sum(marginal_A_1234.function());

          cout << endl << "Prob(A1=0, A2=0, A3=0, A4=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.front(),
                      marginal_A_1234.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.function().findTransformed(comb_raw.front()) /
                      images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(A1=1, A2=1, A3=0, A4=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.back(),
                      marginal_A_1234.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.function().findTransformed(comb_raw.back()) /
                      images_sum
               << "  computed values" << endl;
        }

        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // B1,2,3)
          vector<vector<size_t>> comb_raw = {{0, 0, 0}, {1, 1, 0}};
          auto marginal_B_123 =
              graph.getJointMarginalDistribution({"B1", "B2", "B3"});

          float images_sum = compute_images_sum(marginal_B_123.function());

          cout << endl << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.front(),
                      marginal_B_123.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_B_123.function().findTransformed(comb_raw.front()) /
                      images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.back(),
                      marginal_B_123.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_B_123.function().findTransformed(comb_raw.back()) /
                      images_sum
               << "  computed values" << endl;
        }

        // set different evidences
        graph.setEvidence("X1", 1);
        graph.setEvidence("X2", 1);
        // produce a list of samples for the hidden variables, conditioned by
        // the novel evidences
        samples = graph.makeSamples(
            GibbsSampler::SamplesGenerationContext{1000, 50, 2, 1000});
        {
          // compute the marginal probabilities of the following two
          // combinations (values refer to variables in the subgraph, i.e.
          // A1,2,3,4)
          vector<vector<size_t>> comb_raw = {{0, 0, 0, 0}, {1, 1, 0, 0}};
          auto marginal_A_1234 =
              graph.getJointMarginalDistribution({"A1", "A2", "A3", "A4"});

          float images_sum = compute_images_sum(marginal_A_1234.function());

          cout << endl << "Prob(A1=0, A2=0, A3=0, A4=0 | X1=1,X2=1)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.front(),
                      marginal_A_1234.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.function().findTransformed(comb_raw.front()) /
                      images_sum
               << "  computed values" << endl;

          cout << endl << "Prob(A1=1, A2=1, A3=0, A4=0 | X1=1,X2=1)" << endl;
          cout << getEmpiricalProbability(
                      comb_raw.back(),
                      marginal_A_1234.function().vars().getVariables(), samples,
                      graph.getAllVariables())
               << "  empirical values from Gibbs sampling" << endl;
          cout << marginal_A_1234.function().findTransformed(comb_raw.back()) /
                      images_sum
               << "  computed values" << endl;
        }
      });

  return EXIT_SUCCESS;
}

float compute_images_sum(const Function &distribution) {
  float result = 0;
  distribution.forEachNonNullCombination<true>(
      [&result](const auto &, float img) { result += img; });
  return result;
}
