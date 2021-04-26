/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Presenter.h>
#include <Error.h>
#include <print/GroupPrint.h>
#include <print/RangePrint.h>
#include <iostream>
using namespace std;
using namespace EFG::categoric;

int main() {
    EFG::sample::samplePart([]() {
        //define a group variable
        Group groupABCD(makeVariable(2, "A"), makeVariable(2, "B"));
        cout << "groupABCD with only A and B for the moment: " << groupABCD << endl;

        groupABCD.add(makeVariable(2, "C"));
        groupABCD.add(makeVariable(2, "D"));
        cout << "groupABCD: " << groupABCD << endl;

        // try to add an already existing variable
        try {
            groupABCD.add(makeVariable(2, "C"));
        }
        catch (...) {
            cout << "insertion of C in ABCD group correctly refused" << endl;
        }
        cout << "groupABCD: " << groupABCD << endl;
    }, "Group of variables", "refer to Section 4.1.1.1 of the documentation");

    EFG::sample::samplePart([]() {
        // build some variables
        auto A = makeVariable(2, "A");
        auto B = makeVariable(4, "B");
        auto C = makeVariable(3, "C");

        // build and display the joint domains
        cout << "A B range:" << endl;
        cout << Range({ A, B }) << endl;

        cout << "A C range:" << endl;
        cout << Range({ A, C }) << endl;

        cout << "A B C range:" << endl;
        cout << Range({ A, B, C }) << endl;
    }, "Joint domains", "refer to Section 4.1.1.2 of the documentation");

	return EXIT_SUCCESS;
}

