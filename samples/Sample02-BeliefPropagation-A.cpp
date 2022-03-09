/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <model/Graph.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/const/FactorExponential.h>
#include <CombinationMaker.h>
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

int main () {
    EFG::sample::samplePart([](){
        //create a simple graph with two nodes
        model::Graph graph;
        float teta = 1.5f;

        factor::modif::Factor shapeTemp(std::set<categoric::VariablePtr>{ makeVariable(2, "A") , makeVariable(2, "B") });
        shapeTemp.setImageRaw(sample::makeCombination(std::vector<std::size_t>{0,0}), 1.f);
        shapeTemp.setImageRaw(sample::makeCombination(std::vector<std::size_t>{1,1}), 1.f);
        graph.insertCopy(factor::cnst::FactorExponential(shapeTemp, teta));

        //make a new belief propagation setting B=0 as observation
        graph.resetEvidences(std::map<std::string, std::size_t>{{"B", 0}});

        //compute the marginal probabilities
        cout << "P(A|B=0)" << endl;
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({expf(teta), 1.f}) << endl;
        cout << graph.getMarginalDistribution("A") << endl << endl;

        //make a new belief propagation setting B1=1 as observation
        graph.setEvidences({ 1 });

        cout << "P(A|B=1)" << endl;
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({1.f, expf(teta)}) << endl;
        cout << graph.getMarginalDistribution("A") << endl << endl;
    },  "Graph with a single potential", "refer to Section 4.2.1 of the documentation");

    EFG::sample::samplePart([](){
        model::Graph graph;
        float alfa = 0.5f, beta = 1.f;

        graph.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({makeVariable(2, "B"), makeVariable(2, "C")} , true) , alfa));
        graph.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({makeVariable(2, "A"), makeVariable(2, "B")} , true) , beta));

        //make a new belief propagation setting C=1 as observation
        graph.resetEvidences(std::map<std::string, std::size_t>{{"C", 1}});

        //compute the marginals of A,B and then compare results with the theoretical ones, see documentation
        cout << "P(B|C=1)\n";
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({ 1.f, expf(alfa)}) << endl;
        cout << graph.getMarginalDistribution("B") << endl << endl;

        cout << "P(A|C=1)\n";
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({ expf(alfa) + expf(beta), 1.f + expf(alfa) * expf(beta) }) << endl;
        cout << graph.getMarginalDistribution("A") << endl << endl;

        //make a new belief propagation setting B=1 as observation
        graph.resetEvidences(std::map<std::string, std::size_t>{{"B", 1}});

        cout << "P(A|B=1)\n";
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({ 1.f, expf(beta) }) << endl;
        cout << graph.getMarginalDistribution("A") << endl << endl;

        cout << "P(C|B=1)\n";
        cout << "theoretical " << endl;
        cout << EFG::sample::makeDistribution({ 1.f, expf(alfa) }) << endl;
        cout << graph.getMarginalDistribution("C") << endl << endl;
    }, "Graph with two potentials and 3 variables", "refer to Section 4.2.2 of the documentation");

    EFG::sample::samplePart([](){
        auto process_chain = [](const std::size_t& chain_size, const std::size_t& var_size, const float& w) {
            if (chain_size < 2) throw Error("invalid chain size");
            if (var_size < 2) throw Error("invalid variable size");

            //build the set of variables in the chain
            vector<VariablePtr> Y;
            Y.reserve(chain_size);
            for (size_t k = 0; k < chain_size; ++k) {
                Y.push_back(makeVariable(var_size, "Y_" + to_string(k)));
            }
            model::Graph graph;
            //build the correlating potentials and add it to the chain
            for (size_t k = 1; k < chain_size; ++k) {
                graph.insert(std::make_shared<factor::cnst::FactorExponential>(factor::cnst::Factor({ Y[k - 1], Y[k] }, true), w));
            }
            //set Y_0 as an observations and compute the marginals of the last variable in the chain
            graph.resetEvidences(std::map<std::string, std::size_t>{{Y.front()->name(), 0}});
            cout << graph.getMarginalDistribution(Y.back()->name()) << endl;
            cout << endl;
        };
        //build the chain
        std::size_t domainSize = 5;
        for (int k = 2; k <= 10; k++) {
            cout << "chain size equal to " << k << ", marginals of Y_n:   ";
            process_chain(k, domainSize, 3.5f);
        }
    }, "Belief degradation on a chain of variables", "refer to Section 4.2.3 of the documentation");

    return EXIT_SUCCESS;
}
