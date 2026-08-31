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

std::unique_ptr<Graph> make_graph_chain(std::size_t chain_size,
                                        VarStateSize var_size, float weight_XY,
                                        float weight_YY);

int main() {
  SAMPLE_SECTION(
      "Chain like structure. After running the sample, check the content of "
      "the generated Graph_XY.xml, Graph_YY.xml, Graph_XY.json and "
      "Graph_YY.json ",
      "4.4", [] {
        size_t chain_size = 10;  // you can change it
        size_t var_dom_size = 2; // you can change it

        vector<size_t> Y_MAP;
        Y_MAP.reserve(chain_size);

        {
          // create a chain with a strong weight on the potentials XY.
          auto G_XY = make_graph_chain(chain_size, var_dom_size, 2.f, 0.5f);
          // compute MAP on hidden variables and display it
          for (size_t k = 0; k < chain_size; ++k) {
            Y_MAP.push_back(G_XY->getMAP(k));
          }
          cout << "Strong correlation with evidences,   MAP on Y0,1,..   "
               << Y_MAP << endl;

          // export into an xml (just as an exporting example)
          auto export_path =
              std::filesystem::temp_directory_path() / "Graph_XY.json";
          G_XY->to_file(export_path);
          cout << "Check exported json at: " << export_path.string() << endl;
        }

        {
          // create a chain with a strong weight on the potentials YY.
          auto G_YY = make_graph_chain(chain_size, var_dom_size, 0.5f, 2.f);
          // compute MAP on hidden variables and display it
          Y_MAP.clear();
          for (size_t k = 0; k < chain_size; ++k) {
            Y_MAP.push_back(G_YY->getMAP(k));
          }
          cout << "Strong correlation among hidden variables,   MAP on Y0,1,.. "
                  "  "
               << Y_MAP << endl;

          // export into an xml (just as an exporting example)
          auto export_path =
              std::filesystem::temp_directory_path() / "Graph_YY.json";
          G_YY->to_file(export_path);
          cout << "Check exported json at: " << export_path.string() << endl;
        }
      });

  return EXIT_SUCCESS;
}

std::unique_ptr<Graph> make_graph_chain(std::size_t chain_size,
                                        VarStateSize var_size, float weight_XY,
                                        float weight_YY) {
  if (chain_size < 2)
    throw EFG::Error("invalid chain size");
  if (var_size < 2)
    throw EFG::Error("invalid variable size");

  ModelBuilder builder;

  std::vector<std::size_t> Y_vars, X_vars;
  for (std::size_t k = 0; k < chain_size; ++k) {
    X_vars.push_back(builder.make_variable(var_size));
  }
  for (std::size_t k = 0; k < chain_size; ++k) {
    Y_vars.push_back(builder.make_variable(var_size));
  }
  for (std::size_t k = 0; k < chain_size; ++k) {
    auto factor = make_exp_simply_correlated<2>(var_size, weight_XY);
    builder.add_binary_factor(std::move(factor), X_vars[k], Y_vars[k]);
  }
  for (std::size_t k = 1; k < chain_size; ++k) {
    auto factor = make_exp_simply_correlated<2>(var_size, weight_YY);
    builder.add_binary_factor(std::move(factor), Y_vars[k - 1], Y_vars[k]);
  }

  auto graph = std::make_unique<Graph>(ModelBuilder::build(std::move(builder)));

  std::vector<Evidence> evidences;
  size_t o = 0;
  for (size_t k = 0; k < chain_size; k++) {
    if (k % 2) {
      // = 1
      evidences.emplace_back(Evidence{X_vars[k], 1});
    } else {
      // = 0
      evidences.emplace_back(Evidence{X_vars[k], 0});
    }
  }
  graph->setEvidences(evidences);

  return graph;
}
