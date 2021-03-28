#include <model/Learnable.h>
#include <algorithm>
#include "handler/UnaryHandler.h"
#include "handler/BinaryHandler.h"
#include <Error.h>
using namespace std;

namespace EFG::model {

	GraphLearnable::~GraphLearnable() {

		std::for_each(this->AtomicLearner.begin(), this->AtomicLearner.end(), [](AtomicHandler* a) { delete a; });

	}

	bool GraphLearnable::__Compare(const std::vector<CategoricVariable*>& set1, const std::vector<CategoricVariable*>& set2) {

		size_t S = set1.size();
		if (S != set2.size()) return false;

		if (S == 1) return (set1[0]->GetName().compare(set2[0]->GetName()) == 0);
		else {
			if ((set1[0]->GetName().compare(set2[0]->GetName()) == 0) && (set1[1]->GetName().compare(set2[1]->GetName()) == 0)) return true;
			if ((set1[1]->GetName().compare(set2[0]->GetName()) == 0) && (set1[0]->GetName().compare(set2[1]->GetName()) == 0)) return true;
		}
		return false;

	}

	std::list<GraphLearnable::AtomicHandler*>::iterator GraphLearnable::__FindInAtomics(const std::vector<CategoricVariable*>& vars) {

		for (auto it = this->AtomicLearner.begin(); it != this->AtomicLearner.end(); ++it) {
			if (__Compare((*it)->GetDistribution().GetVariables(), vars)) return it;
		}
		return this->AtomicLearner.end();

	}

	std::pair<std::list<GraphLearnable::CompositeHandler>::iterator, std::list<GraphLearnable::AtomicHandler*>::iterator> GraphLearnable::__FindInComposites(const std::vector<CategoricVariable*>& vars) {

		list<AtomicHandler*>* L;
		for (auto it = this->CompositeLearner.begin(); it != this->CompositeLearner.end(); ++it) {
			L = it->GetComponents();
			for (auto it2 = L->begin(); it2 != L->end(); ++it2) {
				if (__Compare((*it2)->GetDistribution().GetVariables(), vars)) return make_pair(it, it2);
			}
		}
		return make_pair(this->CompositeLearner.end(), this->AtomicLearner.end());

	}

#define INSERT_TUNABLE \
	if (pot_inserted == nullptr) return nullptr; \
	if (weight_tunability) {\
		if (pot_inserted->GetDistribution().GetVariables().size() == 1) {\
			this->_Add<UnaryHandler>(this, pot_inserted);\
		}\
		else {\
			this->_Add<BinaryHandler>(this, pot_inserted);\
		}\
	}\
	return pot_inserted;

	pot::ExpFactor* GraphLearnable::_Insert(pot::ExpFactor& pot, const bool& weight_tunability) {
		auto pot_inserted = this->NodeFactory::_Insert(pot);
		INSERT_TUNABLE
	}

	pot::ExpFactor* GraphLearnable::_Insert(pot::ExpFactor&& pot, const bool& weight_tunability) {
		auto pot_inserted = this->NodeFactory::_Insert(std::move(pot));
		INSERT_TUNABLE
	}

	void GraphLearnable::_Insert(const Structure& strct, const bool& useMove) {
		// also here a MACRO can be used, but will result in complicated structure difficult to debug
		class Inserter {
		public:
			Inserter(GraphLearnable& user) : user(&user) { };

			void Insert(const Structure* strct) {
				(*this)(get<0>(*strct));
				(*this)(get<2>(*strct));
				const vector<vector<pot::ExpFactor*>>& learnable_exp = get<1>(*strct);
				std::for_each(learnable_exp.begin(), learnable_exp.end(), [this](const vector<pot::ExpFactor*>& cluster) {
					auto it = cluster.begin();
					auto first_inserted = (*this)(*it);
					++it;
					if (it != cluster.end()) {
						const vector<CategoricVariable*>& vars_to_share = first_inserted->GetDistribution().GetVariables();
						std::for_each(it, cluster.end(), [this, &vars_to_share](pot::ExpFactor* e) {
							(*this)(e);
							this->user->_Share(e->GetDistribution().GetVariables(), vars_to_share);
						});
					}
				});
			}
		protected:
			virtual void operator()(const vector<pot::Factor*>& collection) = 0;
			virtual void operator()(const vector<pot::ExpFactor*>& untunable) = 0;
			virtual pot::ExpFactor* operator()(pot::ExpFactor* tunable) = 0;

			GraphLearnable* user;
		};

		if (useMove) {
			class MoveInserter : public Inserter {
			public:
				MoveInserter(GraphLearnable& user) : Inserter(user) {};
			private:
				void operator()(const vector<pot::Factor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::Factor* p) { this->user->node::Node::NodeFactory::_Insert(std::move(*p)); }); };
				void operator()(const vector<pot::ExpFactor*>& untunable) final { std::for_each(untunable.begin(), untunable.end(), [this](pot::ExpFactor* p) { this->user->_Insert(std::move(*p), false); }); };
				pot::ExpFactor* operator()(pot::ExpFactor* tunable) final { return this->user->_Insert(std::move(*tunable), true); };
			};
			MoveInserter(*this).Insert(&strct);
		}
		else {
			class NormalInserter : public Inserter {
			public:
				NormalInserter(GraphLearnable& user) : Inserter(user) {};
			private:
				void operator()(const vector<pot::Factor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::Factor* p) { this->user->node::Node::NodeFactory::_Insert(*p); }); };
				void operator()(const vector<pot::ExpFactor*>& untunable) final { std::for_each(untunable.begin(), untunable.end(), [this](pot::ExpFactor* p) { this->user->_Insert(*p, false); }); };
				pot::ExpFactor* operator()(pot::ExpFactor* tunable) final { return this->user->_Insert(*tunable, true); };
			};
			NormalInserter(*this).Insert(&strct);
		}

	}

	void GraphLearnable::_Share(const std::vector<CategoricVariable*>& set1, const std::vector<CategoricVariable*>& set2) {

		list<AtomicHandler*>::iterator a1, a2;
		std::pair<std::list<GraphLearnable::CompositeHandler>::iterator, std::list<GraphLearnable::AtomicHandler*>::iterator> c1, c2;

		a1 = this->__FindInAtomics(set1);
		c1 = this->__FindInComposites(set1);

		a2 = this->__FindInAtomics(set2);
		c2 = this->__FindInComposites(set2);

		if ((c1.first == this->CompositeLearner.end()) && (a1 == this->AtomicLearner.end())) throw Error("model::GraphLearnable", "inexistent handler");
		if ((c2.first == this->CompositeLearner.end()) && (a2 == this->AtomicLearner.end())) throw Error("model::GraphLearnable", "inexistent handler");

		if ((a1 != this->AtomicLearner.end()) && (a2 != this->AtomicLearner.end())) {
			this->CompositeLearner.emplace_back(*a1, *a2);
			this->AtomicLearner.erase(a1);
			this->AtomicLearner.erase(a2);
			this->LearnerList.remove(*a1);
			this->LearnerList.remove(*a2);
			this->LearnerList.push_back(&this->CompositeLearner.back());
			return;
		}

		if ((c1.first != this->CompositeLearner.end()) && (c2.first != this->CompositeLearner.end())) {
			auto el2 = c2.first->GetComponents();
			for (auto it = el2->begin(); it != el2->end(); ++it) c1.first->Append(*it);
			el2->clear();
			this->LearnerList.remove(&(*c2.first));
			this->CompositeLearner.erase(c2.first);
			return;
		}

		if (a1 != this->AtomicLearner.end()) {
			c2.first->Append(*a1);
			this->AtomicLearner.erase(a1);
			this->LearnerList.remove(*a1);
		}
		else {
			c1.first->Append(*a2);
			this->AtomicLearner.erase(a2);
			this->LearnerList.remove(*a2);
		}

	}

	std::vector<float> GraphLearnable::GetTunable() const {

		vector<float> w;
		w.reserve(this->GetModelSize());
		for (auto it : this->LearnerList) w.push_back(it->GetWeight());
		return w;

	}

	std::vector<float> GraphLearnable::GetTunableGrad(const distr::Combinations& training_set, const bool& force_alpha_recomputation) {

		bool recompute_alfa = false;
		if (force_alpha_recomputation) recompute_alfa = true;
		else {
			if (this->LastTrainingSetUsed != &training_set) recompute_alfa = true;
		}
		this->LastTrainingSetUsed = &training_set;

		vector<float> w_grad;
		w_grad.reserve(this->GetModelSize());

		if (recompute_alfa) {
			#ifdef THREAD_POOL_ENABLED
			if (this->ThPool != nullptr) {
				for (auto it : this->LearnerList) {
					this->ThPool->push([it, this]() {it->RecomputeAlfaPart(*this->LastTrainingSetUsed); });
				}
				this->ThPool->wait();
			}
			else {
			#endif
				for (auto it : this->LearnerList) it->RecomputeAlfaPart(*this->LastTrainingSetUsed);
			#ifdef THREAD_POOL_ENABLED
			}
			#endif
		}
		for (auto it : this->LearnerList) w_grad.push_back(it->GetAlfaPart());

		vector<float> beta_part = this->_GetBetaPart(training_set);
		size_t k, K = w_grad.size();
		for (k = 0; k < K; ++k)
			w_grad[k] -= beta_part[k];

		if (this->UseRegularization) {
			k = 0;
			for (auto it : this->LearnerList) {
				w_grad[k] -= 2.f * it->GetWeight();
				++k;
			}
		}
		return w_grad;

	}

	void GraphLearnable::SetTunable(const std::vector<float>& w_vector) {

		if (this->GetModelSize() != w_vector.size()) throw Error("model::GraphLearnable", "inconsistent vector of weights");
		size_t k = 0;
		for (auto it : this->LearnerList) {
			it->SetWeight(w_vector[k]);
			++k;
		}

	}

	node::Node::NodeFactory::Structure GraphLearnable::GetStructure() const {

		vector<pot::Factor*> sh;
		vector<pot::ExpFactor*> exp_sh;
		{
			auto srct = this->NodeFactory::GetStructure();
			sh = get<0>(srct);
			exp_sh = get<2>(srct);
		}


		vector<vector<pot::ExpFactor*>> tunab_clusters;
		unordered_set<pot::ExpFactor*>			S;

		tunab_clusters.reserve(this->AtomicLearner.size() + this->CompositeLearner.size());

		std::for_each(this->AtomicLearner.begin(), this->AtomicLearner.end(), [&tunab_clusters , &S](AtomicHandler* a) {
			tunab_clusters.emplace_back();
			tunab_clusters.back().push_back(a->GetWrappedExp());
			S.emplace(a->GetWrappedExp());
		});

		std::for_each(this->CompositeLearner.begin(), this->CompositeLearner.end(), [&tunab_clusters, &S](const CompositeHandler& c) {
			tunab_clusters.emplace_back();
			tunab_clusters.back().reserve(c.GetComponents()->size());
			std::for_each(c.GetComponents()->begin(), c.GetComponents()->end(), [&tunab_clusters, &S](AtomicHandler* a) {
				tunab_clusters.back().push_back(a->GetWrappedExp());
				S.emplace(a->GetWrappedExp());
			});
		});

		vector<pot::ExpFactor*> constant;
		constant.reserve(exp_sh.size() - S.size());
		std::for_each(exp_sh.begin(), exp_sh.end(), [&constant , &S](pot::ExpFactor* e) { if (S.find(e) == S.end()) constant.push_back(e); });

		return make_tuple(sh, tunab_clusters, constant);

	}

	std::list<GraphLearnable::AtomicHandler*>	 GraphLearnable::_GetAllHandlers() const {

		list<GraphLearnable::AtomicHandler*> L = this->AtomicLearner;
		for (auto it : this->CompositeLearner) {
			std::for_each(it.GetComponents()->begin(), it.GetComponents()->end(), [&L](AtomicHandler* a) { L.push_back(a); });
		}
		return L;

	}

}
