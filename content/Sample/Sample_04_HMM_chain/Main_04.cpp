#include <iostream>
#include <string>
using namespace std;

#include "../../CRF/Header/Graphical_model.h"
#include "../Prefix_path.h"
#include "../Utilities.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG
#include <vector>

void create_HMM_as_xml(const string& name_file, const int& chain_size, const string& dyna_funct_shape_location);

int main(int argc, char *argv[]) {
	string prefix = compute_prefix() + "Sample_04_HMM_chain" + "/";

	if (argc < 2) {
		system("ECHO chain size was not passed as argument to the program. Maybe you have not launched from shell this example");
		abort();
	}

	list<float> marginals;
	int chain_size = atoi(argv[1]);
	cout << "Size of the HMM chain passed as input " << chain_size << endl;

	//generate chain with the specified size: an xml file is created in the folder containing the project, which is later red
	create_HMM_as_xml(prefix + "HMM.xml" , chain_size, "YY_dynamic.txt");

	Graph HMM("HMM.xml", prefix);
	cout << "HMM created \n";

	//get all the variables in the model
	list<Categoric_var*> X;
	vector<Categoric_var*> Y;
	for (int k = 0; k < chain_size; k++) {
		X.push_back(HMM.Find_Variable("X" + to_string(k + 1)));
		Y.push_back(HMM.Find_Variable("Y" + to_string(k + 1)));
	}

	//impose some random values for the observations variables (all the X)
	list<size_t> observed_val;
	for (int k = 0; k < chain_size; k++) {
		observed_val.push_back(rand() % 4);
	}
	HMM.Set_Observation_Set_var(X);
	HMM.Set_Observation_Set_val(observed_val);

	//get the marginal probabilities of all the hidden variables
	for (int k = 0; k < chain_size; k++) {
		HMM.Get_marginal_distribution(&marginals, Y[k]);
		cout << "P(" << Y[k]->Get_name() << "|X)\n";
		print_distribution(marginals);
	}

	system("pause");
	return 0;
}

void create_HMM_as_xml(const string& name_file, const int& chain_size, const string& dyna_funct_shape_location) {

	ofstream f(name_file);
	if (!f.is_open()) {
		cout << "not possible to open write stream \n";
		abort();
	}

	f << "<?xml version=\"1.0\"?>\n";
	f << "<HMM>\n";
	for (int k = 0; k < chain_size; k++) {
		f << "	<Variable name=\"Y" << to_string(k + 1) << "\" Size=\"4\"></Variable>\n";
		f << "	<Variable name=\"X" << to_string(k + 1) << "\" Size=\"4\"></Variable>\n";
	}

	f << "	<Potential var=\"Y1\" weight=\"1.0\">\n";
	f << "		<Distr_val v=\"1\" D=\"1\"></Distr_val>\n";
	f << "	</Potential>\n";

	for (int k = 0; k < chain_size; k++)
		f << "	<Potential var=\"X" << (k + 1) << "\" var=\"Y" << (k + 1) << "\" weight=\"1.0\" Source=\"XY_observations.txt\"></Potential>\n";
	for (int k = 1; k < chain_size; k++)
		f << "	<Potential var=\"Y" << (k + 1) << "\" var=\"Y" << k << "\" weight=\"1.0\" Source=\"YY_dynamic.txt\"></Potential>\n";

	f << "</HMM>";

	f.close();

}