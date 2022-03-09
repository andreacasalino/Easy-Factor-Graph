/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <io/json/Exporter.h>
#include <distribution/DistributionIterator.h>
#include "JSONstream.h"
#include <algorithm>
#include <fstream>

namespace EFG::io::json {
	void printGroup(const categoric::Group& group, structJSON& recipient, const std::string& fieldName = "variables") {
		arrayJSON vars;
		std::for_each(group.getVariables().begin(), group.getVariables().end(), [&vars](const categoric::VariablePtr& v) {
			vars.addElement(String(v->name()));
		});
		recipient.addElement(fieldName, vars);
	};

	structJSON printPotential(const distribution::Distribution& distr) {
		structJSON potential;
		printGroup(distr.getGroup(), potential);
		auto it = distr.getIterator();
		arrayJSON values;
		iterator::forEach(it, [&values](const distribution::DistributionIterator& it) {
			arrayJSON v;
			for (std::size_t k = 0; k < it.getCombination().size(); ++k) v.addElement(Number<std::size_t>(it.getCombination().data()[k]));
			values.addElement(v);
		});
		potential.addElement("values", values);
		return potential;
	};

	void Exporter::exportComponents(const std::string& filePath, const std::string& modelName, const std::tuple<const strct::EvidenceAware*, const strct::StructureAware*, const strct::StructureTunableAware*>& components) {
		structJSON exporter;
		std::string name = modelName;
		if (name.empty()) {
			name = "graphical-model";
		}
		exporter.addElement("name" , String(name));
		arrayJSON variables;
		// hidden set
		auto H_vars = std::get<0>(components)->getHiddenVariables();
		std::for_each(H_vars.begin(), H_vars.end(), [&variables](const categoric::VariablePtr& v) {
			structJSON var;
			var.addElement("name", String(v->name()));
			var.addElement("Size", Number<std::size_t>(v->size()));
			variables.addElement(var);
		});
		// evidence set
		auto O_vars = std::get<0>(components)->getEvidences();
		for (auto itO = O_vars.begin(); itO != O_vars.end(); ++itO) {
			structJSON var;
			var.addElement("name", String(itO->first->name()));
			var.addElement("Size", Number<std::size_t>(itO->first->size()));
			var.addElement("flag", String("O"));
			variables.addElement(var);
		}
		arrayJSON potentials;
		// factors
		auto factors = std::get<1>(components)->getConstFactors();
		std::for_each(factors.begin(), factors.end(), [&potentials](const std::shared_ptr<distribution::factor::cnst::Factor>& f) {
			potentials.addElement(printPotential(*f));
		});
		// exp const factors
		auto factorsExp = std::get<1>(components)->getConstFactorsExp();
		std::for_each(factorsExp.begin(), factorsExp.end(), [&potentials](const std::shared_ptr<distribution::factor::cnst::FactorExponential>& f) {
			auto pot = printPotential(*f);
			pot.addElement("weight", Number<float>(f->getWeight()));
			pot.addElement("tunability", String("N"));
			potentials.addElement(pot);
		});
		if (nullptr != std::get<2>(components)) {
			// exp tunable factors
			auto factorsExp = std::get<2>(components)->getFactorsExp();
			std::for_each(factorsExp.begin(), factorsExp.end(), [&potentials](const std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>& cluster) {
				auto itCl = cluster.begin();
				auto pot = printPotential(**itCl);
				pot.addElement("weight", Number<float>((*itCl)->getWeight()));
				potentials.addElement(pot);
				const auto& varsFront = (*itCl)->getGroup();
				++itCl;
				std::for_each(itCl, cluster.end(), [&potentials, &varsFront](const std::shared_ptr<distribution::factor::modif::FactorExponential>& f) {
					auto pot = printPotential(*f);
					pot.addElement("weight", Number<float>(f->getWeight()));
					printGroup(varsFront, pot, "share");
					potentials.addElement(pot);
				});
			});
		}
		std::ofstream f(filePath);
		if (!f.is_open()) {
			throw Error("invalid filePath");
		}
		exporter.addElement("variables" , variables);
		exporter.addElement("factors", potentials);
		f << exporter.str();
	}
}
