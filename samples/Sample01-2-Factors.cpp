/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/factor/ImageFinder.h>
using namespace EFG::categoric;
using namespace EFG::factor;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Factors construction", "4.1.2.1", [] {
    // create a couple of variables with Dom size equal to 4
    auto A = make_variable(4, "A");
    auto B = make_variable(4, "B");

    // create a Simple shape involving A and B
    Factor Phi_AB(VariablesSoup{A, B});
    // fill the domain in order to have for Phi_AB(a,b) = a + 2*b (just to
    // put some numbers)
    {
      GroupRange range_AB(Phi_AB.function().vars());
      for_each_combination(range_AB, [&Phi_AB](const auto &comb) {
        Phi_AB.set(comb, static_cast<float>(comb[0] + 2 * comb[1]));
      });
    }
    // print the distribution
    cout << Phi_AB.function() << endl << endl;

    // define another couple of variables with the same Dom size of A and B
    auto X = make_variable(4, "X");
    auto Y = make_variable(4, "Y");

    // build a factor involving X and Y, cloning the one that involves A and
    // B
    Factor Phi_XY(Phi_AB); // initially clone as is
    Phi_XY.replaceVariables(
        VariablesSoup{X, Y}); // then replace variables group
    cout << Phi_XY.function() << endl << endl;

    // permute X with Y
    Factor Phi_XY_permuted = Phi_XY.cloneWithPermutedGroup(VariablesSoup{Y, X});
    cout << Phi_XY_permuted.function() << endl << endl;
  });

  SAMPLE_SECTION("Simple Factor query", "4.1.2.2", [] {
    // build the variables
    auto V1 = make_variable(3, "V1");
    auto V2 = make_variable(3, "V2");
    auto V3 = make_variable(3, "V3");

    // build a factor correlating V1, V2 and V3
    Factor Phi_C =
        Factor(VariablesSoup{V1, V2, V3}, Factor::SimplyCorrelatedTag{});
    cout << "Correlating factor" << endl;
    cout << Phi_C.function() << endl << endl;

    float weight =
        1.5f; // you can tune this value to see how the probabilities change

    // build the exponential correlating factor and evaluates the probabilities
    FactorExponential Psi_C(Phi_C, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_C.getProbabilities() << endl << endl;

    // build a factor correlating V1, V2 and V3
    Factor Phi_A =
        Factor(VariablesSoup{V1, V2, V3}, Factor::SimplyAntiCorrelatedTag{});
    cout << "Anti correlating factor" << endl;
    cout << Phi_A.function() << endl << endl;

    // build the exponential anti correlating factor and evaluates the
    // probabilities
    FactorExponential Psi_A(Phi_A, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_A.getProbabilities() << endl << endl;
  });

  SAMPLE_SECTION("Find specific combination in Factors", "", [] {
    Factor factor(VariablesSoup{make_variable(2, "A"), make_variable(3, "B"),
                                make_variable(2, "C"), make_variable(3, "D")});
    factor.set(std::vector<std::size_t>{0, 0, 0, 0}, 1.f);
    factor.set(std::vector<std::size_t>{0, 0, 1, 0}, 2.f);
    factor.set(std::vector<std::size_t>{1, 0, 1, 1}, 3.f);
    cout << "current content of the distribution" << endl;
    cout << factor.function() << endl << endl;

    cout << "value found for 0 0 1 0  ->  "
         << factor.function().findTransformed(
                std::vector<std::size_t>{0, 0, 1, 0})
         << endl;

    auto bigger_group = factor.function().vars().getVariables();
    bigger_group.push_back(make_variable(2, "E"));
    auto combination_finder = factor.makeFinder(bigger_group);
    cout << "value found for 1,0,1,1,0 from group " << bigger_group << "  -> "
         << combination_finder.findImage(
                std::vector<std::size_t>{1, 0, 1, 1, 0})
         << endl;
  });

  SAMPLE_SECTION("Factors merging", "", [] {
    auto A = make_variable(2, "A");
    auto B = make_variable(2, "B");
    auto C = make_variable(2, "C");

    Factor factor_AC(VariablesSoup{A, C}, Factor::SimplyCorrelatedTag{});
    Factor factor_BC(VariablesSoup{B, C}, Factor::SimplyCorrelatedTag{});

    std::cout << "distributions to merge" << std::endl;
    cout << factor_AC.function() << endl << endl;
    cout << factor_BC.function() << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << Factor::merge(factor_AC, factor_BC).function() << endl << endl;

    // change factors to merge and then merge again
    auto setAllImages = [](Factor &subject, float val) {
      GroupRange range{subject.function().vars()};
      for_each_combination(range,
                           [&](const auto &comb) { subject.set(comb, val); });
    };
    setAllImages(factor_AC, 0.5f);
    setAllImages(factor_BC, 0.5f);

    std::cout << "distributions to merge after the change" << std::endl;
    cout << factor_AC.function() << endl << endl;
    cout << factor_BC.function() << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << Factor::merge(factor_AC, factor_BC).function() << endl << endl;
  });

  return EXIT_SUCCESS;
}
