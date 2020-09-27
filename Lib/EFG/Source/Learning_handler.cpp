/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Graphical_model.h>
using namespace std;

namespace EFG {

	Graph_Learnable::atomic_Learning_handler::atomic_Learning_handler(Potential_Exp_Shape* pot_to_handle) :
		I_Potential_Decorator(pot_to_handle), weigth_modifier(*pot_to_handle) { }

	void Graph_Learnable::atomic_Learning_handler::Recompute_grad_alfa_part(const Discrete_Domain& train_set) {

		Discrete_Distribution::const_Full_Match_finder finder( this->Get_wrapped()->Get_Distribution() , train_set.Get_Variables());
		auto it = train_set.get_iter();
		this->alfa_part = 0.f;
		float coeff = 1.f / (float)train_set.size();
		while (it.is_not_at_end()){
			auto val = finder(*it);
			//impossible to have val == nullptr since it's an exp shape
			this->alfa_part += coeff * val->Get_val_raw();
			++it;
		}

	}




	Graph_Learnable::composite_Learning_handler::~composite_Learning_handler() {

		for (auto it = this->Components.begin(); it != this->Components.end(); ++it)
			delete* it;

	}

	Graph_Learnable::composite_Learning_handler::composite_Learning_handler(atomic_Learning_handler* initial_A, atomic_Learning_handler* initial_B) {

		this->Components.push_back(initial_A);
		this->Append(initial_B);

	}

	void Graph_Learnable::composite_Learning_handler::Set_weight(const float& w_new) {

		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; ++it)
			(*it)->Set_weight(w_new);

	}

	void Graph_Learnable::composite_Learning_handler::Recompute_grad_alfa_part(const Discrete_Domain& train_set) {

		this->alfa_part = 0.f;
		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; ++it) {
			(*it)->Recompute_grad_alfa_part(train_set);
			this->alfa_part += (*it)->Get_grad_alfa_part();
		}

	}

	float Graph_Learnable::composite_Learning_handler::Get_grad_beta_part() {

		float beta = 0.f;
		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; ++it) {
			beta += (*it)->Get_grad_beta_part();
		}
		return beta;

	}

	void Graph_Learnable::composite_Learning_handler::Append(atomic_Learning_handler* to_add) {

		this->Components.push_back(to_add);
		to_add->Set_weight(this->Components.front()->Get_weight());

	};




	float Graph_Learnable::Unary_handler::Get_grad_beta_part() {

		auto marginals = Potential_Shape(this->pNode->Get_all_Unaries(), false).Get_marginals();
		float beta = 0.f;
		auto it = this->Get_wrapped()->Get_Distribution().get_iter();
		while (it.is_not_at_end()) {
			beta += it->Get_val_raw() * marginals[it->Get_indeces()[0]];
			++it;
		}
		return beta;

	}




	Graph_Learnable::Binary_handler::Binary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle) :
		atomic_Learning_handler(pot_to_handle), 
		pNode1(model->_Find_Node(pot_to_handle->Get_Distribution().Get_Variables().front()->Get_name())),
		pNode2(model->_Find_Node(pot_to_handle->Get_Distribution().Get_Variables().back()->Get_name())) { 
	};

	float Graph_Learnable::Binary_handler::Get_grad_beta_part() {

		auto conn1 = this->pNode1->Get_Active_connections();
		auto temp1 = this->pNode1->Get_Temporary();
		auto perm1 = this->pNode1->Get_Permanent();

		auto conn2 = this->pNode2->Get_Active_connections();
		auto temp2 = this->pNode1->Get_Temporary();
		auto perm2 = this->pNode1->Get_Permanent();

		vector<const I_Potential*> pots;
		pots.reserve(temp1->size() + perm1->size() + conn1->size() + temp2->size() + perm2->size() + conn2->size() - 1 );
		pots.push_back(this->Get_wrapped());
		for(auto it = temp1->begin(); it!=temp1->end(); ++it) pots.push_back(&(*it));
		for(auto it = perm1->begin(); it!=perm1->end(); ++it) pots.push_back(*it);
		for(auto it = conn1->begin(); it!=conn1->end(); ++it) {
			if((*it)->Get_Neighbour() != this->pNode2 ) pots.push_back((*it)->Get_IncomingMessage());
		}
		for(auto it = temp2->begin(); it!=temp2->end(); ++it) pots.push_back(&(*it));
		for(auto it = perm2->begin(); it!=perm2->end(); ++it) pots.push_back(*it);
		for(auto it = conn2->begin(); it!=conn2->end(); ++it) {
			if((*it)->Get_Neighbour() != this->pNode1 ) pots.push_back((*it)->Get_IncomingMessage());
		}

		Potential_Shape pot_tot(pots, false, true);

		auto marginals = pot_tot.Get_marginals();
		auto it = this->Get_wrapped()->Get_Distribution().get_iter();
		float beta = 0.f;
		size_t k = 0;
		while (it.is_not_at_end()) {
			beta += it->Get_val_raw() * marginals[k];
			++it;
			++k;
		}
		return beta;
	}




	Conditional_Random_Field::Binary_handler_with_Observation::Binary_handler_with_Observation(Potential_Exp_Shape* pot, Node* Hidden_var, size_t* observed_val) :
		atomic_Learning_handler(pot), pNode_hidden(Hidden_var), ref_to_val_observed(observed_val) {

		const vector<Categoric_var*>& vars = this->Get_wrapped()->Get_Distribution().Get_Variables();
		this->pos_observed = 0;
		this->pos_hidden = 1;
		if (vars.front() == Hidden_var->Get_var()) {
			this->pos_observed = 1;
			this->pos_hidden = 0;
		}

	};

	float Conditional_Random_Field::Binary_handler_with_Observation::Get_grad_beta_part() {

		vector<float> marginals = Potential_Shape(this->pNode_hidden->Get_all_Unaries(), false).Get_marginals();

		auto it = this->Get_wrapped()->Get_Distribution().get_iter();
		float beta = 0.f;
		const size_t* comb;
		while (it.is_not_at_end()) {
			comb = it->Get_indeces();
			if(comb[this->pos_observed] == *this->ref_to_val_observed) 
				beta += it->Get_val_raw() * marginals[comb[this->pos_hidden]];
			++it;
		}
		return beta;

	}

}