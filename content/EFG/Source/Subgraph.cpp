/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Subgraph.h"
using namespace std;

namespace EFG {

	bool is_absent_in_subgraph(I_Potential* to_Check , const list<I_Potential*>& bin_already_inserted) {

		auto it_end = bin_already_inserted.end();
		for (auto it = bin_already_inserted.begin(); it != it_end; it++) {
			if (*it == to_Check) return false;
		}
		return true;

	}
	Node::Node_factory::_SubGraph_t::_SubGraph_t(Node_factory& model, const std::list<Categoric_var*>& variables) :
	obs(&model.subject_4_subgraphs , this) {

		this->source = &model;
		check_all_vars_are_different(variables);
		auto it_end = variables.end();
		for (auto it = variables.begin(); it != it_end; it++) {
			this->Nodes.push_back(model.__Find_Node(*it));
			if (this->Nodes.back() == NULL) throw 0;
		}

	}

	Node::Node_factory::_SubGraph_t::_SubGraph_t(_SubGraph_t& subgraph_to_reduce, const std::list<Categoric_var*>& variables) :
	obs(&subgraph_to_reduce.source->subject_4_subgraphs, this) {

		this->source = subgraph_to_reduce.source;
		check_all_vars_are_different(variables);
		auto it_end = variables.end();
		list<Node*>::iterator itN, itN_end;
		Node* temp = NULL;
		for (auto it = variables.begin(); it != it_end; it++) {
			itN_end = subgraph_to_reduce.Nodes.end();
			temp = NULL;
			for (itN = subgraph_to_reduce.Nodes.begin(); itN != itN_end; itN++) {
				if ((*itN)->pVariable == *it) {
					temp = *itN;
					break;
				}
			}
			if (temp == NULL) throw 0; //inexistent variable
			this->Nodes.push_back(temp);
		}

	}

	void Node::Node_factory::_SubGraph_t::Get_All_variables_in_model(std::list<Categoric_var*>* result) {

		result->clear();
		auto it_end = this->Nodes.end();
		for (auto it = this->Nodes.begin(); it != it_end; it++)
			result->push_back((*it)->pVariable);

	}

	class Indicator : public Potential_Shape {
	public:
		Indicator(Categoric_var* var, const size_t& val_observed) : Potential_Shape({var}) {

			auto Dist = Get_distr_static(this);
			size_t* c = (size_t*)malloc(sizeof(size_t));
			c[0] = val_observed;
			Dist->push_back(new Distribution_value_concrete(c, 1.f));

		};
	};
	void Append_collection(list<I_Potential*>* L, const list<Potential*>& to_append) {

		auto it_end = to_append.end();
		for (auto it = to_append.begin(); it != it_end; ++it)
			L->push_back(*it);

	}
	class Joint_Marginal : public Potential_Shape {
	public:
		Joint_Marginal(const vector<float>& E, const I_Potential::combinations& comb) : Potential_Shape(comb.Get_variables()) {

			size_t k, K;
			auto Dist = Get_distr_static(this);

			float coeff = 0.f;
			K = E.size();
			for (k = 0; k < K; k++) coeff += E[k];
			if (coeff == 0.f) coeff = 1.f;
			else coeff = 1.f / coeff;

			K = this->Get_involved_var()->size();
			combinations::iterator it(comb);
			size_t e = 0;
			const size_t* cc = NULL;
			while (it.is_not_at_end()) {
				cc = *it;
				size_t* c = (size_t*)malloc(K * sizeof(size_t));
				for (k = 0; k < K; k++)
					c[k] = cc[k];
				Dist->push_back(new Distribution_value_concrete(c, coeff * E[e]));
				++it;
				e++;
			}

		};
	};
	unique_ptr<Potential_Shape> Node::Node_factory::_SubGraph_t::Get_joint_marginal_distribution() {

		this->source->Belief_Propagation(true);
		list<I_Potential*> pots;

		list<Indicator>		indicators;
		list<I_Potential*> binary_already_inserted;
		list<I_Potential*>::iterator itb, itb_end;
		observation_info* info = NULL;
		list<Node::Neighbour_connection*> outside, inside;
		list<Node::Neighbour_connection*>::iterator ito, ito_end;
		auto it_end = this->Nodes.end();
		bool insert;
		for (auto it = this->Nodes.begin(); it != it_end; ++it) {
			//check whether is observed
			info = this->__Check_is_observed(*it);
			if (info != NULL) {
			//observations
				indicators.emplace_back((*it)->pVariable , info->Value);
				pots.push_back(&indicators.back());
			}
			else {
			//hidden
				this->__Get_outside_inside(*it, &outside, &inside);
				Append_collection(&pots, (*it)->Temporary_Unary);
				Append_collection(&pots, (*it)->Permanent_Unary);
				ito_end = outside.end();
				for (ito = outside.begin(); ito != ito_end; ++ito)
					pots.push_back((*ito)->Message_to_this_node);
				ito_end = inside.end();
				for (ito = inside.begin(); ito != ito_end; ++ito) {
					insert = true;
					itb_end = binary_already_inserted.end();
					for (itb = binary_already_inserted.begin(); itb != itb_end; itb++) {
						if (*itb == (*ito)->Shared_potential) {
							insert = false;
							break;
						}
					}					
					if (insert) {
						pots.push_back((*ito)->Shared_potential);
						binary_already_inserted.push_back(pots.back());
					}
				}
			}
		}

		list<Categoric_var*> vars;
		this->Get_All_variables_in_model(&vars);
		I_Potential::combinations comb(vars);

		vector<float> E;
		Node_factory::__Eval_energy(&E, comb, pots, false);

		return unique_ptr<Potential_Shape>(new Joint_Marginal(E, comb));

	}

	void Node::Node_factory::_SubGraph_t::Get_message_from_outside(std::vector<float>* message, Categoric_var* var) {

		message->clear();
		this->source->Belief_Propagation(true);

		Node* n = NULL;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it) {
			if ((*it)->pVariable == var) {
				n = *it;
				break;
			}
		}
		if (n == NULL) throw 0; //variable not contained in subgraph

		list<Potential*> messages;
		observation_info* obs_info = this->__Check_is_observed(n);

		if (obs_info == NULL) {
			list<Node::Neighbour_connection*> outside, temp;
			this->__Get_outside_inside(n, &outside, &temp);
			for (auto it = outside.begin(); it != outside.end(); ++it)
				messages.push_back((*it)->Message_to_this_node);
			for (auto it = n->Temporary_Unary.begin(); it != n->Temporary_Unary.end(); it++)
				messages.push_back(*it);
		}

		if (messages.empty()) {
			size_t K = var->size();
			float coeff = 1.f / (float)K;
			for (size_t k = 0; k < K; k++) message->push_back(coeff);
		}
		else {
			Potential mex(messages, false);
			mex.Get_images(message);
		}

	}

	bool is_in_Nodes(const list<Node*>& nodes, Node* involved) {

		auto it_end = nodes.end();
		for (auto it = nodes.begin(); it != it_end; ++it) {
			if (*it == involved) return true;
		}
		return false;

	}
	void Node::Node_factory::_SubGraph_t::__Get_outside_inside(Node* involved, std::list<Node::Neighbour_connection*>* outside, std::list<Node::Neighbour_connection*>* inside) {

		outside->clear();
		inside->clear();
		auto itA_end = involved->Active_connections.end();
		for (auto itA = involved->Active_connections.begin(); itA != itA_end; itA++) {
			if (is_in_Nodes(this->Nodes, (*itA)->Neighbour)) inside->push_back(*itA);
			else outside->push_back(*itA);
		}

	}

	Node::Node_factory::observation_info* Node::Node_factory::_SubGraph_t::__Check_is_observed(Node* involved) {

		auto it_end = this->source->Last_observation_set.end();
		for (auto it = this->source->Last_observation_set.begin(); it != it_end; it++) {
			if (it->Involved_node == involved)
				return &(*it);
		}
		return NULL;

	}

}