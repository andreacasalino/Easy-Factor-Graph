/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Graphical_model.h>
#include <iostream>
#include <set>
using namespace std;

namespace EFG {

	Graph::Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const I_belief_propagation_strategy& propagator ) :
		Node_factory(true, propagator) {

		XML_reader* reader = nullptr;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) { 
			cout << "warninig: file not readable in Graph construction" << endl; 
			reader = nullptr;
		}
		if (reader != nullptr) {
			Structure_importer strct(*reader , prefix_config_xml_file);
			this->_Insert(strct.Get_structure());
			this->_Set_Evidences(strct.Get_Observations());
		}
		delete reader;

	};




	Graph_Learnable::~Graph_Learnable() {

		for (auto it = this->Atomic_Learner.begin(); it != this->Atomic_Learner.end(); ++it)
			delete *it;

	}

	bool Graph_Learnable::__Compare(const std::vector<Categoric_var*>& set1, const std::vector<Categoric_var*>& set2) {

		size_t S = set1.size();
		if (S != set2.size()) return false;

		if (S == 1) return (set1[0]->Get_name().compare(set2[0]->Get_name()) == 0);
		else {
			if ((set1[0]->Get_name().compare(set2[0]->Get_name()) == 0) && (set1[1]->Get_name().compare(set2[1]->Get_name()) == 0)) return true;
			if ((set1[1]->Get_name().compare(set2[0]->Get_name()) == 0) && (set1[0]->Get_name().compare(set2[1]->Get_name()) == 0)) return true;
		}
		return false;

	}

	std::list<Graph_Learnable::atomic_Learning_handler*>::iterator Graph_Learnable::__Find_in_Atomics(const std::vector<Categoric_var*>& vars) {

		for (auto it = this->Atomic_Learner.begin(); it != this->Atomic_Learner.end(); ++it) {
			if (__Compare((*it)->Get_Distribution().Get_Variables(), vars)) return it;
		}
		return this->Atomic_Learner.end();

	}

	std::pair<std::list<Graph_Learnable::composite_Learning_handler>::iterator, std::list<Graph_Learnable::atomic_Learning_handler*>::iterator>	    Graph_Learnable::__Find_in_Composites(const std::vector<Categoric_var*>& vars) {

		list<atomic_Learning_handler*>* L;
		for (auto it = this->Composite_Learner.begin(); it != this->Composite_Learner.end(); ++it) {
			L = it->Get_Components();
			for (auto it2 = L->begin(); it2 != L->end(); ++it2) {
				if (__Compare((*it2)->Get_Distribution().Get_Variables(), vars)) return make_pair( it, it2);
			}
		}
		return make_pair(this->Composite_Learner.end() , this->Atomic_Learner.end());

	}

	Potential_Exp_Shape* Graph_Learnable::_Insert(Potential_Exp_Shape* pot, const bool& weight_tunability) {

		auto pot_inserted = this->Node_factory::_Insert(pot);
		if (pot_inserted == nullptr) return nullptr;

		if (weight_tunability) {
			if (pot_inserted->Get_Distribution().Get_Variables().size() == 1) {
				//new unary
				this->_Add<Unary_handler>(this, pot_inserted);
			}
			else {
				//new binary
				this->_Add<Binary_handler>(this, pot_inserted);
			}
		}
		return pot_inserted;

	}

	void Graph_Learnable::_Share(const std::vector<Categoric_var*>& set1, const std::vector<Categoric_var*>& set2) {

		list<atomic_Learning_handler*>::iterator a1, a2;
		std::pair<std::list<Graph_Learnable::composite_Learning_handler>::iterator, std::list<Graph_Learnable::atomic_Learning_handler*>::iterator> c1, c2;

		a1 = this->__Find_in_Atomics(set1);
		c1 = this->__Find_in_Composites(set1);

		a2 = this->__Find_in_Atomics(set2);
		c2 = this->__Find_in_Composites(set2);

		if ((c1.first == this->Composite_Learner.end()) && (a1 == this->Atomic_Learner.end())) throw std::runtime_error("inexistent handler");
		if ((c2.first == this->Composite_Learner.end()) && (a2 == this->Atomic_Learner.end())) throw std::runtime_error("inexistent handler");

		if ((a1 != this->Atomic_Learner.end()) && (a2 != this->Atomic_Learner.end())) {
			this->Composite_Learner.emplace_back(*a1, *a2);
			this->Atomic_Learner.erase(a1);
			this->Atomic_Learner.erase(a2);
			this->Learner_list.remove(*a1);
			this->Learner_list.remove(*a2);
			this->Learner_list.push_back(&this->Composite_Learner.back());
			return;
		}

		if ((c1.first != this->Composite_Learner.end()) && (c2.first != this->Composite_Learner.end())) {
			auto el2 = c2.first->Get_Components();
			for (auto it = el2->begin(); it != el2->end(); ++it) c1.first->Append(*it);
			el2->clear();
			this->Learner_list.remove(&(*c2.first));
			this->Composite_Learner.erase(c2.first);
			return;
		}

		if (a1 != this->Atomic_Learner.end()) {
			c2.first->Append(*a1);
			this->Atomic_Learner.erase(a1);
			this->Learner_list.remove(*a1);
		}
		else {
			c1.first->Append(*a2);
			this->Atomic_Learner.erase(a2);
			this->Learner_list.remove(*a2);
		}

	}

	std::vector<float> Graph_Learnable::Get_tunable() const {

		vector<float> w;
		w.reserve(this->Get_model_size());
		for (auto it : this->Learner_list) w.push_back(it->Get_weight());
		return w;

	}

	std::vector<float> Graph_Learnable::Get_tunable_grad(const Discrete_Domain& training_set, const bool& force_alpha_recomputation) {

		bool recompute_alfa = false;
		if (force_alpha_recomputation) recompute_alfa = true;
		else {
			if (this->Last_training_set_used != &training_set) recompute_alfa = true;
		}
		this->Last_training_set_used = &training_set; 

		vector<float> w_grad;
		w_grad.reserve(this->Get_model_size());

		if (recompute_alfa) {
			for (auto it : this->Learner_list) it->Recompute_grad_alfa_part(*this->Last_training_set_used);
		}
		for(auto it : this->Learner_list) w_grad.push_back(it->Get_grad_alfa_part());
		
		vector<float> beta_part = this->_Get_beta_part(training_set);
		size_t k, K = w_grad.size();
		for (k = 0; k < K; ++k)
			w_grad[k] -= beta_part[k];

		if (this->Use_regularization) {
			k = 0;
			for (auto it : this->Learner_list) {
				w_grad[k] -= 2.f * it->Get_weight();
				++k;
			}
		}
		return w_grad;

	}

	void Graph_Learnable::Set_tunable(const std::vector<float>& w_vector) {

		if (this->Get_model_size() != w_vector.size()) throw std::runtime_error("inconsistent vector of weights");
		size_t k = 0;
		for (auto it : this->Learner_list) {
			it->Set_weight(w_vector[k]);
			++k;
		}

	}

	const Node::Node_factory::structure Graph_Learnable::Get_structure() const {

		vector<Potential_Shape*> sh;
		vector<Potential_Exp_Shape*> exp_sh;
		{
			auto srct = this->Node_factory::Get_structure();
			sh = get<0>(srct);
			exp_sh = get<2>(srct);
		}


		vector<vector<Potential_Exp_Shape*>> tunab_clusters;
		set<Potential_Exp_Shape*>			 S;

		tunab_clusters.reserve(this->Atomic_Learner.size() + this->Composite_Learner.size());
		auto itA_end = this->Atomic_Learner.end();
		for (auto itA = this->Atomic_Learner.begin(); itA != itA_end; ++itA) {
			tunab_clusters.emplace_back();
			tunab_clusters.back().push_back( (*itA)->Get_wrapped_exp() );
			S.emplace((*itA)->Get_wrapped_exp());
		}
		list<atomic_Learning_handler*>::const_iterator it, it_end;
		auto itC_end = this->Composite_Learner.end();
		for (auto itC = this->Composite_Learner.begin(); itC != itC_end; ++itC) {
			it_end = itC->Get_Components()->end(); 
			tunab_clusters.emplace_back();
			tunab_clusters.back().reserve(itC->Get_Components()->size());
			for (it = itC->Get_Components()->begin(); it != it_end; ++it) {
				tunab_clusters.back().push_back((*it)->Get_wrapped_exp());
				S.emplace((*it)->Get_wrapped_exp());
			}
		}

		vector<Potential_Exp_Shape*> constant;
		constant.reserve(exp_sh.size() - S.size());
		size_t K = exp_sh.size();
		for(size_t k=0; k<K; ++k){
			if(S.find(exp_sh[k]) == S.end()) constant.push_back(exp_sh[k]);
		}

		return make_tuple(sh , tunab_clusters, constant);

	}

	void Graph_Learnable::_Insert(const structure& strct) {

		const vector<Potential_Shape*>& sh = get<0>(strct);
		size_t k, K = sh.size();
		for (k = 0; k < K; ++k) this->Node_factory::_Insert(sh[k]);

		const vector<Potential_Exp_Shape*>& constant = get<2>(strct);
		K = constant.size();
		for (k = 0; k < K; ++k) this->_Insert(constant[k], false);

		const vector<vector<Potential_Exp_Shape*>>& learnable_exp = get<1>(strct);
		K = learnable_exp.size();
		size_t c, C;
		Potential_Exp_Shape* first_inserted = nullptr;
		for (k = 0; k < K; ++k) {
			first_inserted = this->_Insert(learnable_exp[k][0], true);

			C = learnable_exp[k].size();
			if(C > 1){
				const vector<Categoric_var*>& vars_to_share = first_inserted->Get_Distribution().Get_Variables();
				for (c=1; c<C; ++c) {
					this->_Insert(learnable_exp[k][c], true);
					this->_Share(learnable_exp[k][c]->Get_Distribution().Get_Variables(), vars_to_share);
				}
			}
		}

	}

	std::list<Graph_Learnable::atomic_Learning_handler*>	 Graph_Learnable::_Get_all_handlers() const {

		list<Graph_Learnable::atomic_Learning_handler*> L = this->Atomic_Learner;
		for (auto it : this->Composite_Learner) {
			auto it2_end = it.Get_Components()->end();
			for (auto it2 = it.Get_Components()->begin(); it2 != it2_end; ++it2) L.push_back(*it2);
		}
		return L;

	}




	Random_Field::Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const I_belief_propagation_strategy& propagator) : 
	Graph_Learnable(true, propagator) {

		XML_reader* reader = nullptr;
		try { reader = new XML_reader(prefix_config_xml_file + config_xml_file); }
		catch (int) {
			cout << "warninig: file not readable in Random_Field construction" << endl;
			reader = nullptr;
		}
		if (reader != nullptr) {
			Structure_importer strct(*reader , prefix_config_xml_file);
			this->_Insert(strct.Get_structure());
			this->_Set_Evidences(strct.Get_Observations());
		}
		delete reader;

	};

	vector<float> Random_Field::_Get_beta_part(const Discrete_Domain& training_set) {

		this->Set_Evidences(std::vector<std::pair<std::string, size_t>>{});

		this->_Belief_Propagation(true);

		vector<float> betas;
		betas.reserve(this->Get_model_size());
		auto L = this->_Get_learner_list();
		for (auto it = L->begin(); it != L->end(); ++it) betas.push_back((*it)->Get_grad_beta_part());
		return betas;

	}

	void Random_Field::Insert(Potential_Exp_Shape& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share) {

		auto P = this->Graph_Learnable::_Insert(&pot, true);
		if (P != nullptr) {
			vector<Categoric_var*> vars_shared;
			vars_shared.reserve(vars_of_pot_whose_weight_is_to_share.size());
			for (size_t k = 0; k < vars_of_pot_whose_weight_is_to_share.size(); ++k) {
				Node* temp = this->_Find_Node(vars_of_pot_whose_weight_is_to_share[k]);
				if (temp == nullptr) throw std::runtime_error("inexistent variable");
				vars_shared.push_back(temp->Get_var());
			}
			this->_Share(vars_shared, pot.Get_Distribution().Get_Variables());
		}

	};




	Conditional_Random_Field::Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file, const I_belief_propagation_strategy& propagator) : 
	Graph_Learnable(true , propagator) {

		XML_reader reader(prefix_config_xml_file + config_xml_file); 
		Structure_importer strct(reader, prefix_config_xml_file);
		auto obsv = strct.Get_Observations();
		std::vector<std::string> ev;
		vector<size_t> ev_val;
		ev.reserve(obsv.size());
		ev_val.reserve(obsv.size());
		for (size_t k = 0; k < obsv.size(); ++k) {
			ev.push_back(obsv[k].first);
			ev_val.push_back(obsv[k].second);
		}

		this->_Import(strct.Get_structure() , ev);
		this->Set_Evidences(ev_val);

	};

	Conditional_Random_Field::Conditional_Random_Field(const structure& strct, const std::vector<std::string>& observed_var, const bool& use_cloning_Insert, const I_belief_propagation_strategy& propagator) :
	Graph_Learnable(use_cloning_Insert, propagator) {

		this->_Import(strct, observed_var);

	}

	Conditional_Random_Field::Conditional_Random_Field(const Node_factory& o) : Graph_Learnable(true, _Get_porpagator(o)) {

		auto obsv = o.Get_Actual_Observation_Set();
		std::vector<std::string> ev;
		vector<size_t> ev_val;
		ev.reserve(obsv.size());
		ev_val.reserve(obsv.size());
		for (size_t k = 0; k < obsv.size(); ++k) {
			ev.push_back(obsv[k].first->Get_name());
			ev_val.push_back(obsv[k].second);
		}

		this->_Import(o.Get_structure() , ev);
		this->Set_Evidences(ev_val);

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
				const vector<Categoric_var*>& vars = arr[a]->Get_Distribution().Get_Variables();
				V = vars.size();
				if (V == 1) {
					if (ordered_evidences.find(vars.front()->Get_name()) != ordered_evidences.end()) add = false;
				}
				else if (V == 2) {
					if ( (ordered_evidences.find(vars.front()->Get_name()) != ordered_evidences.end()) && (ordered_evidences.find(vars.back()->Get_name()) != ordered_evidences.end()) ) 
						add = false;
				}
				if (add) minimal.push_back(arr[a]);
			}
			return minimal;
		};
	};
	void Conditional_Random_Field::_Import(const structure& strct, const std::vector<std::string>& evidences) {

	// import the minimal structure that excludes the potential fully connected to obeservatons
		std::set<std::string> ordered_evidences;
		for (auto it : evidences) ordered_evidences.insert(it);
		structure strct_minimal;
		RedundantRemover Rm;
		get<0>(strct_minimal) = Rm(get<0>(strct), ordered_evidences);
		get<2>(strct_minimal) = Rm(get<2>(strct), ordered_evidences);
		get<1>(strct_minimal).reserve(get<1>(strct).size());
		for (size_t k = 0; k < get<1>(strct).size(); ++k) {
			vector<Potential_Exp_Shape*> temp = Rm(get<1>(strct)[k], ordered_evidences);
			if (!temp.empty()) get<1>(strct_minimal).emplace_back(move(temp));
		}
		this->_Insert(strct_minimal);

	//create set of observations and hidden vars
		if (evidences.empty()) cout << "Warning in when building a Conditional_Random_Field: empty evidences set" << endl;
		vector<pair<string, size_t>> obsv;
		obsv.reserve(evidences.size());
		for (size_t o = 0; o < evidences.size(); ++o) obsv.emplace_back(make_pair(evidences[o], 0));
		this->_Set_Evidences(obsv);

	//replace handlers partially connected to observations
		list<atomic_Learning_handler*> all_atomics = this->_Get_all_handlers();
		vector<size_t*> obsv_vals = {nullptr, nullptr};
		size_t K;
		for (auto it : all_atomics) {
			const vector<Categoric_var*>& vars = it->Get_Distribution().Get_Variables();
			K = vars.size();
			if (K == 2) {
				obsv_vals[0] = this->_Find_Observation(vars[0]->Get_name());
				obsv_vals[1] = this->_Find_Observation(vars[1]->Get_name());

				if (obsv_vals[0] != nullptr)
					this->_Replace<Binary_handler_with_Observation>(it->Get_Distribution().Get_Variables(), this->_Find_Node(vars.back()->Get_name()), obsv_vals[0]);
				else if (obsv_vals[1] != nullptr)
					this->_Replace<Binary_handler_with_Observation>(it->Get_Distribution().Get_Variables(), this->_Find_Node(vars.front()->Get_name()), obsv_vals[1]);
			}
		}

		this->vars_order_training_set = nullptr;
		this->pos_observations_in_training_set = nullptr;

	}

	vector<float> Conditional_Random_Field::_Get_beta_part(const Discrete_Domain& training_set) {

		vector<float> betas;
		size_t k, K = this->Get_model_size();
		betas.reserve(K);
		for (k = 0; k < K; ++k) betas.push_back(0.f);
		float coeff = 1.f / (float)training_set.size();

		//recompute pos_observations if needed
		if (&training_set.Get_Variables() != this->vars_order_training_set) {
			this->vars_order_training_set = &training_set.Get_Variables();

			class VarCmp{
				less<string>		comparer;
			public:
				bool operator()(const Categoric_var* a, const Categoric_var* b) const { return this->comparer(a->Get_name() , b->Get_name()); };
			};
			map<Categoric_var* , size_t, VarCmp>  obs_map;
			const vector<Categoric_var*>& train_set_vars = training_set.Get_Variables();
			K = train_set_vars.size();
			for(k =0; k<K; ++k) obs_map.emplace(train_set_vars[k] , k);
	
			auto obs_temp = this->Get_Actual_Observation_Set();
			K = obs_temp.size();
			if(this->pos_observations_in_training_set == nullptr) this->pos_observations_in_training_set = new size_t[K];
			this->pos_observations_size = K;
			for(k=0; k<K; ++k) this->pos_observations_in_training_set[k] = obs_map.find(obs_temp[k].first)->second;
		}

		auto it = training_set.get_iter();
		vector<size_t> observations;
		observations.reserve(this->pos_observations_size);
		K = this->Get_model_size();
		list<I_Learning_handler*>::const_iterator it_h;
		while (it.is_not_at_end()) {
			observations.clear();
			for(k =0 ; k<this->pos_observations_size; ++k) observations.push_back((*it)[this->pos_observations_in_training_set[k]]);
			
			this->_Set_Evidences(observations);
			this->_Belief_Propagation(true);

			it_h = this->_Get_learner_list()->begin();
			for(k=0; k<K; ++k){
				betas[k] += coeff * (*it_h)->Get_grad_beta_part();
				++it_h;
			}

			++it;
		}
		return betas;

	}

}