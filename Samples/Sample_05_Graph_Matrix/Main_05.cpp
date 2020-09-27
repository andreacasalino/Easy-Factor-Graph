/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 05: Matricial structure' of the documentation

#include <iostream>
#include <string>
#include <memory>
using namespace std;

#include <Graphical_model.h>
#include "../Utilities.h"
using namespace EFG;

unique_ptr<Graph> create_Matrix(const size_t& Size, const size_t& var_size, const float& w_pot);

int main() {

	vector<float> marginals;

	size_t Size = 10; // A matrix of Size x Size variables will be created (see 'Sample 05: Matricial structure' of the documentation), you can change this value
	size_t var_dom_size = 3; //you can change it
	float weight_potential = 1.0f;

	auto Matrix = create_Matrix(Size, var_dom_size, weight_potential);
	//save the file into an xml (just as an example)
	Matrix->Reprint(Get_prefix() + "Sample_05_graphs/Matrix");

	//set V0_0 = 0 as an edivence and compute marginals of the variables along the diagonal of the matrix
	Categoric_var* V0 = Matrix->Find_Variable("V0_0");
	Matrix->Set_Evidences(vector<pair<string, size_t>>{{"V0_0" , 0}});

	for (size_t k = 1; k < Size; k++) {
		marginals = Matrix->Get_marginal_distribution("V" + to_string(k) + "_" + to_string(k));
		cout << "V" << k << "_" << k << "     ";
		print_distribution(marginals);
	}

	return 0;
}

unique_ptr<Graph> create_Matrix(const size_t& Size, const size_t& var_size, const float& w_pot) {

	if (Size < 2) abort();
	if (var_size < 2) abort();

	Graph* Mat = new Graph();

	//Create a correlating potential to replicate
	Categoric_var Va(var_size, "Va"), Vb(var_size, "Vb");
	Potential_Exp_Shape P_ab(Potential_Shape(vector<Categoric_var*>{ &Va, &Vb }, true), w_pot);
	size_t c;
	for (size_t r = 0; r < Size; r++) {
		//create a new row of variables
		for (c = 1; c < Size; c++) {
			Categoric_var Y_att(var_size, "V" + to_string(r) + "_" + to_string(c));
			if (c == 1) {
				Categoric_var Y_prev(var_size, "V" + to_string(r) + "_0");
				Potential_Exp_Shape P_temp(P_ab, { &Y_prev , &Y_att });
				Mat->Insert(P_temp);
			}
			else {
				Potential_Exp_Shape P_temp(P_ab, { Mat->Find_Variable("V" + to_string(r) + "_" + to_string(c - 1)) , &Y_att });
				Mat->Insert(P_temp);
			}
		}

		if (r > 0) {
			//connect the new row to the previous one
			for (c = 0; c < Size; c++) {
				Categoric_var* Va = Mat->Find_Variable("V" + to_string(r) + "_" + to_string(c));
				Categoric_var* Vb = Mat->Find_Variable("V" + to_string(r - 1) + "_" + to_string(c));
				Potential_Exp_Shape P_temp(P_ab, { Va , Vb });
				Mat->Insert(P_temp);
			}
		}
	}

	return unique_ptr<Graph>(Mat);

}