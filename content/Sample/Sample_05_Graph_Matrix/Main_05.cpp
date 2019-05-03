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


void create_xml(const string& name_file, const int& row_size, const int& col_size, const string& dyna_funct_shape_location);

int main(int argc, char *argv[]) {
	string prefix =  compute_prefix() + "Sample_05_Graph_Matrix" + "/";

	if (argc < 3) {
		system("ECHO matrix sizes were not passed as argument to the program");
		abort();
	}

	int row =  atoi(argv[1]);
	if (row < 3) {
		system("ECHO matrix rows must be at least 3");
		abort();
	}
	int col =  atoi(argv[2]);
	if (col < 3) {
		system("ECHO matrix columns must be at least 3");
		abort();
	}

	//generate the graph with the specified sizes: an xml file is created in the folder containing the project, which is later red
	create_xml(prefix +  "Matrix.xml", row, col, "binary_shape.txt");

	Graph Matrix_graph("Matrix.xml", prefix);
	cout << "matrix graph created \n";

	//get all the variables in the model in a matrix of references
	vector<vector<Categoric_var*>> Var;
	int r, c, k = 0;
	for (r = 0; r < row; r++) {
		Var.push_back(vector<Categoric_var*>());
		for (c = 0; c < col; c++) {
			Var.back().push_back(Matrix_graph.Find_Variable("v" + to_string(k)));
			k++;
		}
	}

	// assume the top left corner of the matrix as an observation and perform
	// a belief propagation across the rest of the model
	Matrix_graph.Set_Observation_Set_var({ Var[0][0] });
	Matrix_graph.Set_Observation_Set_val({ 1 });

	//print the marginals of all the other variables
	list<float> marginals;
	k = 0;
	for (r = 0; r < row; r++) {
		for (c = 0; c < col; c++) {
			if (k > 0) {
				Matrix_graph.Get_marginal_distribution(&marginals, Var[r][c]);
				cout << "P(" << Var[r][c]->Get_name() << "|v0=1)\n";
				print_distribution(marginals);
			}

			k++;
		}
	}


	system("pause");
	return 0;
}


void print_Potential(ofstream& f, const string& shape_location, const string& var1, const string& var2) {

	f << "  	<Potential var=\"" << var1 << "\" var=\"" << var2 << "\"  weight=\"1.0\"  Source=\"" << shape_location << "\"></Potential>\n";

}

void create_xml(const string& name_file, const int& row_size, const int& col_size, const string& dyna_funct_shape_location) {

	ofstream f(name_file);
	if (!f.is_open()) {
		cout << "not possible to open write stream \n";
		abort();
	}

	f << "<?xml version=\"1.0\"?>\n";
	f << "<Matrix>\n";

	vector<vector<string>> names;
	int r, c;
	int k = 0;
	names.reserve(row_size);
	for (r = 0; r < row_size; r++) {
		names.push_back(vector<string>());
		names.back().reserve(col_size);
		for (c = 0; c < col_size; c++) {
			names.back().push_back("v" + to_string(k));
			f << "	<Variable name=\"" << names.back().back() << "\" Size=\"2\"></Variable>\n";
			k++;
		}
	}


	for (r = 0; r < row_size; r++) {
		for (c = 0; c < col_size; c++) {
			if ((c + 1) < col_size)
				print_Potential(f, dyna_funct_shape_location, names[r][c], names[r][c + 1]);

			if ((r + 1) < row_size)
				print_Potential(f, dyna_funct_shape_location, names[r][c], names[r + 1][c]);
		}
	}

	f << "</Matrix>";

	f.close();

}
