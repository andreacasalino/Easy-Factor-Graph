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
using namespace EFG;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  {
    SampleSection section("Group of variables managing", "4.1.1.1");

    // define a group of variables, all with size = 2
    categoric::Group groupABCD(categoric::make_variable(2, "A"),
                               categoric::make_variable(2, "B"));
    cout << "groupABCD with only A and B for the moment: " << groupABCD << endl;

    groupABCD.add(categoric::make_variable(2, "C"));
    groupABCD.add(categoric::make_variable(2, "D"));
    cout << "groupABCD after adding also C and D: " << groupABCD << endl;

    // try to add an already existing variable
    try {
      groupABCD.add(categoric::make_variable(2, "C"));
    } catch (...) {
      cout << "insertion of C in ABCD group correctly refused" << endl;
    }
    cout << "groupABCD: " << groupABCD << endl;
  }

  {
    SampleSection section("Joint domain of variables", "4.1.1.2");
    // build some variables
    auto A = categoric::make_variable(2, "A");
    auto B = categoric::make_variable(4, "B");
    auto C = categoric::make_variable(3, "C");

    {
      categoric::Group AB_group(categoric::VariablesSoup{A, B});
      cout << AB_group.getVariables() << endl;
      // display all combinations in the joint domain
      EFG::categoric::GroupRange range(AB_group);
      EFG::categoric::for_each_combination(
          range, [](const EFG::categoric::Combination &comb) {
            cout << comb << std::endl;
          });
      cout << endl;
    }

    {
      categoric::Group AC_group(categoric::VariablesSoup{A, C});
      cout << AC_group.getVariables() << endl;
      // display all combinations in the joint domain
      EFG::categoric::GroupRange range(AC_group);
      EFG::categoric::for_each_combination(
          range, [](const EFG::categoric::Combination &comb) {
            cout << comb << std::endl;
          });
      cout << endl;
    }

    {
      categoric::Group ABC_group(categoric::VariablesSoup{A, C, B});
      cout << ABC_group.getVariables() << endl;
      // display all combinations in the joint domain
      EFG::categoric::GroupRange range(ABC_group);
      EFG::categoric::for_each_combination(
          range, [](const EFG::categoric::Combination &comb) {
            cout << comb << std::endl;
          });
      cout << endl;
    }
  }

  return EXIT_SUCCESS;
}
