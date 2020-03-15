/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Graphical_model.h"
using namespace std;

namespace EFG {

	Graph_Learnable::atomic_Learning_handler::atomic_Learning_handler(Potential_Exp_Shape* pot_to_handle) :
		I_Potential_Decorator(pot_to_handle), weigth_modifier(*pot_to_handle) {

		this->Destroy_wrapped = false; //this exponential shape will be wrapped also by a Potential to be stored in the graphical model

	}

	Potential_Exp_Shape* Graph_Learnable::atomic_Learning_handler::__extract_weight_hndlr(atomic_Learning_handler** other) {

		auto extracted = (*other)->pwrapped;
		(*other)->detach();
		return extracted;

	}

	Graph_Learnable::atomic_Learning_handler::atomic_Learning_handler(atomic_Learning_handler** other) : atomic_Learning_handler(__extract_weight_hndlr(other)) {

		delete* other;

	};

	void Graph_Learnable::atomic_Learning_handler::Recompute_grad_alfa_part(const I_Potential::combinations& train_set) {

		vector<const I_Distribution_value*> match;
		train_set.Find_images_single_matches(&match, *this->pwrapped->Get_wrapped_Shape());
		this->alfa_part = 0.f;
		float coeff = 1.f / (float)match.size();
		size_t K = match.size();
		for (size_t k = 0; k < K; k++) {
			if(match[k] != NULL)
				this->alfa_part += coeff * match[k]->Get_val();
		}

	}

	bool Graph_Learnable::atomic_Learning_handler::is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share) {

		if (vars_of_pot_whose_weight_is_to_share.size() == 1) {
			if (vars_of_pot_whose_weight_is_to_share.front() == this->Get_involved_var()->front())
				return true;
			else
				return false;
		}
		else {
			auto vars = this->Get_involved_var();

			if ((vars_of_pot_whose_weight_is_to_share.front() == vars->front()) && (vars_of_pot_whose_weight_is_to_share.back() == vars->back()))
				return true;

			if ((vars_of_pot_whose_weight_is_to_share.front() == vars->back()) && (vars_of_pot_whose_weight_is_to_share.back() == vars->front()))
				return true;

			return false;

		}
		return false;

	}




	Graph_Learnable::composite_Learning_handler::~composite_Learning_handler() {

		for (auto it = this->Components.begin(); it != this->Components.end(); it++)
			delete* it;

	}

	Graph_Learnable::composite_Learning_handler::composite_Learning_handler(atomic_Learning_handler* initial_A, atomic_Learning_handler* initial_B) {

		this->Components.push_back(initial_A);
		this->Append(initial_B);

	}

	void Graph_Learnable::composite_Learning_handler::Set_weight(const float& w_new) {

		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; it++)
			(*it)->Set_weight(w_new);

	}

	void Graph_Learnable::composite_Learning_handler::Recompute_grad_alfa_part(const I_Potential::combinations& train_set) {

		this->alfa_part = 0.f;
		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; it++) {
			(*it)->Recompute_grad_alfa_part(train_set);
			this->alfa_part += (*it)->Get_grad_alfa_part();
		}

	}

	float Graph_Learnable::composite_Learning_handler::Get_grad_beta_part() {

		float beta = 0.f;
		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; it++) {
			beta += (*it)->Get_grad_beta_part();
		}
		return beta;

	}

	bool Graph_Learnable::composite_Learning_handler::is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share) {

		auto it_end = this->Components.end();
		for (auto it = this->Components.begin(); it != it_end; it++) {
			if ((*it)->is_here_Pot_to_share(vars_of_pot_whose_weight_is_to_share))
				return true;
		}
		return false;

	}

	void Graph_Learnable::composite_Learning_handler::Append(atomic_Learning_handler* to_add) {

		this->Components.push_back(to_add);
		to_add->Set_weight(this->Components.front()->Get_weight());

	};




	float Graph_Learnable::Unary_handler::Get_grad_beta_part() {

		vector<float> marginals;
		list<Potential*> message_union;
		this->pNode->Gather_all_Unaries(&message_union);
		Potential UP(message_union);
		UP.Get_marginals(&marginals);

		float beta = 0.f;
		auto Dist = Get_distr_static(this->pwrapped->Get_wrapped_Shape());
		auto it_end = Dist->end();
		for (auto it = Dist->begin(); it != it_end; it++) 
			beta += (*it)->Get_val() * marginals[(*it)->Get_indeces()[0]];
		return beta;

	}




	Graph_Learnable::Binary_handler::Binary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle) :
		atomic_Learning_handler(pot_to_handle), 
		pNode1(model->__Find_Node(pot_to_handle->Get_involved_var()->front())), 
		pNode2(model->__Find_Node(pot_to_handle->Get_involved_var()->back())) { 

		combinations comb(*this->pwrapped->Get_involved_var());
		comb.Find_images_single_matches(&this->E_shape, *this->pwrapped->Get_wrapped_Shape());
		comb.Find_images_single_matches(&this->E_exp_shape, *this->pwrapped);

	};

	void get_equi_marginals(vector<float>* marginals, const size_t& S) {

		marginals->reserve(S);
		marginals->clear();
		float coeff = 1.f / (float)S;
		for (size_t k = 0; k < S; k++)
			marginals->push_back(coeff);

	}
	float Graph_Learnable::Binary_handler::Get_grad_beta_part() {

		vector<float> E1, E2;
		{
			list<Potential*> pots1 , pots2;
			this->pNode1->Compute_neighbourhood_messages(&pots1, this->pNode2);
			this->pNode2->Compute_neighbourhood_messages(&pots2, this->pNode1);

			if (pots1.empty()) get_equi_marginals(&E1, this->pNode1->Get_var()->size());
			else {
				Potential Mex_1(pots1);
				Mex_1.Get_marginals(&E1);
			}

			if (pots2.empty()) get_equi_marginals(&E2, this->pNode2->Get_var()->size());
			else {
				Potential Mex_2(pots2);
				Mex_2.Get_marginals(&E2);
			}
		}

		float Z = 0.f;
		size_t k, K = this->E_exp_shape.size();
		const size_t* comb;
		vector<float> P_12;
		P_12.reserve(K);
		for (k = 0; k < K; k++) {
			comb = this->E_exp_shape[k]->Get_indeces();
			P_12.push_back(this->E_exp_shape[k]->Get_val() * E1[comb[0]] * E2[comb[1]]);
			Z += P_12.back();
		}

		float beta = 0.f;
		if (Z < 1e-8) {
			Z = 1.f / (float)K;
			auto Dist = Get_distr_static(this->pwrapped->Get_wrapped_Shape());
			auto itD_end = Dist->end();
			for (auto itD = Dist->begin(); itD != itD_end; itD++)
				beta += (*itD)->Get_val();
			beta *= Z;
		}
		else {
			Z = 1.f / Z;
			for (k = 0; k < K; k++) {
				if (this->E_shape[k] != NULL)
					beta += this->E_shape[k]->Get_val() * P_12[k];
			}
			beta *= Z;
		}
		return beta;

	}




	void Conditional_Random_Field::Binary_handler_with_Observation::Create(Node* Hidden_var, size_t* observed_val, atomic_Learning_handler** handle_to_substitute) {

		*handle_to_substitute = new Binary_handler_with_Observation(Hidden_var, observed_val, handle_to_substitute);

	}

	Conditional_Random_Field::Binary_handler_with_Observation::Binary_handler_with_Observation(Node* Hidden_var, size_t* observed_val, atomic_Learning_handler** handle_to_substitute) :
		atomic_Learning_handler(handle_to_substitute), pNode_hidden(Hidden_var), ref_to_val_observed(observed_val) {

		auto vars = this->pwrapped->Get_involved_var();
		this->pos_observed = 0;
		this->pos_hidden = 1;
		if (vars->front() == Hidden_var->Get_var()) {
			this->pos_observed = 1;
			this->pos_hidden = 0;
		}

	};

	float Conditional_Random_Field::Binary_handler_with_Observation::Get_grad_beta_part() {

		vector<float> marginals;
		list<Potential*> message_union;
		this->pNode_hidden->Gather_all_Unaries(&message_union);
		Potential UP(message_union);
		UP.Get_marginals(&marginals);

		auto Distr = Get_distr_static(this->pwrapped->Get_wrapped_Shape());
		auto it_end = Distr->end();
		float beta = 0.f;
		const size_t* comb;
		for (auto it = Distr->begin(); it != it_end; it++) {
			comb = (*it)->Get_indeces();
			if (comb[this->pos_observed] == *this->ref_to_val_observed)
				beta += (*it)->Get_val() * marginals[comb[this->pos_hidden]];
		}
		return beta;

	}

}