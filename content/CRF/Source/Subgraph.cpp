#include "../Header/Node.h"
using namespace std;

namespace Segugio {

	Node::Node_factory::_SubGraph::~_SubGraph() {

		if (this->__SubGraph != NULL)
			delete this->__SubGraph;

		if (this->logZ != NULL)
			delete this->logZ;

	}

	Node* find_among_hidden(const list<list<Node*>>& hidden_clusters, Categoric_var* var) {

		list<Node*>::const_iterator it2;
		for (auto it = hidden_clusters.begin(); it != hidden_clusters.end(); it++) {
			for (it2 = it->begin(); it2 != it->end(); it2++) {
				if ((*it2)->Get_var() == var)
					return *it2;
			}
		}

		system("ECHO found variable not in the hidden set");
		abort();

	}
	Node::Node_factory::_SubGraph::_SubGraph(Node_factory* Originating, const std::list<Categoric_var*>& sub_set_to_consider) : 
		logZ(NULL), __SubGraph(NULL) {

		if (Originating->mState != 2)
			abort();

		list<Node*> nodes_in_sub_structure;
		for (auto it_V = sub_set_to_consider.begin();
			it_V != sub_set_to_consider.end(); it_V++) 
			nodes_in_sub_structure.push_back(find_among_hidden(Originating->Last_hidden_clusters , *it_V));

		struct Potential_4_Insertion : public _Pot_wrapper_4_Insertion {
			Potential_4_Insertion(Potential* wrp) : wrapped(wrp) {};
			virtual const std::list<Categoric_var*>* Get_involved_var_safe() { return this->wrapped->Get_involved_var_safe(); };
			virtual Potential*											  Get_Potential_to_Insert(const std::list<Categoric_var*>& var_involved, const bool& get_cloned) {
				return wrapped;
			};
		private:
			Potential*			wrapped;
		};

		list<Potential*> sub_structure;
		list<Potential*>::iterator itC;
		list<Node::Neighbour_connection*>::iterator it_neigh;
		list<Node*>::iterator itN;
		bool insert;
		bool is_in_hidden;
		for (auto it = nodes_in_sub_structure.begin(); it != nodes_in_sub_structure.end(); it++) {
			for (itC = (*it)->Temporary_Unary.begin(); itC != (*it)->Temporary_Unary.end(); itC++)
				sub_structure.push_back(*itC);
			for (itC = (*it)->Permanent_Unary.begin(); itC != (*it)->Permanent_Unary.end(); itC++)
				sub_structure.push_back(*itC);

			//find binary potential included in the substructure
			for (it_neigh = (*it)->Active_connections.begin();
				it_neigh != (*it)->Active_connections.end(); it_neigh++) {
				is_in_hidden = false;
				for (itN = nodes_in_sub_structure.begin(); itN != nodes_in_sub_structure.end(); itN++) {
					if (*itN == (*it_neigh)->Neighbour) {
						is_in_hidden = true;
						break;
					}
				}

				if (is_in_hidden) { //è dentro lista di subset
					insert = true;
				//check this potential was not already inserted
					for (itC = sub_structure.begin(); itC != sub_structure.end(); itC++) {
						if (*itC == (*it_neigh)->Shared_potential) {
							insert = false;
							break;
						}
					}
					if (insert)
						sub_structure.push_back((*it_neigh)->Shared_potential);
				}
			}
		}

		list<_Pot_wrapper_4_Insertion*> sub_structure2;
		for (auto it = sub_structure.begin(); it != sub_structure.end(); it++)
			sub_structure2.push_back(new Potential_4_Insertion(*it));

		this->__SubGraph = new Node_factory(false);
		this->__SubGraph->Insert(sub_structure2);

		for (auto it = sub_structure2.begin(); it != sub_structure2.end(); it++)
			delete *it;

	}

	void Node::Node_factory::_SubGraph::Get_marginal_prob_combination(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination) {

		if (this->logZ == NULL) {
			//compute Z
			list<size_t*> domain;
			list<Categoric_var*> Vars_temp;
			this->__SubGraph->Get_All_variables_in_model(&Vars_temp);
			I_Potential::Get_entire_domain(&domain, Vars_temp);

			this->logZ = new float(0.f);
			list<float> distr_vals;
			this->__SubGraph->Eval_Log_Energy_function(&distr_vals, domain, var_order_in_combination);
			for (auto it = distr_vals.begin(); it != distr_vals.end(); it++)
				*this->logZ += expf(*it);
			*this->logZ = logf(*this->logZ);

			for (auto it = domain.begin(); it != domain.end(); it++)
				free(*it);
		}

		result->clear();
		this->__SubGraph->Eval_Log_Energy_function(result, combinations, var_order_in_combination);
		for (auto it = result->begin(); it != result->end(); it++)
			result->back() = expf(result->back() - *this->logZ);

	}

}