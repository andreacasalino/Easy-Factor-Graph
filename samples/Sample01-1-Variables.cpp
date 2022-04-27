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

  //   EFG::sample::samplePart(
  //       []() {
  //         // build some variables
  //         auto A = makeVariable(2, "A");
  //         auto B = makeVariable(4, "B");
  //         auto C = makeVariable(3, "C");

  //         // build and display the joint domains
  //         cout << "A B range:" << endl;
  //         cout << Range({A, B}) << endl;

  //         cout << "A C range:" << endl;
  //         cout << Range({A, C}) << endl;

  //         cout << "A B C range:" << endl;
  //         cout << Range({A, B, C}) << endl;
  //       },
  //       "Joint domains", "refer to Section 4.1.1.2 of the documentation");

  return EXIT_SUCCESS;
}
