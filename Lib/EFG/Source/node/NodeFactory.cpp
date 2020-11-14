#include <node/NodeFactory.h>
#include <node/belprop/BeliefPropagator.h>
#include "NeighbourConnection.h"
#include <algorithm>
#include <iostream>
using namespace std;

namespace EFG::node {

	Node::NodeFactory::~NodeFactory() {

		this->PotentialObservers.clear();

		list<CategoricVariable*> var_to_delete;
		if (this->bDestroyPotentials_and_Variables) {
			for (auto it = this->BinaryPotentials.get_map()->begin(); it != this->BinaryPotentials.get_map()->end(); ++it)  delete* it->second;

			for (auto itN = this->Nodes.get_map()->begin(); itN != this->Nodes.get_map()->end(); ++itN) {
				for (auto itU = itN->second->PermanentUnary.begin(); itU != itN->second->PermanentUnary.end(); ++itU) delete* itU;

				var_to_delete.push_back(itN->second->GetVar());
			}
		}
		this->Nodes.clear();

		for (auto itV = var_to_delete.begin(); itV != var_to_delete.end(); ++itV) delete* itV;

	}

	class NamePred {
	public:
		NamePred(const bool& flag) : use_val_or_ref(flag) {};

		bool operator()(const string* a, const string* b) const {
			if (this->use_val_or_ref) return (a->compare(*b) == 0);
			else {
				if (a == b) return true;
				return (a->compare(*b) == 0);
			}
		};
	private:
		bool 								use_val_or_ref;
	};

	class NameHasher {
		std::hash<string> hasher;
	public:
		inline size_t operator()(const string* k) const { return this->hasher(*k); }
	};

	class BiNamePred {
	public:
		BiNamePred(const bool& flag) : pred(flag) {};

		bool operator()(const std::pair<const string*, const string*>& a,
			const std::pair<const string*, const string*>& b) const {

			if (this->pred(a.first, b.first) && this->pred(a.second, b.second)) return true;
			if (this->pred(a.first, b.second) && this->pred(a.second, b.first)) return true;
			return false;

		};
	private:
		NamePred                   	    pred;
	};

	class BiNameHasher {
		std::hash<string> hasher;
	public:
		size_t operator()(const std::pair<const string*, const string*>& k) const {
			size_t hash1 = this->hasher(*k.first);
			size_t hash2 = this->hasher(*k.second);
			if (hash1 < hash2) return hash1;
			else		      return hash2;
		}
	};

	Node::NodeFactory::ObsvContainer::ObsvContainer() :
		univocal_map<Node*, std::pair<Node*, size_t>, std::function<Node * (std::pair<Node*, size_t>&)> >([](std::pair<Node*, size_t>& val) { return val.first; }) {}

	Node::NodeFactory::NodeFactory(const bool& use_cloning_Insert, const bp::BeliefPropagator& propagator) :
		bDestroyPotentials_and_Variables(use_cloning_Insert), PropagationMaxIter(1000), Propagator(move(propagator.copy())),
		Observations(),
		Nodes([](Node& n) { return &n.GetVar()->GetName(); }, NameHasher(), NamePred(use_cloning_Insert)),
		BinaryPotentials([](const pot::IPotential* p) {
							const vector<CategoricVariable*>& vars = p->GetDistribution().GetVariables();
							return make_pair(&vars[0]->GetName(), &vars[1]->GetName()); }, BiNameHasher(), BiNamePred(use_cloning_Insert)) {
		this->_SetEvidences(std::vector<std::pair<std::string, size_t>>{});
	};

	CategoricVariable* Node::NodeFactory::FindVariable(const std::string& var_name) const {

		auto it = this->Nodes.get_map()->find(&var_name);
		if (it == this->Nodes.get_map()->end()) return nullptr;
		else return it->second->GetVar();

	}

	Node* Node::NodeFactory::_FindNode(const std::string& var_name) {

		auto it = this->Nodes.get_map()->find(&var_name);
		if (it == this->Nodes.get_map()->end()) return nullptr;
		else return it->second;

	}

	std::vector<CategoricVariable*> Node::NodeFactory::GetHiddenSet() const {

		size_t K = 0;
		std::for_each(this->HiddenClusters.begin(), this->HiddenClusters.end(), [&K](const unordered_set<Node*>& s) { K += s.size(); });

		vector<CategoricVariable*> vars;
		vars.reserve(K);

		auto vars_adder = [&vars](const unordered_set<Node*>& s) { std::for_each(s.begin(), s.end(), [&vars](Node* n) { vars.push_back(n->GetVar()); }); };
		std::for_each(this->HiddenClusters.begin(), this->HiddenClusters.end(), [&vars_adder](const unordered_set<Node*>& s) {  vars_adder(s); });

		return vars;

	}

	std::vector<std::pair<CategoricVariable*, size_t>> Node::NodeFactory::GetObservationSet() const {

		const std::vector<Node*>& order = this->Observations.getOrder();
		size_t K = order.size();
		vector<std::pair<CategoricVariable*, size_t>> obsv;
		obsv.reserve(order.size());

		for (size_t k = 0; k < K; ++k) {
			auto it = this->Observations.get_map()->find(order[k]);
			obsv.emplace_back(move(make_pair(it->first->GetVar(), it->second->second)));
		}
		return obsv;

	};

	std::vector<CategoricVariable*> Node::NodeFactory::GetObservationSetVars() const {

		const std::vector<Node*>& order = this->Observations.getOrder();
		size_t K = order.size();
		vector<CategoricVariable*> vars;
		vars.reserve(K);

		for (size_t k = 0; k < K; ++k) {
			auto it = this->Observations.get_map()->find(order[k]);
			vars.push_back(it->first->GetVar());
		}
		return vars;

	}

	std::vector<CategoricVariable*> Node::NodeFactory::GetAllVariables() const {

		std::vector<CategoricVariable*> vars;
		vars.reserve(this->Nodes.get_map()->size());
		for (auto it = this->Nodes.get_map()->begin(); it != this->Nodes.get_map()->end(); ++it) vars.push_back(it->second->GetVar());
		return vars;

	}

	Node::NodeFactory::Structure Node::NodeFactory::GetStructure() const {

		vector<pot::Factor*> S;
		S.reserve(this->__SimpleShapes.size());
		std::for_each(this->__SimpleShapes.begin(), this->__SimpleShapes.end(), [&S](pot::Factor* f) { S.push_back(f); });

		vector<pot::ExpFactor*> E;
		E.reserve(this->__ExpShapes.size());
		std::for_each(this->__ExpShapes.begin(), this->__ExpShapes.end(), [&E](pot::ExpFactor* e) { E.push_back(e); });

		return make_tuple(S, std::vector<std::vector<pot::ExpFactor*>>{}, E);

	}

	template<typename P>
	P* Node::NodeFactory::__Insert(P* to_insert, const bool& use_move) {
		if (use_move) {
			if (!this->bDestroyPotentials_and_Variables) throw std::runtime_error("no need to move potential that would not be copied");
		}

		class NodeHndl {
			NodeFactory* Source;
		public:
			NodeHndl(NodeFactory* source) : Source(source) {};

			Node* Find(CategoricVariable* var) {
				auto node = this->Source->_FindNode(var->GetName());
				if (node != nullptr) {
					if ((!this->Source->bDestroyPotentials_and_Variables) && (node->GetVar() != var)) {
						throw std::runtime_error("when using non cloning insertion, you must refer to exactly the same variables already in the model");
					}
				}
				return node;
			};
			Node* Create(CategoricVariable* var) {
				if (this->Source->bDestroyPotentials_and_Variables) this->Source->Nodes.emplace_with_builder(*this, var, false);
				else 												this->Source->Nodes.emplace_with_builder(*this, var, true);
				return this->Source->_FindNode(var->GetName());
			};
			Node* operator()(CategoricVariable* v, const bool& f) const { return new Node(v, f); };
		};
		NodeHndl Hndl(this);

		P* inserted = nullptr;
		const std::vector<CategoricVariable*>& var_involved = to_insert->GetDistribution().GetVariables();
		if (var_involved.size() == 1) {
			//unary potential insertion
			Node* node = Hndl.Find(var_involved.front());
			if (node == nullptr)  node = Hndl.Create(var_involved.front());

			inserted = to_insert;
			if (this->bDestroyPotentials_and_Variables) {
				if (use_move) {
					auto tempPtr = createMoving(std::move(*to_insert));
					inserted = tempPtr.get();
					tempPtr.release();
					inserted->SubstituteVariables(std::vector<CategoricVariable*>{node->GetVar()});
				}
				else inserted = new P(*to_insert, std::vector<CategoricVariable*>{node->GetVar()});
			}
			node->PermanentUnary.push_back(inserted);
		}

		else if (var_involved.size() == 2) {

			//binary potential insertion
			Node* peer_A = Hndl.Find(var_involved.front());
			Node* peer_B = Hndl.Find(var_involved.back());

			if ((peer_A != nullptr) && (peer_B != nullptr)) {
				// check whether this binary potential was already present
				auto it_B = this->BinaryPotentials.get_map()->find(std::make_pair(&peer_A->GetVar()->GetName(), &peer_B->GetVar()->GetName()));
				if (it_B != this->BinaryPotentials.get_map()->end()) throw std::runtime_error("found clone of an already inserted binary potential");
			}
			else {
				if (peer_A == nullptr)  peer_A = Hndl.Create(var_involved.front());
				if (peer_B == nullptr)  peer_B = Hndl.Create(var_involved.back());
			}

			inserted = to_insert;
			if (this->bDestroyPotentials_and_Variables) {
				if (use_move) {
					auto tempPtr = createMoving(std::move(*to_insert));
					inserted = tempPtr.get();
					tempPtr.release();
					inserted->SubstituteVariables(std::vector<CategoricVariable*>{peer_A->GetVar(), peer_B->GetVar()});
				}
				else inserted = new P(*to_insert, std::vector<CategoricVariable*>{peer_A->GetVar(), peer_B->GetVar()});
			}
			this->BinaryPotentials.emplace(inserted);

			//create connection
			Node::NeighbourConnection::initConnection(peer_A, peer_B, *inserted);
		}

		else throw std::runtime_error("Only binary or unary potential can be inserted");

		this->PotentialObservers.emplace_back(*inserted->GetAsSubject());

		//update active connections
		{
			const vector<Node*>& obs_order = this->Observations.getOrder();
			std::vector<std::pair<std::string, size_t>> obs;
			obs.reserve(obs_order.size());
			std::for_each(obs_order.begin(), obs_order.end(), [this, &obs](Node* n) { obs.emplace_back(make_pair(n->GetVar()->GetName(), this->Observations.get_map()->find(n)->second->second)); });
			this->_SetEvidences(obs);
		}

		return inserted;

	};

#define INSERT_WITH_CHECK(POT_T, INSERT_FUNCTION, POT_NAME , POT_GROUP) \
	POT_T* temp = nullptr; \
	try { temp = INSERT_FUNCTION(POT_NAME); } \
	catch (...) { \
		temp = nullptr; \
		cout << "warning: invalid potential to insert detected\n"; \
	} \
	if (temp != nullptr) this->POT_GROUP.push_back(temp); 

#define INSERT_NORMAL(POT_NAME) this->__Insert(&POT_NAME)

#define INSERT_MOVE(POT_NAME) this->__Insert(&POT_NAME, true)

	void Node::NodeFactory::_Insert(pot::Factor& shape) {
		INSERT_WITH_CHECK(pot::Factor, INSERT_NORMAL, shape, __SimpleShapes)
	}

	void Node::NodeFactory::_Insert(pot::Factor&& shape) {
		INSERT_WITH_CHECK(pot::Factor, INSERT_MOVE, shape, __SimpleShapes)
	}

	pot::ExpFactor* Node::NodeFactory::_Insert(pot::ExpFactor& exp_shape) {
		INSERT_WITH_CHECK(pot::ExpFactor, INSERT_NORMAL, exp_shape, __ExpShapes)
		return temp;
	}

	pot::ExpFactor* Node::NodeFactory::_Insert(pot::ExpFactor&& exp_shape) {
		INSERT_WITH_CHECK(pot::ExpFactor, INSERT_MOVE, exp_shape, __ExpShapes)
		return temp;
	}

	void Node::NodeFactory::_Insert(const Structure& strct, const bool& useMove) {
	// also here a MACRO can be used, but will result in complicated structure difficult to debug
		class Inserter {
		public:
			Inserter(NodeFactory& user) : user(&user) { };

			void Insert(const Structure* strct) {
				(*this)(get<0>(*strct));
				(*this)(get<2>(*strct));
				std::for_each(get<1>(*strct).begin(), get<1>(*strct).end(), [this](const vector<pot::ExpFactor*>& c) { (*this)(c); });
			}
		protected:
			virtual void operator()(const vector<pot::Factor*>& collection) = 0;
			virtual void operator()(const vector<pot::ExpFactor*>& collection) = 0;

			NodeFactory* user;
		};

		if (useMove) {
			class MoveInserter : public Inserter {
			public:
				MoveInserter(NodeFactory& user) : Inserter(user) {};
			private:
				void operator()(const vector<pot::Factor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::Factor* p) { this->user->_Insert(std::move(*p)); }); };
				void operator()(const vector<pot::ExpFactor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::ExpFactor* p) { this->user->_Insert(std::move(*p)); }); };
			};
			MoveInserter(*this).Insert(&strct);
		}
		else {
			class NormalInserter : public Inserter {
			public:
				NormalInserter(NodeFactory& user) : Inserter(user) {};
			private:
				void operator()(const vector<pot::Factor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::Factor* p) { this->user->_Insert(*p); }); };
				void operator()(const vector<pot::ExpFactor*>& collection) final { std::for_each(collection.begin(), collection.end(), [this](pot::ExpFactor* p) { this->user->_Insert(*p); }); };
			};
			NormalInserter(*this).Insert(&strct);
		}

	}

	void Node::NodeFactory::SetIteration4Propagation(const unsigned int& iter_to_use) {

		unsigned int temp = iter_to_use;
		if (temp < 10) temp = 10;
		this->PropagationMaxIter = temp;

	}

	void Node::NodeFactory::_BeliefPropagation(const bool& sum_or_MAP) {

		this->LastPropagation.reset();
		this->__BeliefPropagation(sum_or_MAP);

	}

	void Node::NodeFactory::__BeliefPropagation(const bool& sum_or_MAP) {

		if (this->Nodes.get_map()->empty()) {
			cout << "warning: asked belief propagation on an empty structure\n";
			return;
		}
		
		if (nullptr != this->LastPropagation) {
			if (this->LastPropagation->WasSum_or_MAP == sum_or_MAP) return;
		}

		//reset all messages for nodes in the hidden set
		auto resetter = [](Node* n) { std::for_each(n->GetActiveConnections()->begin(), n->GetActiveConnections()->end(), [](Node::NeighbourConnection* c) { c->ResetOutgoingMessage(); }); };
		std::for_each(this->HiddenClusters.begin(), this->HiddenClusters.end(), [&resetter](unordered_set<Node*>& cluster) {
			std::for_each(cluster.begin(), cluster.end(), resetter);
		});

		//perform belief propagation
		bool all_done_within_iter = true;
		all_done_within_iter = (*this->Propagator)(this->HiddenClusters, sum_or_MAP, this->PropagationMaxIter, this->ThPool.get());

		if (this->LastPropagation == nullptr) this->LastPropagation = make_unique<PropagationInfo>();
		this->LastPropagation->WasSum_or_MAP = sum_or_MAP;
		this->LastPropagation->TerminatedWithinIter = all_done_within_iter;

	}

	void Node::NodeFactory::ObsvContainer::setVars(const std::vector<Node*>& vars) {

		this->map.clear();
		size_t K = vars.size();
		for (size_t k = 0; k < K; ++k) this->emplace(move(make_pair(vars[k], 0)));
		this->order = vars;

	}

	void Node::NodeFactory::ObsvContainer::setVals(const std::vector<size_t>& vals) {

		size_t K = this->order.size();
		if (vals.size() != K) throw std::runtime_error("inconsistent number of edivences");
		for (size_t k = 0; k < K; ++k) {
			auto it = this->map.find(this->order[k]);
			if (order[k]->GetVar()->size() <= vals[k]) throw std::runtime_error("inconsistent number of edivences");
			it->second->second = vals[k];
		}

	}

	void Node::NodeFactory::_SetEvidences(const std::vector<std::pair<std::string, size_t>>& new_observations) {

		//reactivate all connections
		auto reactivator = [](Node* n) { 
			std::for_each(n->DisabledConnections.begin(), n->DisabledConnections.end(), [n](NeighbourConnection* c) { n->ActiveConnections.push_back(c); });  
			n->DisabledConnections.clear();
		};
		std::for_each(this->Nodes.get_map()->begin(), this->Nodes.get_map()->end(), [&reactivator](auto n) { reactivator(n.second); });

		vector<Node*> Vars;
		Vars.reserve(new_observations.size());
		vector<size_t> Vals;
		Vals.reserve(new_observations.size());

		for (size_t kv = 0; kv < new_observations.size(); ++kv) {
			auto it_var = this->Nodes.get_map()->find(&new_observations[kv].first);
			if (it_var != this->Nodes.get_map()->end()) {
				auto conn = &it_var->second->ActiveConnections;
				while (!conn->empty()) {
					conn->front()->Disable();
				}

				Vars.push_back(it_var->second);
				Vals.push_back(new_observations[kv].second);
			}
		}
		this->Observations.setVars(Vars);

		this->__RecomputeClusters();

		this->_SetEvidences(Vals);

		if (Vars.size() != new_observations.size()) throw std::runtime_error("detected at least one variable not present in this graph");
	}

	void Node::NodeFactory::_SetEvidences(const std::vector<size_t>& new_observations) {

		this->Observations.setVals(new_observations);

		this->LastPropagation.reset();

		//delete all previous temporary messages
		std::for_each(this->Nodes.get_map()->begin(), this->Nodes.get_map()->end(), [](auto n) { n.second->TemporaryUnary.clear(); });

		//recompute temporary messages
		auto tempmex_computator = [](Node* n, const std::size_t& ob) {
			std::for_each(n->DisabledConnections.begin(), n->DisabledConnections.end(), [n, &ob](Node::NeighbourConnection* c) { 
				c->GetNeighbour()->TemporaryUnary.emplace_back(std::vector<size_t>{ob}, std::vector<CategoricVariable*>{n->GetVar()}, *c->GetPot());
			});
		};
		std::for_each(this->Observations.get_map()->begin(), this->Observations.get_map()->end(), [&tempmex_computator](auto o) { tempmex_computator(o.second->first, o.second->second); });

	}

	void Node::NodeFactory::__RecomputeClusters() {

		class cluster_finder {
			std::list<std::unordered_set<Node*>>* clusters;
		public:
			cluster_finder(std::list<std::unordered_set<Node*>>* cl) : clusters(cl) {};
			std::list<std::unordered_set<Node*>>::iterator operator()(Node* to_find) {
				for (auto itH = this->clusters->begin(); itH != this->clusters->end(); ++itH) {
					auto it = itH->find(to_find);
					if (it != itH->end()) return itH;
				}
				return this->clusters->end();
			};
		} finder(&this->HiddenClusters);

		auto merger = [](std::unordered_set<Node*>* destination, std::unordered_set<Node*>* to_merge) {
			std::for_each(to_merge->begin(), to_merge->end(), [&destination](Node* n) {  destination->emplace(n); });
			to_merge->clear();
		};

		this->HiddenClusters.clear();

		//take all the hidden nodes
		list<Node*> open_set;
		std::for_each(this->Nodes.get_map()->begin(), this->Nodes.get_map()->end(), [this, &open_set](auto n) {
			auto ito = this->Observations.get_map()->find(n.second);
			if (ito == this->Observations.get_map()->end()) open_set.push_back(n.second);
		});

		//compute the clusters
		std::unordered_set<Node*>* destination = nullptr;
		list<std::pair<std::list<std::unordered_set<Node*>>::iterator, Node*>> Neigh_cl;
		list<std::pair<std::list<std::unordered_set<Node*>>::iterator, Node*>>::iterator it_Neigh;
		while (!open_set.empty()) {
			Node* att = open_set.front();
			open_set.pop_front();

			auto Neigh = att->GetActiveConnections();
			Neigh_cl.clear();
			destination = nullptr;
			for (auto itn = Neigh->begin(); itn != Neigh->end(); ++itn) {
				Neigh_cl.emplace_back(finder((*itn)->GetNeighbour()), att);
				if (Neigh_cl.back().first != this->HiddenClusters.end()) destination = &(*Neigh_cl.back().first);
			}

			if (destination == nullptr) {
				this->HiddenClusters.emplace_back();
				destination = &this->HiddenClusters.back();
			}
			else {
				it_Neigh = Neigh_cl.begin();
				while (it_Neigh != Neigh_cl.end()) {
					if (it_Neigh->first == this->HiddenClusters.end()) ++it_Neigh;
					else {
						if (&(*it_Neigh->first) != destination) merger(destination, &(*it_Neigh->first));
						it_Neigh = Neigh_cl.erase(it_Neigh);
					}
				}
				auto it_c = this->HiddenClusters.begin();
				auto it_c_end = this->HiddenClusters.end();
				while (it_c != it_c_end) {
					if (it_c->empty()) it_c = this->HiddenClusters.erase(it_c);
					else ++it_c;
				}
			}
			destination->emplace(att);
			for (it_Neigh = Neigh_cl.begin(); it_Neigh != Neigh_cl.end(); ++it_Neigh) {
				destination->emplace(it_Neigh->second);
				open_set.remove(it_Neigh->second);
			}
		}

	}

	std::vector<float> Node::NodeFactory::GetMarginalDistribution(const std::string& var) {

		this->__BeliefPropagation(true);

		Node* node = this->_FindNode(var);
		if (node == nullptr) throw std::runtime_error("variable not present in the model");
		//check the variable is in the hidden set
		auto it = this->Observations.get_map()->find(node);
		if (it != this->Observations.get_map()->end())
			throw std::runtime_error("You asked the marginals of a variable in the evidences set");

		return pot::Factor(node->GetAllUnaries(), false).GetMarginals();

	}

	size_t Node::NodeFactory::GetMAP(const std::string& var) {

		this->__BeliefPropagation(false);

		auto node = this->_FindNode(var);
		if (node == nullptr) throw std::runtime_error("variable not present in the model");
		//check the variable is in the hidden set
		auto it = this->Observations.get_map()->find(node);
		if (it != this->Observations.get_map()->end())
			throw std::runtime_error("You asked the marginals of a variable in the evidences set");

		vector<float> marginals = pot::Factor(node->GetAllUnaries(), false).GetMarginals();
		//find values maximising marginals
		size_t K = marginals.size(), res = 0;
		float max = marginals[0];
		for (size_t k = 1; k < K; ++k) {
			if (marginals[k] > max) {
				max = marginals[k];
				res = k;
			}
		}
		return res;

	}

	std::vector<size_t> Node::NodeFactory::GetMAP() {
		vector<CategoricVariable*> hiddenSet = this->GetHiddenSet();
		vector<size_t> MAP;
		MAP.reserve(hiddenSet.size());
		std::for_each(hiddenSet.begin(), hiddenSet.end(), [&MAP, this](CategoricVariable* v) { MAP.push_back(this->GetMAP(v->GetName())); });
		return MAP;
	}

	size_t* Node::NodeFactory::_FindObservation(const std::string& var_name) {

		auto it = this->Observations.get_map()->find(this->_FindNode(var_name));
		if (it == this->Observations.get_map()->end()) return nullptr;
		else return &it->second->second;

	}

	void Node::NodeFactory::SetThreadPoolSize(const std::size_t& poolSize) {
		if (poolSize <= 1) {
			this->ThPool.reset();
			return;
		}

		if (this->ThPool != nullptr) {
			if(this->ThPool->size() == poolSize) return;
			this->ThPool.reset();
		}
		this->ThPool = make_unique<thpl::equi::Pool>(poolSize);
	}
}

