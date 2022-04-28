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
#include <EasyFactorGraph/structure/SpecialFactors.h>

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

Graph make_graph_chain(const std::size_t &chain_size,
                       const std::size_t &var_size, const float &weight_XY,
                       const float &weight_YY);

int main() {
  SampleSection section(
      "Simple polytree belief propagation, check the content of the created "
      "Graph_XY.xml, Graph_YY.xml  Graph_XY.json and Graph_YY.json, ",
      "4.4");

  size_t chain_size = 10;  // you can change it
  size_t var_dom_size = 2; // you can change it

  vector<size_t> Y_MAP;
  Y_MAP.reserve(chain_size);

  {
    // create a chain with a strong weight on the potentials XY.
    auto G_XY = make_graph_chain(chain_size, var_dom_size, 2.f, 0.5f);
    // compute MAP on hidden variables and display it
    for (size_t k = 0; k < chain_size; ++k) {
      Y_MAP.push_back(G_XY.getMAP("Y_" + to_string(k)));
    }
    cout << "Strong correlation with edivences,   MAP on Y0,1,..   " << Y_MAP
         << endl;

    // export chains into an xml (just as an exporting example)
    xml::Exporter::exportToFile(G_XY,
                                xml::ExportInfo{"Graph_XY.xml", "Graph_XY"});
    // export chains into a json (just as an exporting example)
    json::Exporter::exportToFile(G_XY, "Graph_XY.json");
  }

  {
    // create a chain with a strong weight on the potentials YY.
    auto G_YY = make_graph_chain(chain_size, var_dom_size, 0.5f, 2.f);
    // compute MAP on hidden variables and display it
    Y_MAP.clear();
    for (size_t k = 0; k < chain_size; ++k) {
      Y_MAP.push_back(G_YY.getMAP("Y_" + to_string(k)));
    }
    cout << "Strong correlation among hidden variables,   MAP on Y0,1,..   "
         << Y_MAP << endl;

    // export chains into an xml (just as an exporting example)
    xml::Exporter::exportToFile(G_YY,
                                xml::ExportInfo{"Graph_YY.xml", "Graph_YY"});
    // export chains into a json (just as an exporting example)
    json::Exporter::exportToFile(G_YY, "Graph_YY.json");
  }

  return EXIT_SUCCESS;
}

Graph make_graph_chain(const std::size_t &chain_size,
                       const std::size_t &var_size, const float &weight_XY,
                       const float &weight_YY) {
  if (chain_size < 2)
    throw EFG::Error("invalid chain size");
  if (var_size < 2)
    throw EFG::Error("invalid variable size");

  Graph G;

  auto X = make_variable(var_size, "X_placeholder");
  auto Y = make_variable(var_size, "Y_placeholder");
  auto Ybis = make_variable(var_size, "Y_placeholder_bis");

  FactorExponential P_YY(
      Factor{Group{VariablesSoup{Y, Ybis}}, USE_SIMPLE_CORRELATION_TAG},
      weight_YY);

  FactorExponential P_XY(
      Factor{Group{VariablesSoup{Y, X}}, USE_SIMPLE_CORRELATION_TAG},
      weight_XY);

  // build the chain and set the value of the evidences equal to:
  // X_0 = 0, X_1=var_size-1, X_2= 0, X_3 = var_size-1, etc..
  VariablesSoup Y_vars, X_vars;
  for (size_t k = 0; k < chain_size; k++) {
    Y_vars.push_back(make_variable(var_size, "Y_" + to_string(k)));
    X_vars.push_back(make_variable(var_size, "X_" + to_string(k)));
    auto temp_XY = std::make_shared<FactorExponential>(P_XY);
    temp_XY->replaceVariables(VariablesSoup{X_vars[k], Y_vars[k]});
    G.addConstFactor(temp_XY);
  }
  for (size_t k = 0; k < chain_size; k++) {
    DistributionCnstPtr factor;
    if (0 == k) {
      factor = std::make_shared<Indicator>(Y_vars[0], 0);
    } else {
      auto temp_YY = std::make_shared<FactorExponential>(P_YY);
      temp_YY->replaceVariables(VariablesSoup{Y_vars[k], Y_vars[k - 1]});
      factor = temp_YY;
    }
    G.addConstFactor(factor);
  }
  size_t o = 0;
  for (size_t k = 0; k < chain_size; k++) {
    G.setEvidence(X_vars[k], o);
    if (o == 0)
      o = 1;
    else
      o = 0;
  }
  return G;
}
