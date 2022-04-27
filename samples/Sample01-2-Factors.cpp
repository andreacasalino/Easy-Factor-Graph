/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
using namespace EFG::categoric;
using namespace EFG::distribution;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  {
    SampleSection section("Factors construction", "4.1.2.1");

    // create a couple of variables with Dom size equal to 4
    auto A = make_variable(4, "A");
    auto B = make_variable(4, "B");

    // create a Simple shape involving A and B
    Factor Phi_AB(Group{VariablesSoup{A, B}});
    // fill the domain in order to have for Phi_AB(a,b) = a + 2*b (just to put
    // some numbers)
    {
      GroupRange range_AB(Phi_AB.getGroup());
      for_each_combination(range_AB, [&Phi_AB](const Combination &comb) {
        const auto &data = comb.data();
        Phi_AB.setImageRaw(comb, static_cast<float>(data[0] + 2 * data[1]));
      });
    }
    // print the distribution
    cout << Phi_AB << endl << endl;

    // define another couple of variables with the same Dom size of A and B
    auto X = make_variable(4, "X");
    auto Y = make_variable(4, "Y");

    // build a factor involving X and Y, cloning the one that involves A and B
    Factor Phi_XY(Phi_AB); // initially clone as is
    Phi_XY.replaceVariables(
        VariablesSoup{X, Y}); // then replace variables group
    cout << Phi_XY << endl << endl;

    // permute X with Y
    Factor Phi_XY_permuted =
        Phi_XY.cloneWithPermutedGroup(Group{VariablesSoup{Y, X}});
    cout << Phi_XY_permuted << endl << endl;
  }

  {
    SampleSection section("Simple Factor query", "4.1.2.2");

    // build the variables
    auto V1 = make_variable(3, "V1");
    auto V2 = make_variable(3, "V2");
    auto V3 = make_variable(3, "V3");

    // build a factor correlating V1, V2 and V3
    Factor Phi_C =
        Factor(Group{VariablesSoup{V1, V2, V3}}, USE_SIMPLE_CORRELATION_TAG);
    cout << "Correlating factor" << endl;
    cout << Phi_C << endl << endl;

    // build a factor correlating V1, V2 and V3
    Factor Phi_A = Factor(Group{VariablesSoup{V1, V2, V3}},
                          USE_SIMPLE_ANTI_CORRELATION_TAG);
    cout << "Anti correlating factor" << endl;
    cout << Phi_A << endl << endl;

    float weight =
        1.5f; // you can tune this value to see how the probabilities change

    // build the exponential correlating factor and evaluates the probabilities
    FactorExponential Psi_C(Phi_C, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_C.getProbabilities() << endl << endl;

    // build the exponential anti correlating factor and evaluates the
    // probabilities
    FactorExponential Psi_A(Phi_A, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_A.getProbabilities() << endl << endl;
  }

  {
    SampleSection section("Find specific combination in Factors");

    Factor factor(
        Group{VariablesSoup{make_variable(2, "A"), make_variable(3, "B"),
                            make_variable(2, "C"), make_variable(3, "D")}});
    factor.setImageRaw(std::vector<std::size_t>{0, 0, 0, 0}, 1.f);
    factor.setImageRaw(std::vector<std::size_t>{0, 0, 1, 0}, 2.f);
    factor.setImageRaw(std::vector<std::size_t>{1, 0, 1, 1}, 3.f);
    cout << "current content of the distribution" << endl;
    cout << factor << endl << endl;

    cout << "value found for 0 0 1 0  ->  "
         << factor.evaluate(std::vector<std::size_t>{0, 0, 1, 0}) << endl;

    auto bigger_group = factor.getGroup().getVariables();
    bigger_group.push_back(make_variable(2, "E"));
    auto combination_finder = factor.makeFinder(bigger_group);
    cout << "value found for <1,0,1,1,0> from group " << bigger_group << "  -> "
         << combination_finder.find(std::vector<std::size_t>{1, 0, 1, 1, 0})
                .value
         << endl;
  }

  {
    SampleSection section("Factors merging");

    auto A = make_variable(2, "A");
    auto B = make_variable(2, "B");
    auto C = make_variable(2, "C");

    Factor factor_AC(Group{VariablesSoup{A, C}}, USE_SIMPLE_CORRELATION_TAG);
    Factor factor_BC(Group{VariablesSoup{B, C}}, USE_SIMPLE_CORRELATION_TAG);

    std::cout << "distributions to merge" << std::endl;
    cout << factor_AC << endl << endl;
    cout << factor_BC << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << Factor{factor_AC, factor_BC} << endl << endl;

    // change factors to merge and then merge again
    factor_AC.setAllImagesRaw(0.5f);
    factor_BC.setAllImagesRaw(0.5f);

    std::cout << "distributions to merge after the change" << std::endl;
    cout << factor_AC << endl << endl;
    cout << factor_BC << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << Factor{factor_AC, factor_BC} << endl << endl;
  }

  return EXIT_SUCCESS;
}
