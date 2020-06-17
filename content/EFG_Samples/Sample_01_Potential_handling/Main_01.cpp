/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 01: Potential handling' of the documentation

#include <iostream>
#include <string>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
using namespace EFG;


void part_01();
void part_02();

int main() {
	
	//////////////////////////
	//			part 01				 //
	//////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01();
	cout << endl << endl;

	//////////////////////////
	//			part 02    			 //
	//////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02();
	cout << endl << endl;

	return 0;
}

void part_01() {

	//create a couple of variables with Dom size equal to 4
	Categoric_var A(4, "A");
	Categoric_var B(4, "B");

	//create a Simple shape involving A and B
	Potential_Shape Phi_AB({&A, &B});
	// at this point the image of Phi_AB has all values equal to 0
	//add values in order to have for Phi_AB(a,b) = a + 2*b (without a particular reason)
	size_t ka, kb;
	for (ka = 0; ka < A.size(); ka++) {
		for (kb = 0; kb < B.size(); kb++)
			Phi_AB.Add_value({ ka, kb }, (float)(ka + 2 * kb));  //combinations must follow the same order used for building the factor
	}
	//print the distribution
	cout << "Phi_AB\n";
	Phi_AB.Print_distribution(cout);
	cout << endl;

	//the same distribution can be red from a file in order to build the same factor
	//create the file to read with the same values 
	ofstream f("__temp_file");
	for (ka = 0; ka < A.size(); ka++) {
		for (kb = 0; kb < B.size(); kb++)
			f << ka << " " << kb << " " << (float)(ka + 2 * kb) << endl;
	}
	f.close();
	Potential_Shape Phi_AB_bis({ &A, &B }, string("__temp_file"));
	//delete that file 
 #ifdef __unix__ 
	if(system("rm __temp_file") == -1) cout << "error deleting __temp_file\n";
 #elif defined(_WIN32) || defined(WIN32)
	system("DEL __temp_file");
 #endif
	//print the distribution
	cout << "\n\nPhi_AB_bis\n";
	Phi_AB_bis.Print_distribution(cout);
	cout << endl;

	//get the maximum in the image of Phi_AB
	float max_Phi_AB_image = Phi_AB.max_in_distribution();
	cout << "\n\n max value in the image of Phi_AB is " << max_Phi_AB_image << endl;

	//normalize the factor
	Phi_AB.Normalize_distribution();
	cout << "\n\nPhi_AB normalized\n";
	Phi_AB.Print_distribution(cout);
	cout << endl;

	//define another couple of variables with the same Dom size if A and B
	Categoric_var X(A.size(), "A"), Y(B.size(), "B");
	//build Phi_XY, which has the same image of Phi_AB, but considering the realizations of X and Y
	Potential_Shape Phi_XY(Phi_AB, { &X, &Y }); 
	cout << "\n\nPhi_XY\n";
	Phi_XY.Print_distribution(cout);
	cout << endl;

}

void part_02() {

	//define a group of variables with a Dom size equal to 3
	size_t group_size = 3;
	vector<Categoric_var> Variables;
	Variables.reserve(group_size);
	list<Categoric_var*> Group;
	for (size_t k = 0; k < group_size; k++) {
		Variables.emplace_back(3, "V" + to_string(k));
		Group.push_back(&Variables.back());
	}

	//build a factor involving Group_1, considering a simple correlating distribution: the values of the image are equal to 1 only for those
	//combinations for which all variables assume the same realization number
	Potential_Shape Phi_corr(Group, true);
	cout << "Phi_corr \n";
	Phi_corr.Print_distribution(cout); //by default only the combination having an image value non null are showed. You can change it invoking Phi_corr.Print_distribution(cout, true)
	cout << endl;

	//now the factor expressing an anticorrelation for the variables in Group1 is built
	Potential_Shape Phi_anti_corr(Group, false);
	cout << "\n\nPhi_anti_corr \n";
	Phi_anti_corr.Print_distribution(cout); //by default only the combination having an image value non null are showed. You can change it invoking Phi_corr.Print_distribution(cout, true)
	cout << endl;

	//find all the combinations in Phi_anti_corr for which Variables[0] = 0 , Variables[1] = 1 and whatever values all the others
	I_Potential::combinations comb({ 0,1 }, { &Variables[0], &Variables[1] });
	vector<list<const I_Potential::I_Distribution_value*>> found_matching_combination;
	comb.Find_images_multiple_matches(&found_matching_combination , Phi_anti_corr);
	cout << "\n\nValue in Phi_anti_corr for which Variables[0] = 0 and Variables[1] = 1 \n";
	for (auto it = found_matching_combination.front().begin(); it != found_matching_combination.front().end(); it++) {
		const size_t* combination = (*it)->Get_indeces();
		cout << combination[0];
		for (size_t k = 1; k < group_size; k++)
			cout << " " << combination[k];
		cout << " " << (*it)->Get_val() << endl;
	}


}

