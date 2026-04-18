/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/model/Graph.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::structure;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

ModelSeed makeMatrix(std::size_t matrix_size, VarStateSize var_size,
                     float weight_correlation);

int main() {
  SAMPLE_SECTION(
      "Matrix like structure. After running the sample, check the content of "
      "the generated Matrix.xml, Matrix.xml",
      "4.5", [] {
        size_t matrix_size = 10; // A matrix of Size x Size variables will be
                                 // created, you can change it
        size_t var_dom_size = 3; // you can change it
        float weight_potential = 1.3f;

        auto &&seed = makeMatrix(matrix_size, var_dom_size, weight_potential);
        auto vars_names = seed.named;
        Graph Matrix{std::move(seed)};
        // set V0_0 = 0 as an edivence and compute marginals of the variables
        // along the diagonal of the matrix
        Matrix.setEvidences(Evidence{vars_names.at("V0_0"), 0});

        std::vector<float> prob;

        Matrix.setWorkersPoolSize(4);
        auto activated_threads_guard =
            Matrix.activatePool(); // scoped activation

        for (size_t k = 1; k < matrix_size; k++) {
          const std::string var_name = "V" + to_string(k) + "_" + to_string(k);
          cout << "Marginals of " << var_name << "  ";
          Matrix.getMarginalDistribution(prob, vars_names.at(var_name));
          cout << prob << endl;
        }

        auto export_path =
            std::filesystem::temp_directory_path() / "Matrix.json";
        Matrix.to_file(export_path);
        cout << "Check exported json at: " << export_path.string() << endl;
      });

  return EXIT_SUCCESS;
}

ModelSeed makeMatrix(std::size_t matrix_size, VarStateSize var_size,
                     float weight_correlation) {
  if (matrix_size < 2)
    throw EFG::Error("invalid matrix size");
  if (var_size < 2)
    throw EFG::Error("invalid variable size");

  // build the model
  ModelBuilder builder;

  // build the matrix of variables
  std::vector<std::vector<std::size_t>> vars;
  for (std::size_t row = 0; row < matrix_size; ++row) {
    auto &vars_row = vars.emplace_back();
    for (std::size_t col = 0; col < matrix_size; ++col) {
      vars_row.push_back(builder.make_variable(
          var_size, ModelBuilder::VariableDescription{
                        false, "V" + to_string(row) + "_" + to_string(col)}));
    }
  }

  auto connect_row_ = [&](std::size_t row) {
    for (std::size_t col = 1; col < matrix_size; ++col) {
      auto factor = FactorExponential<2>::from_sparse_domain_gen(
          {var_size, var_size}, SimplyCorrelatedDomainGen<2>{var_size});
      factor.trsfm.setWeight(weight_correlation);
      builder.add_binary_factor(std::move(factor), vars[row][col - 1],
                                vars[row][col]);
    }
  };

  connect_row_(0);

  for (std::size_t row = 1; row < matrix_size; ++row) {
    connect_row_(row);
    // connect this row to the previous one
    for (std::size_t col = 0; col < matrix_size; ++col) {
      auto factor = FactorExponential<2>::from_sparse_domain_gen(
          {var_size, var_size}, SimplyCorrelatedDomainGen<2>{var_size});
      factor.trsfm.setWeight(weight_correlation);
      builder.add_binary_factor(std::move(factor), vars[row - 1][col],
                                vars[row][col]);
    }
  }

  return ModelBuilder::build(std::move(builder));
}