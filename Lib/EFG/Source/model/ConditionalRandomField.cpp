#include <model/ConditionalRandomField.h>
#include <Parser.h>
#include <iostream>
#include "../node/NodeFactoryXmlIO.h"
#include <algorithm>
#include <set>
#include "handler/BinaryHandlerWithObservation.h"
using namespace std;

namespace EFG::model {

	ConditionalRandomField::ConditionalRandomField(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const node::bp::BeliefPropagator& propagator) :
		GraphLearnable(true, propagator) {

		std::unique_ptr<xmlPrs::Parser> reader;
		try { reader = std::make_unique<xmlPrs::Parser>(prefix_config_xml_file + config_xml_file); }
		catch (...) {
			cout << "warninig: file not readable in Random_Field construction" << endl;
			reader.reset();
		}
		if (reader != nullptr) {
			XmlStructureImporter strct(*reader.get(), prefix_config_xml_file);
			auto obsv = strct.GetObservations();
			std::vector<std::string> ev;
			vector<size_t> ev_val;
			ev.reserve(obsv.size());
			ev_val.reserve(obsv.size());
			std::for_each(obsv.begin(), obsv.end(), [&ev, &ev_val](auto o) {
				ev.push_back(o.first);
				ev_val.push_back(o.second);
			});
			this->_Import(strct.GetStructure(), true, ev);
			this->SetEvidences(ev_val);
		}

	};

	ConditionalRandomField::ConditionalRandomField(const Structure& strct, const std::vector<std::string>& observed_var, const bool& use_cloning_Insert, const node::bp::BeliefPropagator& propagator) :
		GraphLearnable(use_cloning_Insert, propagator) {

		this->_Import(strct, false, observed_var); //not sure that is always false that must be passed

	}

	ConditionalRandomField::ConditionalRandomField(const NodeFactory& o) : GraphLearnable(true, *_GetPropagator(o)) {

		auto obsv = o.GetObservationSet();
		vector<std::string> ev;
		vector<size_t> ev_val;
		ev.reserve(obsv.size());
		ev_val.reserve(obsv.size());
		std::for_each(obsv.begin(), obsv.end(), [&ev, &ev_val](auto o) {
			ev.push_back(o.first->GetName());
			ev_val.push_back(o.second);
		});

		this->_Import(o.GetStructure(), false, ev);
		this->SetEvidences(ev_val);

	};

	struct RedundantRemover {
		template<typename P>
		vector<P*> operator()(const vector<P*>& arr, const std::set<std::string>& ordered_evidences) {
			vector<P*> minimal;
			minimal.reserve(arr.size());
			size_t A = arr.size(), V;
			bool add;
			for (size_t a = 0; a < A; ++a) {
				add = true;
				const vector<CategoricVariable*>& vars = arr[a]->GetDistribution().GetVariables();
				V = vars.size();
				if (V == 1) {
					if (ordered_evidences.find(vars.front()->GetName()) != ordered_evidences.end()) add = false;
				}
				else if (V == 2) {
					if ((ordered_evidences.find(vars.front()->GetName()) != ordered_evidences.end()) && (ordered_evidences.find(vars.back()->GetName()) != ordered_evidences.end()))
						add = false;
				}
				if (add) minimal.push_back(arr[a]);
			}
			return minimal;
		};
	};
	void ConditionalRandomField::_Import(const Structure& strct, const bool& use_move, const std::vector<std::string>& evidences) {

		// import the minimal structure that excludes the potential fully connected to obeservatons
		std::set<std::string> ordered_evidences;
		for (auto it : evidences) ordered_evidences.insert(it);
		Structure strct_minimal;
		RedundantRemover Rm;
		get<0>(strct_minimal) = Rm(get<0>(strct), ordered_evidences);
		get<2>(strct_minimal) = Rm(get<2>(strct), ordered_evidences);
		get<1>(strct_minimal).reserve(get<1>(strct).size());
		for (size_t k = 0; k < get<1>(strct).size(); ++k) {
			vector<pot::ExpFactor*> temp = Rm(get<1>(strct)[k], ordered_evidences);
			if (!temp.empty()) get<1>(strct_minimal).emplace_back(move(temp));
		}
		this->_Insert(strct_minimal, use_move);

		//create set of observations and hidden vars
		if (evidences.empty()) cout << "Warning in when building a Conditional_Random_Field: empty evidences set" << endl;
		vector<pair<string, size_t>> obsv;
		obsv.reserve(evidences.size());
		for (size_t o = 0; o < evidences.size(); ++o) obsv.emplace_back(make_pair(evidences[o], 0));
		this->_SetEvidences(obsv);

		//replace handlers partially connected to observations
		list<AtomicHandler*> all_atomics = this->_GetAllHandlers();
		vector<size_t*> obsv_vals = { nullptr, nullptr };
		size_t K;
		for (auto it : all_atomics) {
			const vector<CategoricVariable*>& vars = it->GetDistribution().GetVariables();
			K = vars.size();
			if (K == 2) {
				obsv_vals[0] = this->_FindObservation(vars[0]->GetName());
				obsv_vals[1] = this->_FindObservation(vars[1]->GetName());

				if (obsv_vals[0] != nullptr)
					this->_Replace<BinaryHandlerWithObservation>(it->GetDistribution().GetVariables(), this->_FindNode(vars.back()->GetName()), obsv_vals[0]);
				else if (obsv_vals[1] != nullptr)
					this->_Replace<BinaryHandlerWithObservation>(it->GetDistribution().GetVariables(), this->_FindNode(vars.front()->GetName()), obsv_vals[1]);
			}
		}

		this->OrderTrainingSet = nullptr;
		this->posObservationsTrainingSet = nullptr;

	}

	vector<float> ConditionalRandomField::_GetBetaPart(const distr::Combinations& training_set) {

		vector<float> betas;
		size_t k, K = this->GetModelSize();
		betas.reserve(K);
		for (k = 0; k < K; ++k) betas.push_back(0.f);
		float coeff = 1.f / (float)training_set.size();

		//recompute pos_observations if needed
		if (&training_set.GetVariables() != this->OrderTrainingSet) {
			this->OrderTrainingSet = &training_set.GetVariables();

			map<CategoricVariable*, size_t>  obs_map;
			const vector<CategoricVariable*>& train_set_vars = training_set.GetVariables();
			K = train_set_vars.size();
			for (k = 0; k < K; ++k) obs_map.emplace(train_set_vars[k], k);

			auto obs_temp = this->GetObservationSet();
			K = obs_temp.size();
			if (this->posObservationsTrainingSet == nullptr) this->posObservationsTrainingSet = new size_t[K];
			this->posObservationsSize = K;
			for (k = 0; k < K; ++k) this->posObservationsTrainingSet[k] = obs_map.find(obs_temp[k].first)->second;
		}

		vector<size_t> observations;
		observations.reserve(this->posObservationsSize);
		K = this->GetModelSize();
		auto it = training_set.getIter();

		itr::forEach<distr::Combinations::constIterator>(it, [this, &observations, &betas, &coeff, &k](distr::Combinations::constIterator& itt) {
			observations.clear();
			for (k = 0; k < this->posObservationsSize; ++k) observations.push_back((*itt)[this->posObservationsTrainingSet[k]]);

			this->_SetEvidences(observations);
			this->_BeliefPropagation(true);

			k = 0;
			if (this->ThPool == nullptr) {
				std::for_each(this->_GetLearnerList()->begin(), this->_GetLearnerList()->end(), [&betas, &coeff, &k](LearningHandler* h) {
					betas[k] += coeff * h->GetBetaPart();
					++k;
				});
			}
			else {
				std::for_each(this->_GetLearnerList()->begin(), this->_GetLearnerList()->end(), [&betas, &coeff, &k, this](LearningHandler* h) {
					float* pval = &betas[k];
					this->ThPool->push([pval, &coeff, h]() { *pval += coeff * h->GetBetaPart(); });
					++k;
				});
				this->ThPool->wait();
			}
		});

		return betas;

	}

}