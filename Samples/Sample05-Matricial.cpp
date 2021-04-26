/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <model/Graph.h>
#include <io/xml/Exporter.h>
#include <io/json/Exporter.h>
#include <print/ProbabilityDistributionPrint.h>
#include <Presenter.h>
#include <math.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::io;

unique_ptr<model::Graph> makeMatrix(const std::size_t& Size, const std::size_t& var_size, const float& w_pot);

int main() {
	EFG::sample::samplePart([]() {
		size_t Size = 10; // A matrix of Size x Size variables will be created
		size_t var_dom_size = 3; //you can change it
		float weight_potential = 1.1f;

		auto Matrix = makeMatrix(Size, var_dom_size, weight_potential);
		//set V0_0 = 0 as an edivence and compute marginals of the variables along the diagonal of the matrix
		Matrix->resetEvidences(std::map<std::string, std::size_t>{ {"V0_0", 0} });

		for (size_t k = 1; k < Size; k++) {
			cout << Matrix->getMarginalDistribution("V" + to_string(k) + "_" + to_string(k)) << endl;
		}

		//save the file into an xml (just as an example)
		io::xml::Exporter::exportToXml(*Matrix, "Matrix.xml", "Matrix");
		//save the file into an xml (just as an example)
		io::json::Exporter::exportToJson(*Matrix, "Matrix.json", "Matrix");
	}, "Matricial structure", "refer to Section 4.5 of the documentation", "Check the content of the created Matrix.xml and  Matrix.json");
	
	return EXIT_SUCCESS;
}

unique_ptr<model::Graph> makeMatrix(const std::size_t& Size, const std::size_t& var_size, const float& w_pot) {
	if (Size < 2) throw Error("invalid matrix size");
	if (var_size < 2) throw Error("invalid variable size");

	unique_ptr<model::Graph> Mat = make_unique<model::Graph>();

	auto makeMatrixVariable = [&var_size](std::size_t row, std::size_t col) {
		return makeVariable(var_size, "V" + to_string(row) + "_" + to_string(col));
	};

	//Create a correlating potential to replicate
	factor::cnst::FactorExponential P_ab(factor::cnst::Factor(std::set<categoric::VariablePtr>{makeVariable(var_size, "Va"), makeVariable(var_size, "Vb")}, true), w_pot);
	size_t c;
	for (size_t r = 0; r < Size; r++) {
		//create a new row of variables
		for (c = 1; c < Size; c++) {
			VariablePtr Vprev;
			VariablePtr Vatt = makeMatrixVariable(r , c);
			if (c == 1) {
				Vprev = makeMatrixVariable(r, 0);
			}
			else {
				Vprev = makeMatrixVariable(r, c - 1);
			}
			factor::modif::FactorExponential temp(P_ab);
			temp.replaceGroup(categoric::Group(Vprev, Vatt));
			Mat->insertCopy(temp);
		}

		if (r > 0) {
			//connect the new row to the previous one
			for (c = 0; c < Size; c++) {
				VariablePtr Va = makeMatrixVariable(r, c);
				VariablePtr Vb = makeMatrixVariable(r-1, c);
				factor::modif::FactorExponential temp(P_ab);
				temp.replaceGroup(categoric::Group(Va, Vb));
				Mat->insertCopy(temp);
			}
		}
	}

#ifdef THREAD_POOL_ENABLED
	if (Size > 5) {
		Mat->setThreadPoolSize(3);
	}
#endif
	return move(Mat);

}