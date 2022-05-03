/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/io/json/Exporter.h>
#include <EasyFactorGraph/io/xml/Exporter.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::io;
using namespace EFG::distribution;
using namespace EFG::categoric;
using namespace EFG::strct;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

Graph makeMatrix(const std::size_t &matrix_size, const std::size_t &var_size,
                 const float &weight_correlation);

int main() {
  SampleSection section(
      "Matrix like structure. After running the sample, check the content of "
      "the generated Matrix.xml, Matrix.xml",
      "4.5");

  size_t matrix_size = 10; // A matrix of Size x Size variables will be created,
                           // you can change it
  size_t var_dom_size = 3; // you can change it
  float weight_potential = 1.1f;

  auto Matrix = makeMatrix(matrix_size, var_dom_size, weight_potential);
  // set V0_0 = 0 as an edivence and compute marginals of the variables
  // along the diagonal of the matrix
  Matrix.setEvidence("V0_0", 0);

  std::size_t threads = 1;
  if (matrix_size > 5) {
    threads = 3; // use more threads to propagate the belief for big matrices
  }

  for (size_t k = 1; k < matrix_size; k++) {
    const std::string var_name = "V" + to_string(k) + "_" + to_string(k);
    cout << "Marginals of " << var_name << "  ";
    cout << Matrix.getMarginalDistribution(var_name, threads) << endl;
  }

  // save the file into an xml (just as an example)
  xml::Exporter::exportToFile(Matrix, xml::ExportInfo{"Matrix.xml", "Matrix"});
  // save the file into an xml (just as an example)
  json::Exporter::exportToFile(Matrix, "Matrix.json");

  return EXIT_SUCCESS;
}

Graph makeMatrix(const std::size_t &matrix_size, const std::size_t &var_size,
                 const float &weight_correlation) {
  if (matrix_size < 2)
    throw EFG::Error("invalid matrix size");
  if (var_size < 2)
    throw EFG::Error("invalid variable size");

  // build the matrix of variables
  std::vector<std::vector<VariablePtr>> variables;
  for (std::size_t row = 0; row < matrix_size; ++row) {
    auto &vars_row = variables.emplace_back();
    for (std::size_t col = 0; col < matrix_size; ++col) {
      vars_row.push_back(
          make_variable(var_size, "V" + to_string(row) + "_" + to_string(col)));
    }
  }

  // build the model
  Graph model;

  // Create a correlating potential to replicate
  auto A = make_variable(var_size, "placeholder");
  auto B = make_variable(var_size, "placeholder_bis");
  FactorExponential factor(
      Factor{Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG},
      weight_correlation);

  for (std::size_t row = 0; row < matrix_size; ++row) {
    for (std::size_t col = 1; col < matrix_size; ++col) {
      auto temp = std::make_shared<FactorExponential>(factor);
      temp->replaceVariables(
          VariablesSoup{variables[row][col - 1], variables[row][col]});
      model.addConstFactor(temp);
    }

    if (0 < row) {
      // connect this row to the previous one
      for (std::size_t col = 0; col < matrix_size; ++col) {
        auto temp = std::make_shared<FactorExponential>(factor);
        temp->replaceVariables(
            VariablesSoup{variables[row][col], variables[row - 1][col]});
        model.addConstFactor(temp);
      }
    }
  }

  return model;
}