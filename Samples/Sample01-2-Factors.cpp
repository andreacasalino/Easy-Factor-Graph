/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Presenter.h>
#include <print/DistributionPrint.h>
#include <print/ProbabilityDistributionPrint.h>
#include <print/GroupPrint.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/const/Message.h>
#include <distribution/DistributionFinder.h>
#include <distribution/factor/const/FactorExponential.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

int main () {
	// define a group and a distribution
	factor::modif::Factor distr(Group({ makeVariable(2 , "A"), makeVariable(3 , "B"), makeVariable(2 , "C"), makeVariable(3 , "D") }));

	EFG::sample::samplePart([&distr]() {
		cout << "emplacing all domain with 3" << endl;
		distr.setImageEntireDomain(3.f);
		cout << distr << endl << endl;

		cout << "clear" << endl;
		distr.clear();
		cout << distr << endl << endl;

		cout << "emplace missing values" << endl;
		distr.emplaceEntireDomain();
		cout << distr << endl << endl;
	}, "Distribution manipulation");

	EFG::sample::samplePart([&distr]() {
		cout << "marginalize considering A=1 and C=1" << endl;
		cout << factor::cnst::Factor(distr, Combination({ 1,1 }), { makeVariable(2 , "A"), makeVariable(3 , "C") }) << endl << endl;
	}, "Marginalization");

	EFG::sample::samplePart([&distr]() {
		distr.clear();
		distr.add(EFG::Combination({ 0,0,0,0 }), 1.f);
		distr.add(EFG::Combination({ 0,0,1,0 }), 2.f);
		distr.add(EFG::Combination({ 1,0,1,1 }), 3.f);
		cout << "current content of the distribution" << endl;
		cout << distr << endl << endl;

		cout << "value found for <0,0,1,0>  ->  " << distr.find(EFG::Combination({ 0,0,1,0 })) << endl;

		auto groupBigger = distr.getGroup();
		groupBigger.add(makeVariable(2, "E"));
		DistributionFinder finder(distr, groupBigger.getVariables());
		cout << "value found for <1,0,1,1,0> from group " << groupBigger << "  ->  " << finder.find(EFG::Combination({ 1,0,1,1,0 })).second << endl;
	}, "Find combination");

	EFG::sample::samplePart([]() {
		 	factor::modif::Factor distrAC(Group({makeVariable(2 , "A"), makeVariable(2 , "C")}));
		 	distrAC.setImageEntireDomain(2.f);
		 	factor::modif::Factor distrBC(Group({makeVariable(2 , "B"), makeVariable(2 , "C")}));
		 	distrBC.setImageEntireDomain(0.5f);

		 	std::cout << "distributions to merge" << std::endl;
		 	cout << distrAC << endl << endl;
		 	cout << distrBC << endl << endl;

		 	std::cout << "merged distribution" << std::endl;
		 	cout << factor::modif::Factor(&distrAC, &distrBC) << endl << endl;
	}, "Merge distributions");

	EFG::sample::samplePart([]() {
		factor::modif::Factor simpleCorrelation(Group({ makeVariable(3 , "A"), makeVariable(3 , "B"), makeVariable(3 , "C") }), true);
		simpleCorrelation.emplaceEntireDomain();
		std::cout << "simple correlating factor" << std::endl;
		cout << simpleCorrelation << endl << endl;
		std::cout << "probability distribution" << std::endl;
		cout << simpleCorrelation.getProbabilities() << endl << endl;

		factor::cnst::FactorExponential simpleExpCorrelating(simpleCorrelation, 1.f);
		std::cout << "exponential factor built considering a simple correlating factor" << std::endl;
		cout << simpleExpCorrelating << endl << endl;
		std::cout << "probability distribution" << std::endl;
		cout << simpleExpCorrelating.getProbabilities() << endl << endl;

		factor::modif::Factor antiCorrelation(Group({ makeVariable(3 , "A"), makeVariable(3 , "B"), makeVariable(3 , "C") }), false);
		antiCorrelation.emplaceEntireDomain();
		std::cout << "anti correlating factor" << std::endl;
		cout << antiCorrelation << endl << endl;
	}, "Simple correlations");

	EFG::sample::samplePart([]() {
		factor::modif::Factor factor(Group({ makeVariable(3 , "A"), makeVariable(3 , "B")}));
		factor.add(std::vector<std::size_t>{0, 0}, 1);
		factor.add(std::vector<std::size_t>{0, 1}, 2);
		factor.add(std::vector<std::size_t>{0, 2}, 3);
		factor.add(std::vector<std::size_t>{1, 0}, 1);
		factor.add(std::vector<std::size_t>{1, 1}, 2);
		std::cout << "distribution representing an edge" << factor << std::endl << std::endl;

		std::cout << "Map message keeping A" << std::endl << std::endl;
		std::cout << factor::cnst::MessageMAP(factor, Group(makeVariable(3, "B"))) << std::endl;

		std::cout << "Sum message keeping A" << std::endl << std::endl;
		std::cout << factor::cnst::MessageSum(factor, Group(makeVariable(3, "B"))) << std::endl;
	}, "Message computation");

	return EXIT_SUCCESS;
}



// // Refer also to Section 'Sample 01: Potential handling' of the documentation

// #include <iostream>
// #include <string>
// using namespace std;

// #include <potential/Factor.h>
// #include <distribution/PartialMatchFinder.h>
// #include "../Utilities.h"
// using namespace EFG;



// void part_01();
// void part_02();
// void part_03();

// int main() {
	
// 	//////////////////////////
// 	//			part 01	    //
// 	//////////////////////////
// 	cout << "-----------------------\n";
// 	cout << "part 01 \n\n\n";
// 	cout << "-----------------------\n";
// 	part_01();
// 	cout << endl << endl;

// 	//////////////////////////
// 	//			part 02     //
// 	//////////////////////////
// 	cout << "-----------------------\n";
// 	cout << "part 02 \n\n\n";
// 	cout << "-----------------------\n";
// 	part_02();
// 	cout << endl << endl;

// 	//////////////////////////
// 	//			part 03     //
// 	//////////////////////////
// 	cout << "-----------------------\n";
// 	cout << "part 03 \n\n\n";
// 	cout << "-----------------------\n";
// 	part_03();
// 	cout << endl << endl;

// 	return 0;
// }

// void part_01() {

// 	//create a couple of variables with Dom size equal to 4
// 	CategoricVariable A(4, "A");
// 	CategoricVariable B(4, "B");

// 	//create a Simple shape involving A and B
// 	pot::Factor Phi_AB({&A, &B});
// 	//fill the domain in order to have for Phi_AB(a,b) = a + 2*b (without a particular reason)
// 	{
// 		JointDomainIterator::forEach({ &A, &B }, [&Phi_AB](const vector<size_t>& comb) {
// 			size_t a = comb[0];
// 			size_t b = comb[1];
// 			Phi_AB.AddValue(std::vector<size_t>{a, b}, (float)(a + 2 * b));
// 		});
// 	}
// 	//print the distribution
// 	cout << "Phi_AB\n" << Phi_AB.GetDistribution() << endl;

// 	//the same distribution can be red from a file in order to build the same potential
// 	//create the file to read with the same values 
// 	{
// 		ofstream f("__temp_file");
// 		JointDomainIterator::forEach({ &A, &B }, [&f](const vector<size_t>& comb) { f << comb[0] << " " << comb[1] << " " << comb[0] + 2.f * comb[1] << endl; });
// 		f.close();
// 	}
// 	pot::Factor Phi_AB_bis({ &A, &B }, string("__temp_file"));
// 	//delete that file 
//  #ifdef __unix__ 
// 	if(system("rm __temp_file") == -1) cout << "error deleting __temp_file\n";
//  #elif defined(_WIN32) || defined(WIN32)
// 	system("DEL __temp_file");
//  #endif
// 	//print the distribution
// 	cout << "\n\nPhi_AB_bis\n" << Phi_AB_bis.GetDistribution() << endl;

// 	//normalize the factor
// 	Phi_AB.Normalize();
// 	cout << "\n\nPhi_AB_normalized\n" << Phi_AB.GetDistribution() << endl;

// 	//define another couple of variables with the same Dom size if A and B
// 	CategoricVariable X(A.size(), "X"), Y(B.size(), "Y");
// 	//build Phi_XY, which has the same image of Phi_AB, but considering the realizations of X and Y
// 	pot::Factor Phi_XY(Phi_AB, { &X, &Y }); 
// 	cout << "\n\nPhi_XY\n" << Phi_XY.GetDistribution() << endl;

// }

// void part_02() {

// 	//define a group of variables with a Dom size equal to 3
// 	size_t group_size = 3;
// 	vector<CategoricVariable> Variables;
// 	Variables.reserve(group_size);
// 	vector<CategoricVariable*> Group;
// 	Group.reserve(group_size);
// 	for (size_t k = 0; k < group_size; k++) {
// 		Variables.emplace_back(3, "V" + to_string(k));
// 		Group.push_back(&Variables.back());
// 	}

// 	//build a factor involving Group_1, considering a simple correlating distribution: the values of the image are equal to 1 only for those
// 	//combinations for which all variables assume the same realization number
// 	pot::Factor Phi_corr(Group, true);  //only the combination having an image value non null are added
// 	cout << "Phi_corr" << endl << Phi_corr.GetDistribution() << endl;

// 	//now the factor expressing an anticorrelation for the variables in Group1 is built
// 	pot::Factor Phi_anti_corr(Group, false);
// 	cout << "\n\nPhi_anti_corr" << endl << Phi_anti_corr.GetDistribution() << endl;

// 	// find all the combinations in Phi_anti_corr for which Variables[0] = 0 , Variables[1] = 1 and whatever values all the others
// 	{
// 		distr::DiscreteDistribution::constPartialMatchFinder finder(Phi_anti_corr.GetDistribution(), { &Variables[0], &Variables[1] });
// 		auto matching_vals = finder(vector<size_t>{0, 1});
// 		cout << "\n\nValue in Phi_anti_corr for which Variables[0] = 0 and Variables[1] = 1 \n";
// 		for (auto it : matching_vals) {
// 			cout << "<" << it->GetIndeces()[0] << "," << it->GetIndeces()[1] << "," << it->GetIndeces()[2] << "> -> " << it->GetVal() << endl;
// 		}
// 	}

// }

// void part_03() {

// 	//define some variables
// 	CategoricVariable A(3, "varA");
// 	CategoricVariable B(3, "varB");
// 	CategoricVariable C(3, "varC");

// 	//create a first potential
// 	pot::Factor AB(vector<CategoricVariable*>{&A, &B}, false);
// 	//rescale the value in the domain
// 	AB *= 0.5f;
// 	cout << "AB\n" << AB.GetDistribution() << endl;

// 	//clone the potential
// 	pot::Factor ABbis(AB, AB.GetDistribution().GetVariables());

// 	//show the values of the potential generating from merging AB with ABbis
// 	cout << "AB merged with ABbis\n" << pot::Factor(vector<const pot::IPotential*>{&AB, &ABbis}, true).GetDistribution() << endl;

// 	//define a second potential
// 	pot::Factor BC(vector<CategoricVariable*>{&B, & C}, false);
// 	BC *= 0.2f;
// 	cout << "BC\n" << BC.GetDistribution() << endl;

// 	//build a potential given by the product of AB and BC
// 	pot::Factor ABC({&AB, &BC}, false, true); //since last parameter is passed as false, the potentials can have different domains, which will be merged
// 	cout << "ABC\n" << ABC.GetDistribution() << endl;

// 	//show the values in the potential generating from marginalizing B from ABC
// 	cout << "ABC marginalized with B = 1\n" << pot::Factor(vector<size_t>{1}, vector<CategoricVariable*>{&B}, ABC).GetDistribution() << endl;

// }

