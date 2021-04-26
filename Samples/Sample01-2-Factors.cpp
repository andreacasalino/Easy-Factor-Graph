/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Presenter.h>
#include <print/DistributionPrint.h>
#include <CombinationMaker.h>
#include <print/ProbabilityDistributionPrint.h>
#include <print/GroupPrint.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/DistributionFinder.h>
#include <distribution/factor/const/FactorExponential.h>
#include <categoric/Range.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

int main () {
	EFG::sample::samplePart([]() {
		//create a couple of variables with Dom size equal to 4
		VariablePtr A = makeVariable(4, "A");
		VariablePtr B = makeVariable(4, "B");

		//create a Simple shape involving A and B
		factor::modif::Factor Phi_AB(std::set<categoric::VariablePtr>{A, B});
		//fill the domain in order to have for Phi_AB(a,b) = a + 2*b (without a particular reason)
		{
			Range range_AB(Phi_AB.getGroup().getVariables());
			EFG::iterator::forEach(range_AB, [&Phi_AB](const Range& r) {
				Phi_AB.setImageRaw(r.get(), static_cast<float>(r.get().data()[0] + 2 * r.get().data()[1]));
			});
		}
		//print the distribution
		cout << Phi_AB << endl << endl;

		//define another couple of variables with the same Dom size if A and B
		VariablePtr X = makeVariable(4, "X");
		VariablePtr Y = makeVariable(4, "Y");
		//build Phi_XY, which has the same image of Phi_AB, but considering the realizations of X and Y
		factor::modif::Factor Phi_XY(Phi_AB);
		Phi_XY.replaceGroup(Group({X, Y}));
		cout << Phi_XY << endl << endl;
	}, "Distribution construction", "refer to Section 4.1.2.1 of the documentation");

	EFG::sample::samplePart([]() {
		// build the variables
		VariablePtr V1 = makeVariable(3, "V1");
		VariablePtr V2 = makeVariable(3, "V2");
		VariablePtr V3 = makeVariable(3, "V3");

		float weight = 1.5f; // you can tune this value to see how the probabilities change

		// build the correlating factor
		factor::modif::Factor Phi_C = factor::cnst::Factor({ V1, V2, V3 }, true);
		Phi_C.fillDomain();
		cout << "Correlating factor domain and images: " << endl;
		cout << Phi_C << endl << endl;

		// build the anti correlating factor
		factor::modif::Factor Phi_A = factor::cnst::Factor({ V1, V2, V3 }, false);
		Phi_A.fillDomain();
		cout << "Anti correlating factor domain and images: " << endl;
		cout << Phi_A << endl << endl;

		// build the exponential correlating factor and evaluates the probabilities
		factor::cnst::FactorExponential Psi_C(Phi_C, weight);
		cout << "probabilities taken from the correlating exponential factor" << endl;
		cout << Psi_C.getProbabilities() << endl << endl;

		// build the exponential anti correlating factor and evaluates the probabilities
		factor::cnst::FactorExponential Psi_A(Phi_A, weight);
		cout << "probabilities taken from the anti correlating exponential factor" << endl;
		cout << Psi_A.getProbabilities() << endl << endl;
	}, "Distribution simple query", "refer to Section 4.1.2.2 of the documentation");

	EFG::sample::samplePart([]() {
		factor::modif::Factor distr(std::set<categoric::VariablePtr>({ makeVariable(2 , "A"), makeVariable(3 , "B"), makeVariable(2 , "C"), makeVariable(3 , "D") }));
		cout << "marginalize considering A=1 and C=1" << endl;
		cout << factor::cnst::Factor(distr, sample::makeCombination({ 1,1 }), { makeVariable(2 , "A"), makeVariable(3 , "C") }) << endl << endl;
	}, "Marginalization");

	EFG::sample::samplePart([]() {
		factor::modif::Factor distr(std::set<categoric::VariablePtr>({ makeVariable(2 , "A"), makeVariable(3 , "B"), makeVariable(2 , "C"), makeVariable(3 , "D") }));
		distr.setImageRaw(sample::makeCombination({ 0,0,0,0 }), 1.f);
		distr.setImageRaw(sample::makeCombination({ 0,0,1,0 }), 2.f);
		distr.setImageRaw(sample::makeCombination({ 1,0,1,1 }), 3.f);
		cout << "current content of the distribution" << endl;
		cout << distr << endl << endl;

		cout << "value found for <0,0,1,0>  ->  " << distr.find(sample::makeCombination({ 0,0,1,0 })) << endl;

		auto groupBigger = distr.getGroup();
		groupBigger.add(makeVariable(2, "E"));
		DistributionFinder finder(distr, groupBigger.getVariables());
		cout << "value found for <1,0,1,1,0> from group " << groupBigger << "  ->  " << finder.find(sample::makeCombination({ 1,0,1,1,0 })).second << endl;
	}, "Combinations find");

	EFG::sample::samplePart([]() {
		factor::modif::Factor distrAC(std::set<categoric::VariablePtr>({makeVariable(2 , "A"), makeVariable(2 , "C")}));
		distrAC.setAllImagesRaw(2.f);
		factor::modif::Factor distrBC(std::set<categoric::VariablePtr>({makeVariable(2 , "B"), makeVariable(2 , "C")}));
		distrBC.setAllImagesRaw(0.5f);

		std::cout << "distributions to merge" << std::endl;
		cout << distrAC << endl << endl;
		cout << distrBC << endl << endl;

		std::cout << "merged distribution" << std::endl;
		cout << factor::modif::Factor(&distrAC, &distrBC) << endl << endl;
	}, "Merge distributions");

	return EXIT_SUCCESS;
}
