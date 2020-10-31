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

#include <model/Graph.h>
#include "../Utilities.h"
using namespace EFG;

unique_ptr<model::Graph> create_Matrix(const std::size_t& Size, const std::size_t& var_size, const float& w_pot);

int main() {

	vector<float> marginals;

	size_t Size = 10; // A matrix of Size x Size variables will be created (see 'Sample 05: Matricial structure' of the documentation), you can change this value
	size_t var_dom_size = 3; //you can change it
	float weight_potential = 1.0f;

	auto Matrix = create_Matrix(Size, var_dom_size, weight_potential);
	//save the file into an xml (just as an example)
	Matrix->Reprint(Get_prefix() + "Sample_05_graphs/Matrix");

	//set V0_0 = 0 as an edivence and compute marginals of the variables along the diagonal of the matrix
	CategoricVariable* V0 = Matrix->FindVariable("V0_0");
	Matrix->SetEvidences(vector<pair<string, size_t>>{{"V0_0" , 0}});

	for (size_t k = 1; k < Size; k++) {
		marginals = Matrix->GetMarginalDistribution("V" + to_string(k) + "_" + to_string(k));
		cout << "V" << k << "_" << k << "     ";
		print_distribution(marginals);
	}

	return 0;
}

unique_ptr<model::Graph> create_Matrix(const std::size_t& Size, const std::size_t& var_size, const float& w_pot) {

	if (Size < 2) abort();
	if (var_size < 2) abort();

	unique_ptr<model::Graph> Mat = make_unique<model::Graph>();

	//Create a correlating potential to replicate
	CategoricVariable Va(var_size, "Va"), Vb(var_size, "Vb");
	pot::ExpFactor P_ab(pot::Factor(vector<CategoricVariable*>{ &Va, &Vb }, true), w_pot);
	size_t c;
	for (size_t r = 0; r < Size; r++) {
		//create a new row of variables
		for (c = 1; c < Size; c++) {
			CategoricVariable Y_att(var_size, "V" + to_string(r) + "_" + to_string(c));
			if (c == 1) {
				CategoricVariable Y_prev(var_size, "V" + to_string(r) + "_0");
				Mat->InsertMove(pot::ExpFactor(P_ab, { &Y_prev , &Y_att }));
			}
			else  Mat->InsertMove(pot::ExpFactor(P_ab, { Mat->FindVariable("V" + to_string(r) + "_" + to_string(c - 1)) , &Y_att }));
		}

		if (r > 0) {
			//connect the new row to the previous one
			for (c = 0; c < Size; c++) {
				CategoricVariable* Va = Mat->FindVariable("V" + to_string(r) + "_" + to_string(c));
				CategoricVariable* Vb = Mat->FindVariable("V" + to_string(r - 1) + "_" + to_string(c));
				Mat->InsertMove(pot::ExpFactor(P_ab, { Va , Vb }));
			}
		}
	}

	return move(Mat);

}