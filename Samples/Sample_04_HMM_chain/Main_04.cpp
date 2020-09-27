/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 04: Hidden Markov model like structure' of the documentation

#include <iostream>
#include <string>
#include <memory>
using namespace std;

#include <Graphical_model.h>
#include "../Utilities.h"
using namespace EFG;

unique_ptr<Graph> create_Chain(const size_t& Chain_size, const size_t& var_size, const float& w_XY, const float& w_YY);

int main() {

	size_t chain_size = 10; //you can change it
	size_t var_dom_size = 2; //you can change it

	vector<size_t> Y_MAP;
	Y_MAP.reserve(chain_size);

	//create a chain with a strong weight on the potentials XY. Evidences are set as indicated in Sample 04 
	auto G_XY = create_Chain(chain_size, var_dom_size, 2.f, 0.5f);
	//compute MAP on hidden variables and display it
	for (size_t k = 0; k < chain_size; ++k) Y_MAP.push_back(G_XY->Get_MAP("Y_" + to_string(k)));
	cout << "Strong correlation with edivences,   MAP on Y0,1,..   ";
	print_distribution(Y_MAP);

	//create a chain with a strong weight on the potentials YY. Evidences are set as indicated in Sample 04
	auto G_YY = create_Chain(chain_size, var_dom_size, 0.5f, 2.f);
	//compute MAP on hidden variables and display it
	Y_MAP.clear();
	for (size_t k = 0; k < chain_size; ++k) Y_MAP.push_back(G_YY->Get_MAP("Y_" + to_string(k)));
	cout << "Strong correlation among hidden variables,   MAP on Y0,1,..   ";
	print_distribution(Y_MAP);

	//export chains into an xml (just as an exporting example)
	G_XY->Reprint(Get_prefix() + "Sample_04_graphs/Graph_XY");
	G_YY->Reprint(Get_prefix() + "Sample_04_graphs/Graph_YY");

	return 0;
}

unique_ptr<Graph> create_Chain(const size_t& Chain_size, const size_t& var_size, const float& w_XY, const float& w_YY) {

	if (Chain_size < 2) abort();
	if (var_size < 2) abort();

	//build the chain structure described in 'Sample 04: Hidden Markov model like structure'

	Categoric_var X_fake(var_size, "X_fake");
	Categoric_var Y_fake(var_size, "Y_fake");
	Categoric_var Y2_fake(var_size, "Y_fake2");
	//create the potentials to replicate in the chain
	Potential_Exp_Shape P_XY(Potential_Shape(vector<Categoric_var*>{ &X_fake, &Y_fake }, true), w_XY);
	Potential_Exp_Shape P_YY(Potential_Shape(vector<Categoric_var*>{ &Y2_fake, & Y_fake }, true), w_YY);

	Graph* G = new Graph(); //all the potentials are interanlly cloned, see constructor definition

	//build the chain and set the value of the evidences equal to:
	//X_0 = 0, X_1=var_size-1, X_2= 0, X_3 = var_size-1, etc.. 
	vector<pair<string, size_t>> Evidences;
	Evidences.reserve(Chain_size);
	size_t o = 0;
	for (size_t k = 0; k < Chain_size; k++) {
		Categoric_var X_k(X_fake.size(), "X_" + to_string(k));
		Categoric_var Y_k(X_fake.size(), "Y_" + to_string(k));
		Potential_Exp_Shape temp_XY(P_XY, { &X_k, &Y_k });
		G->Insert(temp_XY);
		if (k == 0) {
			Potential_Shape p_Y(vector<Categoric_var*>{ &Y_k });
			p_Y.Add_value(vector<size_t>{0}, 1.f);
			Potential_Exp_Shape P_Y(p_Y, w_YY);
			G->Insert(P_Y);
		}
		else {
			Potential_Exp_Shape temp_YY(P_YY, { G->Find_Variable("Y_" + to_string(k - 1)), &Y_k });
			G->Insert(temp_YY);
		}
		if (o == 0) o = 1;
		else o = 0;
		Evidences.emplace_back(make_pair("X_" + to_string(k), o));
	}
	G->Set_Evidences(Evidences);

	return unique_ptr<Graph>(G);

};