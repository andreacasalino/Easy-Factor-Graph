/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 04: Hidden Markov model like structure' of the documentation

#include <model/Graph.h>
#include <io/xml/Exporter.h>
#include <distribution/factor/const/Indicator.h>
#include <print/ProbabilityDistributionPrint.h>
#include <Presenter.h>
#include <math.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::io;

unique_ptr<model::Graph> makeChain(const std::size_t& Chain_size, const std::size_t& var_size, const float& w_XY, const float& w_YY);

int main() {
	EFG::sample::samplePart([]() {
		size_t chain_size = 10; //you can change it
		size_t var_dom_size = 2; //you can change it

		vector<size_t> Y_MAP;
		Y_MAP.reserve(chain_size);

		//create a chain with a strong weight on the potentials XY. Evidences are set as indicated in Sample 04 
		auto G_XY = makeChain(chain_size, var_dom_size, 2.f, 0.5f);
		//compute MAP on hidden variables and display it
		for (size_t k = 0; k < chain_size; ++k) {
			Y_MAP.push_back(G_XY->getMAP("Y_" + to_string(k)));
		}
		cout << "Strong correlation with edivences,   MAP on Y0,1,..   " << Y_MAP << endl;

		//create a chain with a strong weight on the potentials YY. Evidences are set as indicated in Sample 04
		auto G_YY = makeChain(chain_size, var_dom_size, 0.5f, 2.f);
		//compute MAP on hidden variables and display it
		Y_MAP.clear();
		for (size_t k = 0; k < chain_size; ++k) {
			Y_MAP.push_back(G_YY->getMAP("Y_" + to_string(k)));
		}
		cout << "Strong correlation among hidden variables,   MAP on Y0,1,..   " << Y_MAP << endl;

		//export chains into an xml (just as an exporting example)
		io::xml::Exporter::exportToXml(*G_XY, "./", "Graph_XY.xml");
		io::xml::Exporter::exportToXml(*G_YY, "./", "Graph_YY.xml");
	}, "HMM", "", "Check the content of Graph_XY.xml and Graph_YY.xml");

	return EXIT_SUCCESS;
}

unique_ptr<model::Graph> makeChain(const std::size_t& Chain_size, const std::size_t& var_size, const float& w_XY, const float& w_YY) {
	if (Chain_size < 2) throw Error("invalid chain size");
	if (var_size < 2) throw Error("invalid variable size");

	unique_ptr<model::Graph> G = make_unique<model::Graph>();
	//build the chain structure described in 'Sample 04: Hidden Markov model like structure'
	factor::cnst::FactorExponential P_XY(factor::cnst::Factor(categoric::Group(makeVariable(var_size, "X_fake"), makeVariable(var_size, "Y_fake")), true), w_XY);
	factor::cnst::FactorExponential P_YY(factor::cnst::Factor(categoric::Group(makeVariable(var_size, "Y_fake"), makeVariable(var_size, "Y2_fake")), true), w_YY);

	//build the chain and set the value of the evidences equal to:
	//X_0 = 0, X_1=var_size-1, X_2= 0, X_3 = var_size-1, etc.. 
	std::map<std::string, std::size_t> evidences;
	size_t o = 0;
	for (size_t k = 0; k < Chain_size; k++) {
		factor::modif::FactorExponential temp_XY(P_XY);
		temp_XY.replaceGroup(categoric::Group(makeVariable(var_size, "X_" + to_string(k)) , makeVariable(var_size, "Y_" + to_string(k))));
		G->Insert(temp_XY);
		if (k == 0) {
			G->Insert(factor::cnst::IndicatorFactor(makeVariable(var_size, "Y_" + to_string(k)), 0));
		}
		else {
			factor::modif::FactorExponential temp_YY(P_YY);
			temp_YY.replaceGroup(categoric::Group(makeVariable(var_size, "Y_" + to_string(k - 1)), makeVariable(var_size, "Y_" + to_string(k))));
			G->Insert(temp_YY);
		}

		if (o == 0) o = 1;
		else o = 0;
		evidences.emplace("X_" + to_string(k), o);
	}
	G->resetEvidences(evidences);
	return G;
};