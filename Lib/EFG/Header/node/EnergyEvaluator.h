/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_NODE_ENERGYEVAL_H__
#define __EFG_NODE_ENERGYEVAL_H__

#include <node/NodeFactory.h>
#include <algorithm>
#include <distribution/FullMatchFinder.h>

namespace EFG::node {

	class Node::NodeFactory::EnergyEvaluator : public sbj::Subject::Observer {
		friend class Node::NodeFactory;
	public:
		template<typename Array>
		float		 			  Eval(const Array& combination) const {

			float E = 1.f;
			for (auto it = this->Finders.begin(); it != this->Finders.end(); ++it) {
				auto val = (*it)(combination);
				if (val == nullptr) return 0.f;
				else E *= val->GetVal();
			}
			return E;

		};

		template<typename Array>
		std::vector<float>		  EvalNormalized(const std::vector<Array>& combinations) const {

			if (combinations.empty()) throw std::runtime_error("empty combinations vector");

			auto max_extractor = [](const distr::DiscreteDistribution& distr) {
				auto it = distr.getIter();
				if (!it.isNotAtEnd()) return 1.f;

				float max = it->GetVal(), att;
				++it;
				itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&max, &att](distr::DiscreteDistribution::constIterator& itt) {
					att = itt->GetVal();
					if (att > max) max = att;
				});
				return max;
			};

			std::vector<float> vals;
			vals.reserve(combinations.size());

			list<distr::DiscreteDistribution::constFullMatchFinder>::const_iterator itF;
			size_t f;
			std::vector<float> E_max;
			E_max.reserve(this->Potentials.size());
			std::for_each(this->Potentials.begin(), this->Potentials.end(), [&max_extractor, &E_max](const pot::IPotential* p) { E_max.push_back(1.f / max_extractor(p->GetDistribution())); });
			for (size_t c = 0; c < combinations.size(); ++c) {
				vals.push_back(1.f);
				f = 0;
				for (itF = this->Finders.begin(); itF != this->Finders.end(); ++itF) {
					auto val = (*itF)(combinations[c]);
					if (val == nullptr) {
						vals.back() = 0.f;
						break;
					}
					else vals.back() *= val->GetVal() * E_max[f];
					++f;
				}
			}
			return vals;

		};

		/*
		* \brief Returns an estimation of the likelihood of the model
		* \details considering a particular training set as reference:
		* P(model | train_set). This method is called by an I_Trainer during the gradient descend performed when training the model
		* @param[in] training_set the training set to consider, i.e. the series of samples, wrapped in a combinations object
		* @param[out] result estimation of the likelihood
		*/
		template<typename Array>
		float GetTrainingSetLikelihood(const std::vector<Array>& combinations) const {

			std::vector<float> L = this->EvalNormalized(combinations);
			float result = 0.f;
			float coeff = 1.f / (float)combinations.size();
			auto it_end = L.end();
			for (auto it = L.begin(); it != it_end; ++it)
				result += coeff * *it;
			return result;

		};

	private:
		EnergyEvaluator(const NodeFactory& factory, const std::vector<CategoricVariable*>& vars_order);
	// data
		std::list<const pot::IPotential*>							   Potentials;
		std::list<distr::DiscreteDistribution::constFullMatchFinder>   Finders;
	};

	Node::NodeFactory::EnergyEvaluator Node::NodeFactory::GetEnergyEvaluator(const std::vector<CategoricVariable*>& vars_order) const { return EnergyEvaluator(*this, vars_order); };


}

#endif