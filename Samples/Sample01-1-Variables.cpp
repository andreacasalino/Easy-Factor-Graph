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

int main() {
{
	EFG::sample::Presenter presenter("variables group" , "", "");
    //define a group variable
    EFG::categoric::Group groupABCD(EFG::categoric::makeVariable(2 , "A") , EFG::categoric::makeVariable(2 , "B"));
    cout << "groupABCD: " << groupABCD << endl; 

    groupABCD.add(EFG::categoric::makeVariable(2 , "C"));
    groupABCD.add(EFG::categoric::makeVariable(2 , "D"));
    cout << "groupABCD: " << groupABCD << endl; 

    // try to add an already existing variable
    try {
        groupABCD.add(EFG::categoric::makeVariable(2 , "C"));
    }
    catch(...) {
        cout << "insertion correctly refused" << endl;
    }
    cout << "groupABCD: " << groupABCD << endl; 
}

{
	EFG::sample::Presenter presenter("range for joint domain" , "", "");
	// build some variables
	auto A = EFG::categoric::makeVariable(2 , "A");
	auto B = EFG::categoric::makeVariable(4 , "B");
	auto C = EFG::categoric::makeVariable(3 , "C");
    
	// build and display the joint domains
	cout << "A B range:" << endl;
	cout << EFG::categoric::Range({A, B}) << endl;

	cout << "A C range:" << endl;
	cout << EFG::categoric::Range({A, C}) << endl;

	cout << "A B C range:" << endl;
	cout << EFG::categoric::Range({A, B, C}) << endl;
}

	return EXIT_SUCCESS;
}

