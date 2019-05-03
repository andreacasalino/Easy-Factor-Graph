#include <iostream>
#include <string>
using namespace std;

#include "../../CRF/Header/Potential.h"
#include "../Prefix_path.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG



void Print_Tableau(const list<Potential*>& shapes, const string& name_file);

int main() {
	///////////////////////////////////
	//			part 01				 //
	///////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";

	string prefix = compute_prefix() + "Sample_01_Potential_handling" + "/";


	//degfine a categorical variable, specifying its domain
	Categoric_var V(6, "V");

	//define a shape function involving variable V
	Potential_Shape* Phi1 = new Potential_Shape({ &V });
	// add some random values to the distribution
	for (size_t k = 0; k < 3; k++)
		Phi1->Add_value({ k }, (float)rand() / (float)RAND_MAX);
	cout << "Phi1 \n";
	Phi1->Print_distribution(cout);
	cout << endl << endl << endl;


	//define an exponential potential wrapping the previous shape function
	Potential_Exp_Shape Psi1(Phi1); //a weight W equal to 1 is assumed, i.e. Psi(x) = exp(W*Phi(x)) = exp(Phi(x))
	cout << "Psi1 \n";
	Psi1.Print_distribution(cout);
	cout << endl << endl << endl;


	//define two other variables
	Categoric_var V2(4, "V2");
	Categoric_var V3(5, "V3");

	//read from file a shape function involving V,V2 and V3
	Potential_Shape* Phi2 = new Potential_Shape({ &V,&V2 ,&V3 });
	Phi2->Import(prefix + "Ternary_Shape_function.txt");
	cout << "Phi2 \n";
	Phi2->Print_distribution(cout);
	cout << endl << endl << endl;

	//define an exponential potential wrapping the previous shape function
	Potential_Exp_Shape Psi2(Phi2, 2.f); //a weight W equal to 2 is assumed, i.e. Psi(x) = exp(W*Phi(x)) = exp(2*Phi(x))
	cout << "Psi2 \n";
	Psi2.Print_distribution(cout);
	cout << endl << endl << endl;

	//define a shape function with all values in the domain equal to 1 
	Potential_Shape Phi3({ &V, &V2 });
	Phi3.Set_ones();
	cout << "Phi3 \n";
	Phi3.Print_distribution(cout);
	cout << endl << endl << endl;

	system("pause");


	///////////////////////////////////
	//			part 02				 //
	///////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";

	int Dom_size = 4;
	int Numb_shapes = 4;

	// create Numb_shapes random unary shape
	Categoric_var VV((size_t)Dom_size, "VV");
	list<Potential*> shapes;
	for (int k = 0; k < Numb_shapes; k++) {
		Potential_Shape* temp_shape = new Potential_Shape({ &VV });
		temp_shape->Set_random(0.7f);
		shapes.push_back(new Potential(temp_shape));
	}
	//print the unary shape sampled in a ordered tableau
	Print_Tableau(shapes, prefix + "Tableau.xls");
	string temp_comm = "\"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Microsoft Office\\Microsoft Excel 2010.lnk\" \"" + compute_prefix() + "Sample_01_Potential_handling/Tableau.xls\"";
	ofstream f_bat("comm_temp.bat");
	f_bat << temp_comm;
	f_bat.close();
	system("comm_temp.bat");
	system("CLS");
	system("DEL comm_temp.bat");

	//compute a unary potential which is the product of the unary shapes computed above
	Potential shape_product(shapes);
	shape_product.Print_distribution(cout);
	cout << endl;

	//clean up
	for (auto it = shapes.begin(); it != shapes.end(); it++)
		delete *it;


	system("pause");
	return 0;
}

void Print_Tableau(const list<Potential*>& shapes, const string& name_file) {

	ofstream f(name_file);
	if (!f.is_open()) {
		system("ECHO unable to write Tableau");
		abort();
	}

	Categoric_var* V = shapes.front()->Get_involved_var_safe()->front();
	size_t V_size = V->size();

	list<size_t*> val_to_search;
	for (size_t k = 0; k < V_size; k++) {
		val_to_search.push_back((size_t*)malloc(sizeof(size_t)));
		val_to_search.back()[0] = k;
	}

	list<list<float>> Vals;
	for (auto it = shapes.begin(); it != shapes.end(); it++) {
		Vals.push_back(list<float>());
		(*it)->Find_Comb_in_distribution(&Vals.back(), val_to_search, { V });
	}

	float prod;
	auto it = Vals.begin();
	list<list<float>>::iterator it_distr;
	for (size_t k = 0; k < V_size; k++) {
		f << k << "\t\t";

		prod = 1.f;
		for (it_distr = Vals.begin(); it_distr != Vals.end(); it_distr++) {
			prod *= it_distr->front();

			f << it_distr->front() << "\t";
			it_distr->pop_front();
		}

		f << "\t\t" << prod << endl;
	}

	f.close();

};