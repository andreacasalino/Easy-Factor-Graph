#include <model/RandomField.h>
#include <Parser.h>
#include <iostream>
#include "../node/NodeFactoryXmlIO.h"
#include <algorithm>
using namespace std;

namespace EFG::model {

	RandomField::RandomField(const std::string& config_xml_file) :
		GraphLearnable(true) {
		EFG::node::importInfo readerInfo = EFG::node::createXmlReader(config_xml_file);
		if(nullptr != readerInfo.reader) {
			XmlStructureImporter strct(*readerInfo.reader, readerInfo.prefix);
			this->_Insert(strct.GetStructure(), true);
			this->_SetEvidences(strct.GetObservations());
		}
	};

	vector<float> RandomField::_GetBetaPart(const distr::Combinations& training_set) {

		this->SetEvidences(std::vector<std::pair<std::string, size_t>>{});

		this->_BeliefPropagation(true);

		vector<float> betas;
		betas.reserve(this->GetModelSize());
		auto L = this->_GetLearnerList();
		if (this->ThPool == nullptr) {
			std::for_each(L->begin(), L->end(), [&betas](LearningHandler* l) { betas.push_back(l->GetBetaPart()); });
		}
		else {
			std::for_each(L->begin(), L->end(), [&betas, this](LearningHandler* l) { 
				betas.push_back(0.f);
				float* pval = &betas.back();
				this->ThPool->push([pval, l]() { *pval = l->GetBetaPart(); });
			});
			this->ThPool->wait();
		}
		return betas;

	}

#define INSERT_SHARE \
	if (P != nullptr) {\
		vector<CategoricVariable*> vars_shared;\
		vars_shared.reserve(vars_of_pot_whose_weight_is_to_share.size());\
		for (size_t k = 0; k < vars_of_pot_whose_weight_is_to_share.size(); ++k) {\
			node::Node* temp = this->_FindNode(vars_of_pot_whose_weight_is_to_share[k]);\
			if (temp == nullptr) throw std::runtime_error("inexistent variable");\
			vars_shared.push_back(temp->GetVar());\
		}\
		this->_Share(vars_shared, pot.GetDistribution().GetVariables());\
	}

	void RandomField::Insert(pot::ExpFactor& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share) {

		auto P = this->GraphLearnable::_Insert(pot, true);
		INSERT_SHARE

	};

	void RandomField::InsertMove(pot::ExpFactor&& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share) {

		auto P = this->GraphLearnable::_Insert(std::move(pot), true);
		INSERT_SHARE

	};

}