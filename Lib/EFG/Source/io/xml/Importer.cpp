/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <io/xml/Importer.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/const/FactorExponential.h>
#include <distribution/factor/modifiable/FactorExponential.h>
#include <Parser.h>
#include <Error.h>
#include <algorithm>

namespace EFG::io::xml {
	const std::string* findAttribute(xmlPrs::Tag& tag, const std::string& attributeName) {
		auto it = tag.getAttributes().find(attributeName);
		if (it == tag.getAttributes().end()) {
			return nullptr;
		}
		return &it->second;
	};

	categoric::VariablePtr findVariable(const std::string& name, const std::set<categoric::VariablePtr>& variables) {
		auto itV = variables.find(categoric::makeVariable(2, name));
		if (itV == variables.end()) {
			throw Error("Inexistent variable");
		}
		return *itV;
	};

	categoric::Group importGroup(xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
		auto vars = tag.getAttributes().equal_range("var");
		if ((std::distance(vars.first, vars.second) != 1) && (std::distance(vars.first, vars.second) != 2)) {
			throw Error("only unary or binary factor are supported");
		}
		auto itV = vars.first;
		categoric::Group group(findVariable(itV->second, variables));
		++itV;
		for (itV; itV != vars.second; ++itV) {
			group.add(findVariable(itV->second, variables));
		}
		return group;
	};

	std::shared_ptr<distribution::factor::cnst::Factor> importFactor(const std::string& prefix, xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
		auto group = importGroup(tag, variables);

		const auto* corr = findAttribute(tag, "Correlation");
		if (nullptr != corr) {
			if (0 == corr->compare("T")) {
				return std::make_shared<distribution::factor::cnst::Factor>(group, true);
			}
			if (0 == corr->compare("F")) {
				return std::make_shared<distribution::factor::cnst::Factor>(group, false);
			}
			throw Error("invalid option for Correlation");
		}

		const auto* source = findAttribute(tag, "Source");
		if (nullptr != source) {
			return std::make_shared<distribution::factor::cnst::Factor>(group, prefix + "/" + *source);
		}

		std::shared_ptr<distribution::factor::modif::Factor> factor = std::make_shared<distribution::factor::modif::Factor>(group);
		auto itDistr = tag.getNested("Distr_val");
		std::vector<std::size_t> comb;
		for (auto it = itDistr.begin(); it != itDistr.end(); ++it) {
			auto itComb = it->second->getAttributes().equal_range("v");
			if (std::distance(itComb.first, itComb.second) != factor->getGroup().getVariables().size()) {
				throw Error("invalid combination");
			}
			comb.clear();
			comb.reserve(std::distance(itComb.first, itComb.second));
			for (auto itC = itComb.first; itC != itComb.second; ++itC) {
				comb.push_back(std::atoi(itC->second.c_str()));
			}
			auto val = findAttribute(*it->second, "D");
			if (nullptr == val) {
				throw Error("image value not found");
			}
			factor->add(Combination(comb), static_cast<float>(std::atof(val->c_str())));
		}
		return factor;
	};

	distribution::DistributionPtr importDistribution(const std::string& prefix, xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
		auto shape = importFactor(prefix, tag, variables);
		const auto* w = findAttribute(tag, "weight");
		if (nullptr == w) {
			return shape;
		}

		const auto* tunab = findAttribute(tag, "tunability");
		if (nullptr == tunab) {
			return std::make_shared<distribution::factor::cnst::FactorExponential>(*shape, static_cast<float>(std::atof(w->c_str())));
		}
		return std::make_shared<distribution::factor::modif::FactorExponential>(*shape, static_cast<float>(std::atof(w->c_str())));
	};

	std::map<std::string, std::size_t> Importer::importComponents(const std::string& filePath, const std::string& fileName, const std::pair<nodes::InsertCapable*, nodes::InsertTunableCapable*>& components) {
		xmlPrs::Parser parser(filePath + "/" + fileName);
		// import variables
		std::set<categoric::VariablePtr> variables;
		const auto& varTag = parser.getRoot().getNested("Variable");
		std::map<std::string, std::size_t> evidence;
		for (auto itV = varTag.begin(); itV != varTag.end(); ++itV) {
			const auto* name = findAttribute(*itV->second, "name");
			if (nullptr == name) {
				throw Error("variable name not found");
			}
			const auto* size = findAttribute(*itV->second, "Size");
			if (nullptr == size) {
				throw Error("variable size not found");
			}
			categoric::VariablePtr newVar = categoric::makeVariable(std::atoi(size->c_str()), *name);
			if (variables.find(newVar) != variables.end()) {
				throw Error("Found multiple variables with same name in the passed xml");
			}
			variables.emplace(newVar);
			const auto* flag = findAttribute(*itV->second, "flag");
			if ((nullptr != flag) && (0 == flag->compare("O"))) {
				evidence.emplace(newVar->name(), 0);
			}
		}
		// import potentials
		const auto& potTag = parser.getRoot().getNested("Potential");
		for (auto itP = potTag.begin(); itP != potTag.end(); ++itP) {
			auto distribution = importDistribution(filePath, *itP->second, variables);
			distribution::factor::modif::FactorExponential* expPtr = dynamic_cast<distribution::factor::modif::FactorExponential*>(distribution.get());
			if ((nullptr == expPtr) || (nullptr == std::get<1>(components)))  {
				// import as non tunable factor
				std::get<0>(components)->insertCopy(*distribution);
			}
			else {
				// tunable factor
				auto shareTag = itP->second->getNested("Share");
				if (shareTag.begin() == shareTag.end()) {
					std::get<1>(components)->insertTunableCopy(*expPtr);
				}
				else {
					std::get<1>(components)->insertTunableCopy(*expPtr, importGroup(*shareTag.begin()->second, variables));
				}
			}
		}		
		return evidence;
	}
}
