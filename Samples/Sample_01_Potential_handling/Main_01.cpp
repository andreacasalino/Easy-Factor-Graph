/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 01: Potential handling' of the documentation

#include <fstream>
#include <iostream>
#include <string>
using namespace std;

#include <Potential.h>
#include "../Utilities.h"
using namespace EFG;



void part_01();
void part_02();
void part_03();

ostream& operator<<(ostream& s, const Discrete_Distribution& pot);

int main() {
	
	//////////////////////////
	//			part 01	    //
	//////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01();
	cout << endl << endl;

	//////////////////////////
	//			part 02     //
	//////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02();
	cout << endl << endl;

	//////////////////////////
	//			part 03     //
	//////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	part_03();
	cout << endl << endl;

	return 0;
}

void part_01() {

	//create a couple of variables with Dom size equal to 4
	Categoric_var A(4, "A");
	Categoric_var B(4, "B");

	//create a Simple shape involving A and B
	Potential_Shape Phi_AB({&A, &B});
	//fill the domain in order to have for Phi_AB(a,b) = a + 2*b (without a particular reason)
	{
		Domain_iterator AB_iter({&A, &B});
		size_t a , b;
		while(AB_iter.is_not_at_end()){
			a = AB_iter()[0];
			b = AB_iter()[1];
			Phi_AB.Add_value(std::vector<size_t>{a,b} , (float)(a + 2 * b));
			++AB_iter;
		}	
	}
	//print the distribution
	cout << "Phi_AB\n" << Phi_AB.Get_Distribution() << endl;

	//the same distribution can be red from a file in order to build the same potential
	//create the file to read with the same values 
	{
		ofstream f("__temp_file");
		Domain_iterator AB_iter({&A, &B});
		while(AB_iter.is_not_at_end()){
			f << AB_iter()[0] << " " << AB_iter()[1] << " " << (float)(AB_iter()[0]  + 2*AB_iter()[1]) << endl; 
			++AB_iter;
		}	
		f.close();
	}
	Potential_Shape Phi_AB_bis({ &A, &B }, string("__temp_file"));
	//delete that file 
 #ifdef __unix__ 
	if(system("rm __temp_file") == -1) cout << "error deleting __temp_file\n";
 #elif defined(_WIN32) || defined(WIN32)
	system("DEL __temp_file");
 #endif
	//print the distribution
	cout << "\n\nPhi_AB_bis\n" << Phi_AB_bis.Get_Distribution() << endl;

	//normalize the factor
	Phi_AB.Normalize_distribution();
	cout << "\n\nPhi_AB_normalized\n" << Phi_AB.Get_Distribution() << endl;

	//define another couple of variables with the same Dom size if A and B
	Categoric_var X(A.size(), "X"), Y(B.size(), "Y");
	//build Phi_XY, which has the same image of Phi_AB, but considering the realizations of X and Y
	Potential_Shape Phi_XY(Phi_AB, { &X, &Y }); 
	cout << "\n\nPhi_XY\n" << Phi_XY.Get_Distribution() << endl;

}


void part_02() {

	//define a group of variables with a Dom size equal to 3
	size_t group_size = 3;
	vector<Categoric_var> Variables;
	Variables.reserve(group_size);
	vector<Categoric_var*> Group;
	Group.reserve(group_size);
	for (size_t k = 0; k < group_size; k++) {
		Variables.emplace_back(3, "V" + to_string(k));
		Group.push_back(&Variables.back());
	}

	//build a factor involving Group_1, considering a simple correlating distribution: the values of the image are equal to 1 only for those
	//combinations for which all variables assume the same realization number
	Potential_Shape Phi_corr(Group, true);  //only the combination having an image value non null are added
	cout << "Phi_corr" << endl << Phi_corr.Get_Distribution() << endl;

	//now the factor expressing an anticorrelation for the variables in Group1 is built
	Potential_Shape Phi_anti_corr(Group, false);
	cout << "\n\nPhi_anti_corr" << endl << Phi_anti_corr.Get_Distribution() << endl;

	// find all the combinations in Phi_anti_corr for which Variables[0] = 0 , Variables[1] = 1 and whatever values all the others
	{
		Discrete_Distribution::const_Partial_Match_finder finder(Phi_anti_corr.Get_Distribution(), { &Variables[0], &Variables[1] });
		auto matching_vals = finder(vector<size_t>{0, 1});
		cout << "\n\nValue in Phi_anti_corr for which Variables[0] = 0 and Variables[1] = 1 \n";
		for (auto it : matching_vals) {
			cout << "<" << it->Get_indeces()[0] << "," << it->Get_indeces()[1] << "," << it->Get_indeces()[2] << "> -> " << it->Get_val() << endl;
		}
	}

}

void part_03() {

	//define some variables
	Categoric_var A(3, "varA");
	Categoric_var B(3, "varB");
	Categoric_var C(3, "varC");

	//create a first potential
	Potential_Shape AB(vector<Categoric_var*>{&A, &B}, false);
	//rescale the value in the domain
	AB *= 0.5f;
	cout << "AB\n" << AB.Get_Distribution() << endl;

	//clone the potential
	Potential_Shape ABbis(AB, AB.Get_Distribution().Get_Variables());

	//show the values of the potential generating from merging AB with ABbis
	cout << "AB merged with ABbis\n" << Potential_Shape(vector<const I_Potential*>{&AB, &ABbis}, true).Get_Distribution() << endl;

	//define a second potential
	Potential_Shape BC(vector<Categoric_var*>{&B, & C}, false);
	cout << "BC\n" << BC.Get_Distribution() << endl;
	BC *= 0.2f;

	//build a potential given by the product of AB and BC
	Potential_Shape ABC({&AB, &BC}, false, true); //since last parameter is passed as false, the potentials can have different domains, which will be merged
	cout << "ABC\n" << ABC.Get_Distribution() << endl;

	//show the values in the potential generating from marginalizing B from ABC
	cout << "ABC marginalized with B = 1\n" << Potential_Shape(vector<size_t>{1}, vector<Categoric_var*>{&B}, ABC).Get_Distribution() << endl;

}

