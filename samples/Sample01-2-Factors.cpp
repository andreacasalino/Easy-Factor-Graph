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
using namespace EFG;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  {
    SampleSection section("Factors construction", "4.1.2.1");

    // create a couple of variables with Dom size equal to 4
    auto A = categoric::make_variable(4, "A");
    auto B = categoric::make_variable(4, "B");

    // create a Simple shape involving A and B
    distribution::Factor Phi_AB(
        categoric::Group{categoric::VariablesSoup{A, B}});
    // fill the domain in order to have for Phi_AB(a,b) = a + 2*b (just to put
    // some numbers)
    {
      categoric::GroupRange range_AB(Phi_AB.getVariables());
      categoric::for_each_combination(
          range_AB, [&Phi_AB](const categoric::Combination &comb) {
            const auto &data = comb.data();
            Phi_AB.setImageRaw(comb, static_cast<float>(data[0] + 2 * data[1]));
          });
    }
    // print the distribution
    cout << Phi_AB << endl << endl;

    // define another couple of variables with the same Dom size of A and B
    auto X = categoric::make_variable(4, "X");
    auto Y = categoric::make_variable(4, "Y");

    // build a factor involving X and Y, cloning the one that involves A and B
    distribution::Factor Phi_XY(Phi_AB); // initially clone as is
    Phi_XY.replaceVariables(
        categoric::VariablesSoup{X, Y}); // then replace variables group
    cout << Phi_XY << endl << endl;

    // permute X with Y
    distribution::Factor Phi_XY_permuted = Phi_XY.cloneWithPermutedGroup(
        categoric::Group{categoric::VariablesSoup{Y, X}});
    cout << Phi_XY_permuted << endl << endl;
  }

  {
    SampleSection section("Simple Factor query", "4.1.2.2");

    // build the variables
    auto V1 = categoric::make_variable(3, "V1");
    auto V2 = categoric::make_variable(3, "V2");
    auto V3 = categoric::make_variable(3, "V3");

    // build a factor correlating V1, V2 and V3
    distribution::Factor Phi_C = distribution::Factor(
        categoric::Group{categoric::VariablesSoup{V1, V2, V3}},
        distribution::USE_SIMPLE_CORRELATION_TAG);
    cout << "Correlating factor" << endl;
    cout << Phi_C << endl << endl;

    // build a factor correlating V1, V2 and V3
    distribution::Factor Phi_A = distribution::Factor(
        categoric::Group{categoric::VariablesSoup{V1, V2, V3}},
        distribution::USE_SIMPLE_ANTI_CORRELATION_TAG);
    cout << "Anti correlating factor" << endl;
    cout << Phi_A << endl << endl;

    float weight =
        1.5f; // you can tune this value to see how the probabilities change

    // build the exponential correlating factor and evaluates the probabilities
    distribution::FactorExponential Psi_C(Phi_C, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_C.getProbabilities() << endl << endl;

    // build the exponential anti correlating factor and evaluates the
    // probabilities
    distribution::FactorExponential Psi_A(Phi_A, weight);
    cout << "probabilities taken from the correlating exponential factor "
         << endl;
    cout << Psi_A.getProbabilities() << endl << endl;
  }

  {
    SampleSection section("Find specific combination in Factors");

    distribution::Factor factor(categoric::Group{categoric::VariablesSoup{
        categoric::make_variable(2, "A"), categoric::make_variable(3, "B"),
        categoric::make_variable(2, "C"), categoric::make_variable(3, "D")}});
    factor.setImageRaw(std::vector<std::size_t>{0, 0, 0, 0}, 1.f);
    factor.setImageRaw(std::vector<std::size_t>{0, 0, 1, 0}, 2.f);
    factor.setImageRaw(std::vector<std::size_t>{1, 0, 1, 1}, 3.f);
    cout << "current content of the distribution" << endl;
    cout << factor << endl << endl;

    cout << "value found for 0 0 1 0  ->  "
         << factor.evaluate(std::vector<std::size_t>{0, 0, 1, 0}) << endl;

    auto bigger_group = factor.getVariables().getVariables();
    bigger_group.push_back(categoric::make_variable(2, "E"));
    auto combination_finder = factor.makeFinder(bigger_group);
    cout << "value found for <1,0,1,1,0> from group " << bigger_group << "  -> "
         << combination_finder.find(std::vector<std::size_t>{1, 0, 1, 1, 0})
                .value
         << endl;
  }

  {
    SampleSection section("Factors merging");

    auto A = categoric::make_variable(2, "A");
    auto B = categoric::make_variable(2, "B");
    auto C = categoric::make_variable(2, "C");

    distribution::Factor factor_AC(
        categoric::Group{categoric::VariablesSoup{A, C}},
        distribution::USE_SIMPLE_CORRELATION_TAG);
    distribution::Factor factor_BC(
        categoric::Group{categoric::VariablesSoup{B, C}},
        distribution::USE_SIMPLE_CORRELATION_TAG);

    std::cout << "distributions to merge" << std::endl;
    cout << factor_AC << endl << endl;
    cout << factor_BC << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << distribution::Factor{factor_AC, factor_BC} << endl << endl;

    // change factors to merge and then merge again
    factor_AC.setAllImagesRaw(0.5f);
    factor_BC.setAllImagesRaw(0.5f);

    std::cout << "distributions to merge after the change" << std::endl;
    cout << factor_AC << endl << endl;
    cout << factor_BC << endl << endl;

    std::cout << "merged distribution" << std::endl;
    cout << distribution::Factor{factor_AC, factor_BC} << endl << endl;
  }

  return EXIT_SUCCESS;
}
