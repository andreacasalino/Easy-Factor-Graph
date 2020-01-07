/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 04: Hidden Markov model like structure' of the documentation

#include <iostream>
#include <string>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
#include "../Utilities.h"
using namespace EFG;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/EFG.lib")
#else
#pragma comment (lib, "../../x64/Release/EFG.lib")
#endif // DEBUG

Graph* create_Chain(const size_t& Chain_size, const size_t& var_size, const float& w_XY, const float& w_YY);

int main() {

	size_t chain_size = 10; //you can change it
	size_t var_dom_size = 2; //you can change it

	list<size_t> Y_MAP;

	//create a chain with a strong weight on the potentials XY. Evidences are set as indicated in Sample 04 
	auto G_XY = create_Chain(chain_size, var_dom_size, 5.f, 0.5f);
	//compute MAP on hidden variables and display it
	G_XY->MAP_on_Hidden_set(&Y_MAP);
	cout << "Strong correlation with edivences,   MAP on Y0,1,..   ";
	print_distribution(Y_MAP);

	//create a chain with a strong weight on the potentials YY. Evidences are set as indicated in Sample 04
	auto G_YY = create_Chain(chain_size, var_dom_size, 0.5f, 5.f);
	//compute MAP on hidden variables and display it
	G_YY->MAP_on_Hidden_set(&Y_MAP);
	cout << "Strong correlation among hidden variables,   MAP on Y0,1,..   ";
	print_distribution(Y_MAP);

	//export chains into an xml (just as example)
	G_XY->Reprint("Graph_XY");
	G_YY->Reprint("Graph_YY");

	delete G_XY;
	delete G_YY;

	return 0;
}

Graph* create_Chain(const size_t& Chain_size, const size_t& var_size, const float& w_XY, const float& w_YY) {

	if (Chain_size < 2) abort();
	if (var_size < 2) abort();

	//build the chain structure described in 'Sample 04: Hidden Markov model like structure'

	Categoric_var X_fake(var_size, "X_fake");
	Categoric_var Y_fake(var_size, "Y_fake");
	Categoric_var Y2_fake(var_size, "Y_fake2");
	Potential_Exp_Shape P_XY(new Potential_Shape({ &X_fake, &Y_fake }, true), w_XY);
	Potential_Exp_Shape P_YY(new Potential_Shape({ &Y2_fake, &Y_fake }, true), w_YY);

	Graph* G = new Graph(); //all the potentials are interanlly cloned, see constructor definition

	list<Categoric_var*>  Evidences;
	for (size_t k = 0; k < Chain_size; k++) {
		Categoric_var X_k(X_fake.size(), "X_" + to_string(k));
		Categoric_var Y_k(X_fake.size(), "Y_" + to_string(k));
		Potential_Exp_Shape temp_XY(&P_XY, { &X_k, &Y_k });
		G->Insert(&temp_XY);
		if (k == 0) {
			Potential_Shape* p_Y = new Potential_Shape({ &Y_k });
			p_Y->Add_value({ 0 }, 1.f);
			Potential_Exp_Shape P_Y(p_Y, w_YY);
			G->Insert(&P_Y);
		}
		else {
			Potential_Exp_Shape temp_YY(&P_YY, { G->Find_Variable("Y_" + to_string(k - 1)), &Y_k });
			G->Insert(&temp_YY);
		} 
		Evidences.push_back(G->Find_Variable("X_" + to_string(k)));
	}

	//set the value of the evidences equal to:
	//X_0 = 0, X_1=var_size-1, X_2= 0, X_3 = var_size-1, etc.. 
	list<size_t> values;
	size_t o = 0;
	for (size_t k = 0; k < Chain_size; k++) {
		values.push_back(o);
		if (o == 0) o = 1;
		else o = 0;
	}
	G->Set_Evidences(Evidences, values);

	return G;

};