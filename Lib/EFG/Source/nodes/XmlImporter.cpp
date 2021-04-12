///**
// * Author:    Andrea Casalino
// * Created:   01.01.2021
// *
// * report any bug to andrecasa91@gmail.com.
// **/
//
//#include <nodes/XmlImporter.h>
//#include <nodes/bases/InsertTunableCapable.h>
//#include <distribution/factor/modifiable/Factor.h>
//#include <distribution/factor/const/FactorExponential.h>
//#include <distribution/factor/modifiable/FactorExponential.h>
//#include <Parser.h>
//#include <Error.h>
//#include <algorithm>
//
//namespace EFG::nodes {
//	const std::string* findAttribute(xmlPrs::Tag& tag, const std::string& attributeName) {
//		auto it = tag.getAttributes().find(attributeName);
//		if (it == tag.getAttributes().end()) {
//			return nullptr;
//		}
//		return &it->second;
//	};
//
//	categoric::VariablePtr findVariable(const std::string& name, const std::set<categoric::VariablePtr>& variables) {
//		auto itV = variables.find(categoric::makeVariable(2, name));
//		if (itV == variables.end()) {
//			throw Error("Inexistent variable");
//		}
//		return *itV;
//	};
//
//	categoric::Group importGroup(xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
//		auto vars = tag.getAttributes().equal_range("var");
//		if ((std::distance(vars.first, vars.second) != 1) && (std::distance(vars.first, vars.second) != 2)) {
//			throw Error("only unary or binary factor are supported");
//		}
//		auto itV = vars.first;
//		categoric::Group group(findVariable(itV->second, variables));
//		++itV;
//		for (itV; itV != vars.second; ++itV) {
//			group.add(findVariable(itV->second, variables));
//		}
//		return group;
//	};
//
//	std::shared_ptr<distribution::factor::cnst::Factor> importFactor(const std::string& prefix, xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
//		auto group = importGroup(tag, variables);
//
//		const auto* corr = findAttribute(tag, "Correlation");
//		if (nullptr != corr) {
//			if (0 == corr->compare("T")) {
//				return std::make_shared<distribution::factor::cnst::Factor>(group, true);
//			}
//			if (0 == corr->compare("F")) {
//				return std::make_shared<distribution::factor::cnst::Factor>(group, false);
//			}
//			throw Error("invalid option for Correlation");
//		}
//
//		const auto* source = findAttribute(tag, "Source");
//		if (nullptr != source) {
//			return std::make_shared<distribution::factor::cnst::Factor>(group, prefix + "/" + *source);
//		}
//
//		std::shared_ptr<distribution::factor::modif::Factor> factor = std::make_shared<distribution::factor::modif::Factor>(group);
//		auto itDistr = tag.getNested("Distr_val");
//		for (auto it = itDistr.begin(); it != itDistr.end(); ++it) {
//			auto itComb = it->second->getAttributes().equal_range("v");
//			if (std::distance(itComb.first, itComb.second) != factor->getGroup().getVariables().size()) {
//				throw Error("invalid combination");
//			}
//			std::vector<std::size_t> comb;
//			comb.reserve(std::distance(itComb.first, itComb.second));
//			for (auto itC = itComb.first; itC != itComb.second; ++itC) {
//				comb.push_back(std::atoi(itC->second.c_str()));
//			}
//			auto val = findAttribute(*it->second, "D");
//			if (nullptr == val) {
//				throw Error("image value not found");
//			}
//			factor->add(comb, static_cast<float>(std::atof(val->c_str())));
//		}
//		return factor;
//	};
//
//	distribution::DistributionPtr importDistribution(const std::string& prefix, xmlPrs::Tag& tag, const std::set<categoric::VariablePtr>& variables) {
//		auto shape = importFactor(prefix, tag, variables);
//		const auto* w = findAttribute(tag, "weight");
//		if (nullptr == w) {
//			return shape;
//		}
//
//		const auto* tunab = findAttribute(tag, "tunability");
//		if (nullptr == tunab) {
//			return std::make_shared<distribution::factor::cnst::FactorExponential>(*shape, static_cast<float>(std::atof(w->c_str())));
//		}
//		return std::make_shared<distribution::factor::modif::FactorExponential>(*shape, static_cast<float>(std::atof(w->c_str())));
//	};
//
//    void XmlImporter::importFromXml(const std::string& path, const std::string& file) {
//		xmlPrs::Parser data(path + "/" + file);
//		//import variables
//		std::set<categoric::VariablePtr> variables;
//		const auto& varTag = data.getRoot().getNested("Variable");
//		std::map<categoric::VariablePtr, std::size_t> evidence;
//		for (auto itV = varTag.begin(); itV != varTag.end(); ++itV) {
//			const auto& name = itV->second->getAttributes().find("name")->second;
//			categoric::VariablePtr newVar = categoric::makeVariable(std::atoi(itV->second->getAttributes().find("Size")->second.c_str()), name);
//			if (variables.find(newVar) != variables.end()) {
//				throw Error("Found multiple variables with same name in the xml");
//			}
//			variables.emplace(newVar);
//			auto flag = itV->second->getAttributes().find("flag");
//			if (flag != itV->second->getAttributes().end()) {
//				if (flag->second.compare("O") == 0) {
//					evidence.emplace(newVar, 0);
//				}
//			}
//		}
//
//		//import potential
//		std::list<std::shared_ptr<distribution::factor::modif::FactorExponential>> tunable;
//		std::list<std::pair<categoric::Group, std::shared_ptr<distribution::factor::modif::FactorExponential>>> sharingTunable;
//		const auto& potTag = data.getRoot().getNested("Potential");
//		for (auto itP = potTag.begin(); itP != potTag.end(); ++itP) {
//			auto distribution = importDistribution(path, *itP->second, variables);
//			distribution::factor::modif::FactorExponential* expPtr = dynamic_cast<distribution::factor::modif::FactorExponential*>(distribution.get());
//			if (nullptr == expPtr) {
//				// normal factor
//				this->Insert(distribution);
//			}
//			else {
//				// exponential factor
//				std::shared_ptr<distribution::factor::modif::FactorExponential> temp;
//				temp.reset(expPtr);
//				auto shareTag = itP->second->getNested("Share");
//				if (shareTag.begin() == shareTag.end()) {
//					tunable.emplace_back(temp);
//				}
//				else {
//					auto shareGroup = importGroup(*shareTag.begin()->second, variables);
//					sharingTunable.emplace_back(shareGroup, temp);
//				}
//			}
//		}
//		// check whether is possible to insert exponential factors
//		nodes::InsertTunableCapable* insertTunabPtr = dynamic_cast<nodes::InsertTunableCapable*>(this);
//		if (nullptr == insertTunabPtr) {
//			std::for_each(tunable.begin(), tunable.end(), [this](const std::shared_ptr<distribution::factor::modif::FactorExponential>& f) {
//				this->Insert(f);
//			});
//			std::for_each(sharingTunable.begin(), sharingTunable.end(), [this](const std::pair<categoric::Group, std::shared_ptr<distribution::factor::modif::FactorExponential>>& f) {
//				this->Insert(f.second);
//			});
//		}
//		else {
//			std::for_each(tunable.begin(), tunable.end(), [&insertTunabPtr](const std::shared_ptr<distribution::factor::modif::FactorExponential>& f) {
//				insertTunabPtr->Insert(f);
//			});
//			std::for_each(sharingTunable.begin(), sharingTunable.end(), [&insertTunabPtr](const std::pair<categoric::Group, std::shared_ptr<distribution::factor::modif::FactorExponential>>& f) {
//				insertTunabPtr->Insert(f.second, f.first);
//			});
//		}
//    }
//}
