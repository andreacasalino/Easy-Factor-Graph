/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::distribution;
using namespace EFG::categoric;
using namespace EFG::strct;
using namespace EFG::io;

// just a bunch of utilities needed by the sample
#include <Frequencies.h>
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

float compute_images_sum(const Distribution &distribution);

int main() {
  {
    SampleSection section("Joint distribution of a subgroup of variables ",
                          "4.8.1");

    Graph graph;

    // build the model
    VariablePtr A = make_variable(2, "A");
    VariablePtr B = make_variable(2, "B");
    VariablePtr C = make_variable(2, "C");
    VariablePtr D = make_variable(2, "D");
    float alfa = 0.5f, beta = 1.5f;
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG}, alfa));
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{Group{VariablesSoup{B, C}}, USE_SIMPLE_CORRELATION_TAG}, beta));
    graph.addConstFactor(std::make_shared<FactorExponential>(
        Factor{Group{VariablesSoup{C, D}}, USE_SIMPLE_CORRELATION_TAG}, 1.f));

    // get the joint marginal probabilities of group ABC
    cout << "P(A,B,C)" << endl;
    cout << "theoretical " << endl;
    cout << make_distribution({expf(alfa) * expf(beta), expf(alfa), 1.f,
                               expf(beta), expf(beta), 1.f, expf(alfa),
                               expf(alfa) * expf(beta)})
         << endl;
    cout << graph
                .getJointMarginalDistribution(
                    std::unordered_set<std::string>{"A", "B", "C"})
                ->getProbabilities()
         << endl
         << endl;

    // get the joint marginal probabilities of group AB
    cout << "P(A,B)" << endl;
    cout << "theoretical " << endl;
    cout << make_distribution({expf(alfa), 1.f, 1.f, expf(alfa)}) << endl;
    cout << graph
                .getJointMarginalDistribution(
                    std::unordered_set<std::string>{"A", "B"})
                ->getProbabilities()
         << endl
         << endl;
  }

  {
    SampleSection section(
        "Joint distribution of a subgroup of variables inside a complex model ",
        "4.8.2");

    Graph graph;
    xml::Importer::importFromFile(graph,
                                  SAMPLE_FOLDER + std::string{"graph.xml"});
    // set the evidences
    graph.setEvidence("X1", 0);
    graph.setEvidence("X2", 0);

    auto hidden_set = graph.getHiddenVariables();
    VariablesSoup hidden_soup =
        VariablesSoup{hidden_set.begin(), hidden_set.end()};

    // produce a list of samples for the hidden variables, conditioned by
    // the observed values for the other ones

    auto samples = graph.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{500, 50});
    {
      // compute the marginal probabilities of the following two
      // combinations (values refer to variables in the subgraph, i.e. A1,2,3,4)
      vector<Combination> comb_raw = {Combination{vector<size_t>{0, 0, 0, 0}},
                                      Combination{vector<size_t>{1, 1, 0, 0}}};
      auto marginal_A_1234 = graph.getJointMarginalDistribution(
          std::unordered_set<std::string>{"A1", "A2", "A3", "A4"});

      float images_sum = compute_images_sum(*marginal_A_1234);

      cout << endl << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(
                  comb_raw.front(), marginal_A_1234->getGroup().getVariables(),
                  samples, hidden_soup)
           << endl;
      cout << marginal_A_1234->evaluate(comb_raw.front()) / images_sum << endl;

      cout << endl << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(
                  comb_raw.back(), marginal_A_1234->getGroup().getVariables(),
                  samples, hidden_soup)
           << endl;
      cout << marginal_A_1234->evaluate(comb_raw.back()) / images_sum << endl;
    }

    {
      // compute the marginal probabilities of the following two
      // combinations (values refer to variables in the subgraph, i.e. B1,2,3)
      vector<Combination> comb_raw = {Combination(vector<size_t>{0, 0, 0}),
                                      Combination(vector<size_t>{1, 1, 0})};
      auto marginal_B_123 = graph.getJointMarginalDistribution(
          std::unordered_set<std::string>{"B1", "B2", "B3"});

      float images_sum = compute_images_sum(*marginal_B_123);

      cout << endl << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(comb_raw.front(),
                                      marginal_B_123->getGroup().getVariables(),
                                      samples, hidden_soup)
           << endl;
      cout << marginal_B_123->evaluate(comb_raw.front()) / images_sum << endl;

      cout << endl << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(comb_raw.back(),
                                      marginal_B_123->getGroup().getVariables(),
                                      samples, hidden_soup)
           << endl;
      cout << marginal_B_123->evaluate(comb_raw.back()) / images_sum << endl;
    }

    // set different evidences
    graph.setEvidence("X1", 1);
    graph.setEvidence("X2", 1);
    // produce a list of samples for the hidden variables, conditioned by
    // the novel evidences
    samples = graph.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{500, 50});
    {
      // compute the marginal probabilities of the following two
      // combinations (values refer to variables in the subgraph, i.e. A1,2,3,4)
      vector<Combination> comb_raw = {Combination(vector<size_t>{0, 0, 0, 0}),
                                      Combination(vector<size_t>{1, 1, 0, 0})};
      auto marginal_A_1234 = graph.getJointMarginalDistribution(
          std::unordered_set<std::string>{"A1", "A2", "A3", "A4"});

      float images_sum = compute_images_sum(*marginal_A_1234);

      cout << endl << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(
                  comb_raw.front(), marginal_A_1234->getGroup().getVariables(),
                  samples, hidden_soup)
           << endl;
      cout << marginal_A_1234->evaluate(comb_raw.front()) / images_sum << endl;

      cout << endl << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1)" << endl;
      cout << "empirical" << endl;
      cout << getEmpiricalProbability(
                  comb_raw.back(), marginal_A_1234->getGroup().getVariables(),
                  samples, hidden_soup)
           << endl;
      cout << marginal_A_1234->evaluate(comb_raw.back()) / images_sum << endl;
    }
  }

  return EXIT_SUCCESS;
}

float compute_images_sum(const Distribution &distribution) {
  const auto &evaluator = distribution.getEvaluator();
  float result = 0;
  for (const auto &[comb, val] : distribution.getCombinationsMap()) {
    result += evaluator.evaluate(val);
  }
  return result;
}
