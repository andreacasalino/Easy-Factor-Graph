/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <model/RandomField.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/const/FactorExponential.h>
#include <io/xml/Importer.h>
#include <Presenter.h>
#include <Frequencies.h>
#include <print/ProbabilityDistributionPrint.h>
#include <Error.h>
#include <math.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

int main() {
    EFG::sample::samplePart([]() {
        VariablePtr A = makeVariable(2, "A");
        VariablePtr B = makeVariable(2, "B");
        VariablePtr C = makeVariable(2, "C");
        VariablePtr D = makeVariable(2, "D");
        float alfa = 0.5f, beta = 1.5f;        
        //build the chain
        model::RandomField graph;
        graph.Insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(Group(A, B), true), alfa));
        graph.Insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(Group(B, C), true), beta));
        graph.Insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(Group(C, D), true), 1.f));
        
        // get the join marginal probabilities of group ABC
        cout << "P(A,B,C)" << endl;
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({expf(alfa) * expf(beta),
                                               expf(alfa),
                                               1.f,
                                               expf(beta),
                                               expf(beta),
                                               1.f,
                                               expf(alfa),
                                               expf(alfa) * expf(beta)}) << endl;
        cout << graph.getJointMarginalDistribution({"A", "B", "C"}).getProbabilities() << endl << endl;

        // get the join marginal probabilities of group AB
        cout << "P(A,B)" << endl;
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({ expf(alfa), 1.f, 1.f, expf(alfa) }) << endl;
        cout << graph.getJointMarginalDistribution({ "A", "B" }).getProbabilities() << endl << endl;
    }, "Subgraph from simple chain");

    EFG::sample::samplePart([]() {        
        //build the graph described in 'Sample 08: Sub-graphing'
        model::RandomField graph;
        io::xml::Importer::importFromXml(graph, SAMPLE_FOLDER, "graph.xml");
        // set the evidences
        graph.resetEvidences(map<string, size_t>{ {"X1", 0}, { "X2",0 }});
        
        // produce a list of samples for the hidden variables, conditioned by the observed values for the other ones
        auto sample = graph.getHiddenSetSamples(500, 200);
        {
        	// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
            auto marginal_A12434 = graph.getJointMarginalDistribution({ "A1" , "A2" ,"A3" ,"A4" });
        	vector<Combination> comb_raw = { Combination(vector<size_t>{0, 0, 0, 0}) , Combination(vector<size_t>{1,1,0,0}) };

            cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0)" << endl;
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.front(), marginal_A12434.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_A12434.find(comb_raw.front()) << endl;

            cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0)";
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.back(), marginal_A12434.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_A12434.find(comb_raw.back()) << endl;
        }
        {
            // compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
            auto marginal_B123 = graph.getJointMarginalDistribution({ "B1" , "B2" ,"B3" });
            vector<Combination> comb_raw = { Combination(vector<size_t>{0, 0, 0}) , Combination(vector<size_t>{1,1,0}) };

            cout << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0)";
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.front(), marginal_B123.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_B123.find(comb_raw.front()) << endl;

            cout << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0)";
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.back(), marginal_B123.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_B123.find(comb_raw.back()) << endl;
        }
        
        // set different observation values
        graph.setEvidences({ 1,1 });
        // produce a list of samples for the hidden variables, conditioned by the novel observations
        sample = graph.getHiddenSetSamples(500, 200);
        {
            // compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
            auto marginal_A12434 = graph.getJointMarginalDistribution({ "A1" , "A2" ,"A3" ,"A4" });
            vector<Combination> comb_raw = { Combination(vector<size_t>{0, 0, 0, 0}) , Combination(vector<size_t>{1,1,0,0}) };

            cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1)" << endl;
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.front(), marginal_A12434.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_A12434.find(comb_raw.front()) << endl;

            cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1)";
            cout << "empirical" << endl;
            cout << sample::getEmpiricalFrequencies(comb_raw.back(), marginal_A12434.getGroup(), sample, graph.getHiddenVariables()) << endl;
            cout << marginal_A12434.find(comb_raw.back()) << endl;
        }
    }, "Subgraph from complex graph");

	return EXIT_SUCCESS;
}
