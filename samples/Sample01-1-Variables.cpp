/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Range.h>
using namespace EFG::categoric;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Joint domain of variables", "4.1.1.2", [] {
    // build some variables
    EFG::categoric::VarStateSize var_A_size = 2;
    EFG::categoric::VarStateSize var_B_size = 4;
    EFG::categoric::VarStateSize var_C_size = 3;

    cout << "domanin of var A and B" << endl;
    // display all combinations in the joint domain of A and B
    EFG::misc::for_each_generated<const EFG::categoric::Combination<2> *>(
        EFG::categoric::Range<2>{{var_A_size, var_B_size}},
        [](const auto *comb) {
          cout << std::span<const EFG::categoric::VarStateSize>{comb->begin(),
                                                                comb->end()}
               << std::endl;
        });
    cout << endl;

    cout << "domanin of var A and C" << endl;
    // display all combinations in the joint domain of A and C
    EFG::misc::for_each_generated<const EFG::categoric::Combination<2> *>(
        EFG::categoric::Range<2>{{var_A_size, var_C_size}},
        [](const auto *comb) {
          cout << std::span<const EFG::categoric::VarStateSize>{comb->begin(),
                                                                comb->end()}
               << std::endl;
        });
    cout << endl;

    cout << "domanin of var A, C and B" << endl;
    // display all combinations in the joint domain of A and C and B
    EFG::misc::for_each_generated<const EFG::categoric::Combination<3> *>(
        EFG::categoric::Range<3>{{var_A_size, var_C_size, var_B_size}},
        [](const auto *comb) {
          cout << std::span<const EFG::categoric::VarStateSize>{comb->begin(),
                                                                comb->end()}
               << std::endl;
        });
    cout << endl;
  });

  return EXIT_SUCCESS;
}
