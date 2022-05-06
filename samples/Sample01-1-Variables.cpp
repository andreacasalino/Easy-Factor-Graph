/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/categoric/Variable.h>
using namespace EFG::categoric;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  {
    SampleSection section("Managing group of variables", "4.1.1.1");

    // define a group of variables, all with size = 2
    Group groupABCD(make_variable(2, "A"), make_variable(2, "B"));
    cout << "groupABCD with only A and B for the moment: " << groupABCD << endl;

    groupABCD.add(make_variable(2, "C"));
    groupABCD.add(make_variable(2, "D"));
    cout << "groupABCD after adding also C and D: " << groupABCD << endl;

    // try to add an already existing variable
    try {
      groupABCD.add(make_variable(2, "C"));
    } catch (...) {
      cout << "insertion of C in ABCD group correctly refused" << endl;
    }
  }

  {
    SampleSection section("Joint domain of variables", "4.1.1.2");
    // build some variables
    auto A = make_variable(2, "A");
    auto B = make_variable(4, "B");
    auto C = make_variable(3, "C");

    {
      Group AB_group(VariablesSoup{A, B});
      cout << AB_group.getVariables() << endl;
      // display all combinations in the joint domain
      GroupRange range(AB_group);
      for_each_combination(
          range, [](const Combination &comb) { cout << comb << std::endl; });
      cout << endl;
    }

    {
      Group AC_group(VariablesSoup{A, C});
      cout << AC_group.getVariables() << endl;
      // display all combinations in the joint domain
      GroupRange range(AC_group);
      for_each_combination(
          range, [](const Combination &comb) { cout << comb << std::endl; });
      cout << endl;
    }

    {
      Group ABC_group(VariablesSoup{A, C, B});
      cout << ABC_group.getVariables() << endl;
      // display all combinations in the joint domain
      GroupRange range(ABC_group);
      for_each_combination(
          range, [](const Combination &comb) { cout << comb << std::endl; });
      cout << endl;
    }
  }

  return EXIT_SUCCESS;
}
