#include <Node.h>
#include <float.h>
#include <iostream>
#include <ctime>
using namespace std;


namespace EFG {

	Node::Node(Categoric_var* var, const bool& dont_clone_var) {

		if (dont_clone_var) this->pVariable = var;
		else 			    this->pVariable = new Categoric_var(var->size(), var->Get_name());

	}

	Node::~Node() {

		//for (auto it = this->Permanent_Unary.begin(); it != this->Permanent_Unary.end(); ++it)
		//	delete *it; //this is detroyed by the Node_Factory

		for (auto it = this->Active_connections.begin(); it != this->Active_connections.end(); ++it)
			delete *it;

		for (auto it = this->Disabled_connections.begin(); it != this->Disabled_connections.end(); ++it)
			delete *it;

		//delete this->pVariable; //this is detroyed by the Node_Factory

	}

	std::vector<const I_Potential*> Node::Get_all_Unaries() const{

		vector<const I_Potential*> pot;
		pot.reserve(this->Permanent_Unary.size() + this->Temporary_Unary.size() + this->Active_connections.size());
		for(auto it=this->Permanent_Unary.begin(); it!=this->Permanent_Unary.end(); ++it) pot.push_back(*it);
		for(auto it=this->Temporary_Unary.begin(); it!=this->Temporary_Unary.end(); ++it) pot.push_back(&(*it));
		for(auto it=this->Active_connections.begin(); it!=this->Active_connections.end(); ++it) pot.push_back((*it)->Get_IncomingMessage());
		return pot;

	}




	void Node::Neighbour_connection::init_connection(Node* peer_A, Node* peer_B, const I_Potential& pot_shared){

		Neighbour_connection* A_B = new Neighbour_connection();
		Neighbour_connection* B_A = new Neighbour_connection();

		A_B->Neighbour = peer_B;
		B_A->Neighbour = peer_A;

		A_B->was_Neighbourhood_updated = false;
		B_A->was_Neighbourhood_updated = false;

		A_B->Message_to_this_node = nullptr;
		B_A->Message_to_this_node = nullptr;

		A_B->Linked = B_A;
		B_A->Linked = A_B;

		A_B->Shared_potential = &pot_shared;
		B_A->Shared_potential = &pot_shared;

		peer_A->Active_connections.push_back(A_B);
		peer_B->Active_connections.push_back(B_A);

	}

	void Node::Neighbour_connection::Disable(){

		Node* this_node = this->Linked->Neighbour;
		this_node->Active_connections.remove(this);
		this_node->Disabled_connections.push_back(this);

		this->Neighbour->Active_connections.remove(this->Linked);
		this->Neighbour->Disabled_connections.push_back(this->Linked);

		this->Linked->Reset_OutgoingMessage();
		this->Reset_OutgoingMessage();

	}

	void Node::Neighbour_connection::__update_Neighbourhood(){

		if(this->was_Neighbourhood_updated) return;
		this->Neighbourhood = this->Linked->Neighbour->Active_connections;
		this->Neighbourhood.remove(this);
		this->was_Neighbourhood_updated = true;

	}

	bool Node::Neighbour_connection::Is_OutgoingMessage_Recomputation_possible(){

		this->__update_Neighbourhood();
		for(auto it=this->Neighbourhood.begin(); it!=this->Neighbourhood.end(); ++it){
			if((*it)->Message_to_this_node == nullptr) return false;
		}
		return true;

	}

	float Node::Neighbour_connection::Recompute_OutgoingMessage(const bool& Sum_or_MAP){

		this->__update_Neighbourhood();

		Node* this_node = this->Linked->Neighbour;
		vector<const I_Potential*> una_pot;
		una_pot.reserve(this_node->Temporary_Unary.size() + this_node->Permanent_Unary.size() + this->Neighbourhood.size());
		for(auto it = this_node->Temporary_Unary.begin(); it!=this_node->Temporary_Unary.end(); ++it) una_pot.push_back(&(*it));
		for(auto it = this_node->Permanent_Unary.begin(); it!=this_node->Permanent_Unary.end(); ++it) una_pot.push_back(*it);
		for(auto it = this->Neighbourhood.begin(); it!=this->Neighbourhood.end(); ++it) una_pot.push_back((*it)->Message_to_this_node);

		if(this->Linked->Message_to_this_node == nullptr){
			if(una_pot.empty()) this->Linked->Message_to_this_node = new Message( *this->Shared_potential, this_node->Get_var(), Sum_or_MAP);
			else 		 	    this->Linked->Message_to_this_node = new Message( *this->Shared_potential, una_pot, Sum_or_MAP);
			return FLT_MAX;
		}
		else{
			if(una_pot.empty()) return static_cast<Message*>(this->Linked->Message_to_this_node)->Update( *this->Shared_potential, this_node->Get_var(), Sum_or_MAP);
			else 		 	    return static_cast<Message*>(this->Linked->Message_to_this_node)->Update( *this->Shared_potential, una_pot, Sum_or_MAP);
		}
		
	}

	void Node::Neighbour_connection::Set_IncomingMessage_ones(){

		delete this->Message_to_this_node;
		this->Message_to_this_node = new Message(this->Linked->Neighbour->Get_var());

	}

	Node::Neighbour_connection::Message::Message(Categoric_var* var_involved) : I_Potential_Decorator() { 
		auto ones_shape = new Potential_Shape({var_involved});
		ones_shape->Set_ones();
		this->Replace_wrapped(ones_shape);
	}

	float Node::Neighbour_connection::Message::Update(const I_Potential& binary_to_merge, const std::vector<const I_Potential*>& potential_to_merge, const bool& Sum_or_MAP) {

		//if (potential_to_merge.empty())  throw 0; //  Message::Update: wrong method for managing an empty set of unary to merge

		vector<const I_Potential*> to_merge;
		to_merge.reserve(1 + potential_to_merge.size());
		to_merge = potential_to_merge;
		to_merge.push_back(&binary_to_merge);
		Potential_Shape M(to_merge, true, true);

		//if(M.Get_Distribution().Get_Variables().size() != 2) throw 1;

		return this->Update(M, potential_to_merge.front()->Get_Distribution().Get_Variables().front(), Sum_or_MAP);

	}

	float Node::Neighbour_connection::Message::Update(const I_Potential& binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP) {

		const vector<Categoric_var*>& vars = binary_to_merge.Get_Distribution().Get_Variables();
		//if(vars.size() != 2) throw 0;

		Categoric_var* var_remaining = vars.front();
		if(var_remaining == var_to_marginalize) var_remaining = vars.back();

		Potential_Shape* mex = new Potential_Shape(vector<Categoric_var*>{var_remaining});

		Discrete_Distribution::const_Partial_Match_finder finder(binary_to_merge.Get_Distribution() , {var_remaining});

		size_t D = var_remaining->size();

		std::list<const Discrete_Distribution::Value*> finder_match;
		std::list<const Discrete_Distribution::Value*>::iterator it, it_end;
		float val;
		if(Sum_or_MAP){
			for(size_t d = 0; d<D; ++d){
				finder_match = finder(&d);
				val = 0.f;
				it_end = finder_match.end();
				for(it = finder_match.begin(); it!=it_end; ++it)  val += (*it)->Get_val();
				mex->Add_value(&d , val);
			}
		}
		else{
			float temp;
			for(size_t d = 0; d<D; ++d){
				finder_match = finder(&d);
				val = 0.f;
				it_end = finder_match.end();
				for(it = finder_match.begin(); it!=it_end; ++it) {
					temp = (*it)->Get_val();
					if(temp > val) val = temp;
				}
				mex->Add_value(&d , val);
			}
		}
		mex->Normalize_distribution();

		struct Distr_diff_eval{ 
			float operator()(const Discrete_Distribution& F1, const Discrete_Distribution& F2) const {
				if (F1.size() != F2.size())  return FLT_MAX;

				float diff = 0.f;
				auto it1 = F1.get_iter();
				auto it2 = F2.get_iter();
				while (it1.is_not_at_end()) {
					diff += abs(it1->Get_val() - it2->Get_val());
					++it1;
					++it2;
				}
				return diff;
			}; 
		};

		float diff = FLT_MAX;
		if(this->Get_wrapped() != nullptr) diff = Distr_diff_eval()(mex->Get_Distribution(), this->Get_wrapped()->Get_Distribution());
		this->Replace_wrapped(mex);
		return diff;

	}




	Node::Node_factory::~Node_factory() {

		this->Potential_observers.clear();

		list<Categoric_var*> var_to_delete;
		if (this->bDestroy_Potentials_and_Variables) {
			for (auto it = this->Binary_potentials.get_map()->begin(); it != this->Binary_potentials.get_map()->end(); ++it)  delete* it->second;

			for (auto itN = this->Nodes.get_map()->begin(); itN != this->Nodes.get_map()->end(); ++itN) {
				for (auto itU = itN->second->Permanent_Unary.begin(); itU != itN->second->Permanent_Unary.end(); ++itU) delete* itU;
				
				var_to_delete.push_back(itN->second->Get_var());
			}
		}
		this->Nodes.clear();

		for (auto itV = var_to_delete.begin(); itV != var_to_delete.end(); ++itV) delete* itV;

		delete this->Propagation_Last;

	}

	class Hasher{ 
	protected:
		hash<string>   							   		   hasher;
	};

	class Node_hasher : public Hasher { public: size_t operator()(const Node* k) const { return this->hasher(k->Get_var()->Get_name()); }; };

	class Name_Pred{
	public:
		Name_Pred(const bool& flag) :  use_val_or_ref(flag) {};

		bool operator()(const string* a, const string* b) const { 
			if(this->use_val_or_ref) return (a->compare(*b) == 0);
			else{
				if(a == b) return true;
				return (a->compare(*b) == 0);
			} 					 
		};
	private:
		bool 								use_val_or_ref;
	};

	class BiName_Pred{
	public:
		BiName_Pred(const bool& flag) : pred(flag) {};

		bool operator()(const std::pair<const string*, const string*>& a,
					    const std::pair<const string*, const string*>& b) const {  

			if(this->pred(a.first, b.first) && this->pred(a.second, b.second)) return true;
			if(this->pred(a.first, b.second) && this->pred(a.second, b.first)) return true;
			return false;

		};
	private:
		Name_Pred                   	    pred;
	};

	struct Last_observation_set_KeyGen { Node*  operator()(std::pair<Node*, size_t>& val) const { return val.first; }; };

	struct Nodes_KeyGen { const string*  operator()(Node& val) const { return &val.Get_var()->Get_name(); }; };
	class  Nodes_hasher : public Hasher {  public: size_t operator()(const string* k) const { return this->hasher(*k); }; };

	struct Binary_potentials_KeyGen { std::pair<const string*, const string*>  operator()(const I_Potential* val) const { auto vars = val->Get_Distribution().Get_Variables(); return make_pair(&vars.front()->Get_name() , &vars.back()->Get_name()); }; };
	class  Binary_potentials_hasher : public Hasher { 
	public:
		size_t operator()(const std::pair<const string*, const string*>& k) const { 
			size_t hash1 = this->hasher(*k.first);
			size_t hash2 = this->hasher(*k.second);
			if(hash1 < hash2) return hash1;
			else		      return hash2;
		}; 
	};

	void add_hidden_cluster(std::list< unordered_set<Node*, std::function<size_t(const Node*)>>  >	& to_emplace){ to_emplace.emplace_back(0 , Node_hasher() ); }

	Node::Node_factory::Obsv_container::Obsv_container() : 
	univocal_map<Node*, std::pair<Node*, size_t>, std::function<Node*(std::pair<Node*, size_t>&)>, std::function<size_t(const Node*)>>(Last_observation_set_KeyGen(), Node_hasher()) {}

	Node::Node_factory::Node_factory(const bool& use_cloning_Insert, const I_belief_propagation_strategy& propagator) : 
	bDestroy_Potentials_and_Variables(use_cloning_Insert), Propagation_max_iter(1000), Propagator(move(propagator.copy())),
        Last_observation_set(),
	    Nodes(Nodes_KeyGen() , Nodes_hasher(), Name_Pred(use_cloning_Insert)),	
	    Binary_potentials(Binary_potentials_KeyGen(), Binary_potentials_hasher(), BiName_Pred(use_cloning_Insert) )   
	{ this->Propagation_Last = nullptr; };

	Categoric_var* Node::Node_factory::Find_Variable(const std::string& var_name) const{

		auto it = this->Nodes.get_map()->find(&var_name);
		if(it == this->Nodes.get_map()->end()) return nullptr;
		else return it->second->Get_var();

	}

	Node*		   Node::Node_factory::_Find_Node(const std::string& var_name){

		auto it = this->Nodes.get_map()->find(&var_name);
		if(it == this->Nodes.get_map()->end()) return nullptr;
		else return it->second;

	}

	std::vector<Categoric_var*> Node::Node_factory::Get_Actual_Hidden_Set() const{

		size_t K = 0;

		for(auto it = this->Last_hidden_clusters.begin(); it!=this->Last_hidden_clusters.end(); ++it) K += it->size(); 

		vector<Categoric_var*> vars;
		vars.reserve(K);
		std::unordered_set<Node*, std::function<size_t(const Node*)> >::const_iterator itc, itc_end;
		for(auto it = this->Last_hidden_clusters.begin(); it!=this->Last_hidden_clusters.end(); ++it) {
			for(itc = it->begin(); itc!=it->end(); ++itc) vars.push_back((*itc)->Get_var());
		}
		return vars;

	}

	std::vector<std::pair<Categoric_var*, size_t>> Node::Node_factory::Get_Actual_Observation_Set() const {

		const std::vector<Node*>& order = this->Last_observation_set.get_order();
		size_t K = order.size();
		vector<std::pair<Categoric_var*, size_t>> obsv;
		obsv.reserve(order.size());
		for(size_t k=0; k<K; ++k){
			auto it = this->Last_observation_set.get_map()->find(order[k]);
			obsv.emplace_back(move( make_pair(it->first->Get_var() , it->second->second) ));
		}
		return obsv;

	};

	std::vector<Categoric_var*> Node::Node_factory::Get_All_variables_in_model() const {

		std::vector<Categoric_var*> vars;
		vars.reserve(this->Nodes.get_map()->size());
		for(auto it = this->Nodes.get_map()->begin(); it!=this->Nodes.get_map()->end(); ++it) vars.push_back(it->second->Get_var());
		return vars;

	}

	const Node::Node_factory::structure Node::Node_factory::Get_structure() const {

		vector<Potential_Shape*> S;
		S.reserve(this->__Simple_shapes.size());
		for(auto it= this->__Simple_shapes.begin(); it!=this->__Simple_shapes.end(); ++it) S.push_back(*it);

		vector<Potential_Exp_Shape*> E;
		E.reserve(this->__Exponential_shapes.size());
		for(auto it= this->__Exponential_shapes.begin(); it!=this->__Exponential_shapes.end(); ++it) E.push_back(*it);

		return make_tuple(S , std::vector<std::vector<Potential_Exp_Shape*>>{}, E);

	}

	void Node::Node_factory::_Insert(Potential_Shape* shape){

		Potential_Shape* temp = nullptr;
		try { temp = this->__Insert(shape); }
		catch (int) {
			temp = nullptr;
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != nullptr) this->__Simple_shapes.push_back(temp);

	}

	Potential_Exp_Shape* Node::Node_factory::_Insert(Potential_Exp_Shape* exp_shape){

		Potential_Exp_Shape* temp = nullptr;
		try { temp = this->__Insert(exp_shape); }
		catch (int) {
			temp = nullptr;
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != nullptr)  this->__Exponential_shapes.push_back(temp);
		return temp;

	}

	void Node::Node_factory::_Insert(const structure& strct){

		size_t k, K;

		const vector<Potential_Shape*>* shp = &get<0>(strct);
		K = shp->size();
		for(k=0; k<K; ++k) this->_Insert((*shp)[k]);

		const vector<Potential_Exp_Shape*>* cst = &get<2>(strct);
		K = cst->size();
		for(k=0; k<K; ++k) this->_Insert((*cst)[k]);

		const std::vector<std::vector<Potential_Exp_Shape*>>* clst = &get<1>(strct);
		size_t C = clst->size();
		for(size_t c=0; c<C; ++c){
			K = (*clst)[c].size();
			for(k=0; k<K; ++k) this->_Insert((*clst)[c][k]);
		}

	}

	void Node::Node_factory::Set_Iteration_4_propagation(const unsigned int& iter_to_use){

		unsigned int temp = iter_to_use;
		if (temp < 10) temp = 10;
		this->Propagation_max_iter = temp;

	}

	void Node::Node_factory::_Belief_Propagation(const bool& sum_or_MAP){

		delete this->Propagation_Last;
		this->Propagation_Last = nullptr;
		this->__Belief_Propagation(sum_or_MAP);

	}

	void Node::Node_factory::__Belief_Propagation(const bool& sum_or_MAP){

		if (this->Nodes.get_map()->empty()) {
			cout << "warning: asked belief propagation on an empty structure\n";
			return; 
		}

		if(this->Propagation_Last == nullptr) this->__Recompute_clusters();
		else {
			if (this->Propagation_Last->Was_Sum_or_MAP == sum_or_MAP) return;
		}

	//reset all messages for nodes in the hidden set
		auto it_cl = this->Last_hidden_clusters.begin();
		list<Neighbour_connection*>::iterator it_conn;
		std::unordered_set<Node*, std::function<size_t(const Node*)> >::iterator it_it_cl;
		for (it_cl; it_cl != this->Last_hidden_clusters.end(); ++it_cl) {
			for (it_it_cl = it_cl->begin(); it_it_cl != it_cl->end(); ++it_it_cl) {
				for (it_conn = (*it_it_cl)->Active_connections.begin(); it_conn != (*it_it_cl)->Active_connections.end(); ++it_conn) (*it_conn)->Reset_OutgoingMessage();
			}
		}

     //perform belief propagation
		bool all_done_within_iter = true;
		all_done_within_iter = (*this->Propagator)(this->Last_hidden_clusters , sum_or_MAP, this->Propagation_max_iter);

		if(this->Propagation_Last == nullptr) this->Propagation_Last = new belief_propagation_info();
		this->Propagation_Last->Was_Sum_or_MAP = sum_or_MAP;
		this->Propagation_Last->Terminated_within_iter = all_done_within_iter;

	}

	void Node::Node_factory::Obsv_container::set_vars(const std::vector<Node*>& vars){

		this->map.clear();
		size_t K = vars.size();
		for(size_t k=0; k< K; ++k) this->emplace(move(make_pair(vars[k] , 0)));
		this->order = vars;

	}

	void Node::Node_factory::Obsv_container::set_vals(const std::vector<size_t>& vals){

		size_t K =this->order.size();
		if(vals.size() != K) throw std::runtime_error("inconsistent number of edivences");
		for(size_t k=0; k<K; ++k){
			auto it = this->map.find(this->order[k]);
			if(order[k]->Get_var()->size() <= vals[k]) throw std::runtime_error("inconsistent number of edivences");
			it->second->second = vals[k];
		}

	}

	void Node::Node_factory::_Set_Evidences(const std::vector<std::pair<std::string, size_t>>& new_observations) {

		auto itN = this->Nodes.get_map()->begin();
		list<Neighbour_connection*>::iterator it_neigh;
		auto itN_end = this->Nodes.get_map()->end();
		for (itN; itN != itN_end; ++itN) {
			for (it_neigh = itN->second->Disabled_connections.begin(); it_neigh != itN->second->Disabled_connections.end(); ++it_neigh)
				itN->second->Active_connections.push_back(*it_neigh);
			itN->second->Disabled_connections.clear();
		}
		
		vector<Node*> Vars;
		Vars.reserve(new_observations.size());
		vector<size_t> Vals;
		Vals.reserve(new_observations.size());

		for(size_t kv = 0; kv<new_observations.size(); ++kv){
			auto it_var = this->Nodes.get_map()->find(&new_observations[kv].first);
			if(it_var != this->Nodes.get_map()->end()){
				auto conn = &it_var->second->Active_connections;
				while(!conn->empty()){
					conn->front()->Disable();
				}

				Vars.push_back(it_var->second);
				Vals.push_back(new_observations[kv].second);
			}
		}
		this->Last_observation_set.set_vars(Vars);

		this->__Recompute_clusters();

		this->_Set_Evidences(Vals);

		if (Vars.size() != new_observations.size()) throw std::runtime_error("detected at least one variable not present in this graph");
	}

	void Node::Node_factory::_Set_Evidences(const std::vector<size_t>& new_observations) {

		this->Last_observation_set.set_vals(new_observations);

		delete this->Propagation_Last;
		this->Propagation_Last = nullptr;

	//delete all previous temporary messages
		for(auto itN = this->Nodes.get_map()->begin(); itN!=this->Nodes.get_map()->end(); ++itN) itN->second->Temporary_Unary.clear();

	//recompute temporary messages
		list<Neighbour_connection*>::iterator it_conn;
		for (auto it = this->Last_observation_set.get_map()->begin(); it != this->Last_observation_set.get_map()->end(); ++it) {
			for (it_conn = it->second->first->Disabled_connections.begin(); 
			it_conn != it->second->first->Disabled_connections.end(); ++it_conn)
				(*it_conn)->Get_Neighbour()->Temporary_Unary.emplace_back(std::vector<size_t>{it->second->second}, std::vector<Categoric_var*>{it->first->pVariable}, *(*it_conn)->Get_pot());
		}

	}

	void Node::Node_factory::__Recompute_clusters(){

		class cluster_finder{
			std::list<std::unordered_set<Node*, std::function<size_t(const Node*)> >>* clusters;	
		public:
			cluster_finder(std::list<std::unordered_set<Node*, std::function<size_t(const Node*)> >>* cl) : clusters(cl) {};
			std::list<std::unordered_set<Node*, std::function<size_t(const Node*)> >>::iterator operator()(Node* to_find){
				for(auto itH = this->clusters->begin(); itH!=this->clusters->end(); ++itH){
					auto it = itH->find(to_find);
					if(it != itH->end()) return itH;
				}
				return this->clusters->end();
			};
		} finder(&this->Last_hidden_clusters);

		struct cluster_merger {
			void operator()(std::unordered_set<Node*, std::function<size_t(const Node*)> >* destination, std::unordered_set<Node*, std::function<size_t(const Node*)> >* to_merge){
				for(auto it=to_merge->begin(); it!=to_merge->end(); ++it) destination->emplace(*it);
				to_merge->clear();
			};
		} merger;

		this->Last_hidden_clusters.clear();

    //take all the hidden nodes
		list<Node*> open_set;
		auto ito_end = this->Last_observation_set.get_map()->end();
		for(auto itN = this->Nodes.get_map()->begin(); itN!=this->Nodes.get_map()->end(); ++itN){
			auto ito = this->Last_observation_set.get_map()->find(itN->second);
			if(ito == ito_end) open_set.push_back(itN->second);
		}

    //compute the clusters
		std::unordered_set<Node*, std::function<size_t(const Node*)> >* destination = nullptr;
		list<std::pair<std::list<std::unordered_set<Node*, std::function<size_t(const Node*)> >>::iterator , Node*>> Neigh_cl;
		list<std::pair<std::list<std::unordered_set<Node*, std::function<size_t(const Node*)> >>::iterator , Node*>>::iterator it_Neigh;
		while(!open_set.empty()){
			Node* att = open_set.front();
			open_set.pop_front();

			auto Neigh = att->Get_Active_connections();
			Neigh_cl.clear();
			destination = nullptr;
			for(auto itn = Neigh->begin(); itn!= Neigh->end(); ++itn) {
				Neigh_cl.emplace_back(finder((*itn)->Get_Neighbour()) , att);
				if(Neigh_cl.back().first != this->Last_hidden_clusters.end()) destination = &(*Neigh_cl.back().first);
			}

			if(destination == nullptr){
				add_hidden_cluster(this->Last_hidden_clusters);
				destination = &this->Last_hidden_clusters.back();
			}
			else{
				it_Neigh = Neigh_cl.begin();
				while(it_Neigh != Neigh_cl.end()){
					if(it_Neigh->first == this->Last_hidden_clusters.end()) ++it_Neigh;
					else{
						if(&(*it_Neigh->first) != destination) merger(destination , &(*it_Neigh->first));
						it_Neigh = Neigh_cl.erase(it_Neigh);
					}
				}
				auto it_c = this->Last_hidden_clusters.begin();
				auto it_c_end = this->Last_hidden_clusters.end();
				while (it_c != it_c_end) {
					if(it_c->empty()) it_c = this->Last_hidden_clusters.erase(it_c);
					else ++it_c;
				}				
			}
			destination->emplace(att);
			for(it_Neigh = Neigh_cl.begin(); it_Neigh!=Neigh_cl.end(); ++it_Neigh) {
				destination->emplace(it_Neigh->second);
				open_set.remove(it_Neigh->second);
			}
		}

	}

	std::vector<float> Node::Node_factory::Get_marginal_distribution(const std::string& var){

		this->__Belief_Propagation(true);

		Node* node = this->_Find_Node(var);
		if(node == nullptr) throw std::runtime_error("variable not present in the model");
	//check the variable is in the hidden set
		auto it = this->Last_observation_set.get_map()->find(node);
		if(it != this->Last_observation_set.get_map()->end())
			throw std::runtime_error("You asked the marginals of a variable in the evidences set");

		return Potential_Shape(node->Get_all_Unaries(), false).Get_marginals();

	}

	size_t Node::Node_factory::Get_MAP(const std::string& var){

		this->__Belief_Propagation(false);

		auto node = this->_Find_Node(var);
		if (node == nullptr) throw std::runtime_error("variable not present in the model");
		//check the variable is in the hidden set
		auto it = this->Last_observation_set.get_map()->find(node);
		if (it != this->Last_observation_set.get_map()->end())
			throw std::runtime_error("You asked the marginals of a variable in the evidences set");

		vector<float> marginals = Potential_Shape(node->Get_all_Unaries(), false).Get_marginals();
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

	std::list<std::vector<size_t>> Node::Node_factory::Gibbs_Sampling_on_Hidden_set(const unsigned int& N_samples, const unsigned int& initial_sample_to_skip, const int& seed){

		this->__Recompute_clusters();

		if(seed >= 0) srand((unsigned int)seed);
		else srand((unsigned int)time(NULL));

		struct discrete_sampler{
			size_t  operator()(const vector<float>& distr) const{
				float r = (float)rand() / (float)RAND_MAX;
				size_t k=0;
				if (r <= distr[k])  return k;

				float cum = distr[k];
				size_t K = distr.size();
				for (k = k+1; k < K; ++k) {
					cum += distr[k];
					if (r <= cum) return k;
				}

				return (distr.size() - 1);
			};

			size_t  operator()(const size_t& size) const{
				return (*this)(vector<float>(size , 1.f / (float)size));
			};
		};

		class Sampler{
			struct Node_info {
				Categoric_var*																		var;
				size_t*																				sample;
				vector<const I_Potential*>														unaries;
				vector<tuple<const I_Potential*, Categoric_var*, const size_t*>>  neighbour;
			};
			
		// data
			size_t*				Sampled;
			vector<Node_info>	Info;
			discrete_sampler	disc_sampler;
		public:
			Sampler(Node_factory* model){
				auto H_vars = model->Get_Actual_Hidden_Set();
				this->Sampled = new size_t[H_vars.size()];
				this->Info.reserve(H_vars.size());

				size_t H = H_vars.size(), p; 
				for(p=0; p<H; ++p) this->Sampled[p] = 0;
				for(size_t h=0; h<H; ++h){
					this->Info.emplace_back();
					this->Info.back().var = H_vars[h];
					this->Info.back().sample = &this->Sampled[h];

					Node* node = model->_Find_Node(H_vars[h]->Get_name());
					auto perm = node->Get_Permanent();
					auto temp = node->Get_Temporary();
					this->Info.back().unaries.reserve(perm->size() + temp->size());
					for(auto it = perm->begin(); it!=perm->end(); ++it) this->Info.back().unaries.push_back(*it);
					for(auto it = temp->begin(); it!=temp->end(); ++it) this->Info.back().unaries.push_back(&(*it));

					auto Neigh = node->Get_Active_connections();
					this->Info.back().neighbour.reserve(Neigh->size());
					for(auto itN = Neigh->begin(); itN!=Neigh->end(); ++itN){
						const size_t* ref = nullptr;
						for(p=0; p<H; ++p){
							if((*itN)->Get_Neighbour()->Get_var() == H_vars[p]){
								ref = &this->Sampled[p];
								break;
							}
						}
						this->Info.back().neighbour.emplace_back(move(make_tuple( (*itN)->Get_pot(), (*itN)->Get_Neighbour()->Get_var(), ref)));
					}
				}
			};
			~Sampler(){ delete[] this->Sampled; };

			inline const size_t* 	get_sampled() const { return this->Sampled; };
			size_t					get_vars_size()  const { return this->Info.size(); };
			void 					operator()(const unsigned int& iterations) {
				size_t i, I=this->Info.size();
				size_t n, N;
				for(unsigned int s = 0; s<iterations; ++s){
					for(i=0; i<I; ++i){
						vector<const I_Potential*> to_merge;
						list<Potential_Shape> temp;
						to_merge.reserve(this->Info[i].unaries.size() + this->Info[i].neighbour.size());
						to_merge = this->Info[i].unaries;
						N = this->Info[i].neighbour.size();
						for(n=0; n<N; ++n){
							temp.emplace_back(std::vector<size_t>{*get<2>(this->Info[i].neighbour[n])} , std::vector<Categoric_var*>{get<1>(this->Info[i].neighbour[n])}, *get<0>(this->Info[i].neighbour[n]) );
							to_merge.push_back(&temp.back());
						}

						if(to_merge.empty())    *this->Info[i].sample = disc_sampler(this->Info[i].var->size());
						else   					*this->Info[i].sample = disc_sampler(Potential_Shape(to_merge, false).Get_marginals());
					}
				}
			};
		} sampler(this);

		unsigned int n_delta = (int)floorf(initial_sample_to_skip*0.1f);
		if (n_delta == 0) n_delta = 1;

	//skip initial samples
		sampler(initial_sample_to_skip);

	//do sampling
		list<vector<size_t>> result;
		size_t v,V = sampler.get_vars_size();
		auto Samples = sampler.get_sampled();
		for (size_t n = 0; n < N_samples; ++n) {
			sampler(n_delta);
			result.emplace_back();
			result.back().reserve(V);
			for(v = 0; v<V; ++v) result.back().push_back(Samples[v]);
		}
		return result;

	}

	size_t* Node::Node_factory::_Find_Observation(const std::string& var_name){

		auto it = this->Last_observation_set.get_map()->find(this->_Find_Node(var_name));
		if(it == this->Last_observation_set.get_map()->end()) return nullptr;
		else return &it->second->second;

	}




    Node::Node_factory::Energy_Evaluator::Energy_Evaluator(const Node_factory& factory, const std::vector<Categoric_var*>& vars_order){

		this->Potentials.reserve(factory.Get_structure_size());
		for(auto it = factory.__Exponential_shapes.begin(); it!= factory.__Exponential_shapes.end(); ++it) this->Potentials.push_back(*it);
		for(auto it = factory.__Simple_shapes.begin(); it!= factory.__Simple_shapes.end(); ++it) this->Potentials.push_back(*it);

		this->Finders.reserve(this->Potentials.size());
		for(auto it = this->Potentials.begin(); it!= this->Potentials.end(); ++it) 
			this->Finders.push_back(new Discrete_Distribution::const_Full_Match_finder((*it)->Get_Distribution(), vars_order));

	}

}