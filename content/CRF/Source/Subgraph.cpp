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

		return NULL;

	}
	Node::Node_factory::_SubGraph::_SubGraph(Node_factory* Originating, const std::list<Categoric_var*>& sub_set_to_consider) : 
		logZ(NULL), __SubGraph(NULL) {
		
		bool is_possible;
		Originating->Belief_Propagation(true, &is_possible);
		if (!is_possible) {
#ifdef _DEBUG
			system("ECHO the subgraph computation is not possible since the hidden set (variables and values) were not already defined");
#endif // DEBUG
			return;
		}

		list<Node*> nodes_in_sub_structure;
		for (auto it_V = sub_set_to_consider.begin();
			it_V != sub_set_to_consider.end(); it_V++) {
			auto node_to_add = find_among_hidden(Originating->Last_hidden_clusters, *it_V);
			if (node_to_add == NULL) {
#ifdef _DEBUG
				system("ECHO passed a variable not in the hidden set for creating sub graph, command ignored");
#endif // DEBUG
			}
			else
				nodes_in_sub_structure.push_back(node_to_add);
		}
		if (nodes_in_sub_structure.empty()) {
#ifdef _DEBUG
			system("ECHO detected empty set of variables for the subgraph construction");
#endif // DEBUG
			return;
		}

		list<Potential*> sub_structure;
		list<Potential*>::iterator itC;
		list<Node::Neighbour_connection*>::iterator it_neigh;
		list<Node*>::iterator itN;
		bool insert;
		bool is_in_subgraph;
		for (auto it = nodes_in_sub_structure.begin(); it != nodes_in_sub_structure.end(); it++) {
			for (itC = (*it)->Temporary_Unary.begin(); itC != (*it)->Temporary_Unary.end(); itC++)
				sub_structure.push_back(*itC);
			for (itC = (*it)->Permanent_Unary.begin(); itC != (*it)->Permanent_Unary.end(); itC++)
				sub_structure.push_back(*itC);

			//find binary potential included in the substructure
			for (it_neigh = (*it)->Active_connections.begin();
				it_neigh != (*it)->Active_connections.end(); it_neigh++) {
				is_in_subgraph = false;
				for (itN = nodes_in_sub_structure.begin(); itN != nodes_in_sub_structure.end(); itN++) {
					if (*itN == (*it_neigh)->Neighbour) {
						is_in_subgraph = true;
						break;
					}
				}

				if (is_in_subgraph) { //è dentro lista di subset
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
				else //add the message incoming from this node
					sub_structure.push_back((*it_neigh)->Message_to_this_node);
			}
		}

		this->__SubGraph = new Node_factory(true); //potentials are all cloned
		for (auto itt = sub_structure.begin(); itt != sub_structure.end(); itt++) {
			Potential_Shape temp(*(*itt)->Get_involved_var_safe());
			(*itt)->clone_distribution(&temp);
			this->__SubGraph->__Insert(&temp);
		}

	}

	void Node::Node_factory::_SubGraph::Get_marginal_prob_combinations(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination) {

		result->clear();

		if (this->__SubGraph == NULL) {
#ifdef _DEBUG
			system("ECHO asked marginals for invalid sub graph");
#endif // DEBUG
			return;
		}

		if (this->logZ == NULL) {
			//compute Z
			list<size_t*> domain;
			list<Categoric_var*> Vars_temp;
			this->__SubGraph->Get_All_variables_in_model(&Vars_temp);
			I_Potential::Get_entire_domain(&domain, Vars_temp);

			this->logZ = new float(0.f);
			list<float> distr_vals;
			this->__SubGraph->Eval_Log_Energy_function(&distr_vals, domain, var_order_in_combination);
			if (distr_vals.empty()) {
#ifdef _DEBUG
				system("ECHO variables not found in sub graph structure");
#endif // DEBUG
				delete this->logZ;
				this->logZ = NULL;
				return;
			}
			for (auto it = distr_vals.begin(); it != distr_vals.end(); it++)
				*this->logZ += expf(*it);
			*this->logZ = logf(*this->logZ);

			for (auto it = domain.begin(); it != domain.end(); it++)
				free(*it);
		}

		this->__SubGraph->Eval_Log_Energy_function(result, combinations, var_order_in_combination);
		for (auto it = result->begin(); it != result->end(); it++)
			*it = expf(*it - *this->logZ);

	}

	void Node::Node_factory::_SubGraph::Get_marginal_prob_combinations(std::list<float>* result, const std::list < std::list<size_t>>& combinations, const std::list<Categoric_var*>& var_order_in_combination) {

		list<size_t*> temp;
		size_t k, K;
		list<size_t>::const_iterator it_l;
		for (auto it = combinations.begin(); it != combinations.end(); it++) {
			K = it->size();
			temp.push_back((size_t*)malloc(K * sizeof(size_t)));
			it_l = it->begin();
			for (k = 0; k < K; k++) {
				temp.back()[k] = *it_l;
				it_l++;
			}
		}

		this->Get_marginal_prob_combinations( result, temp, var_order_in_combination);

		for (auto it = temp.begin(); it != temp.end(); it++)
			free(*it);

	}

	void Node::Node_factory::_SubGraph::MAP(std::list<size_t>* result) { 

		result->clear();

		if (this->__SubGraph == NULL) {
#ifdef _DEBUG
			system("ECHO asked MAP for invalid sub graph");
#endif // DEBUG
			return;
		}

		this->__SubGraph->MAP_on_Hidden_set(result); 

	};

	void Node::Node_factory::_SubGraph::Gibbs_Sampling(std::list<std::list<size_t>>* result, const unsigned int& N_samples, const unsigned int& initial_sample_to_skip) {

		result->clear();

		if (this->__SubGraph == NULL) {
#ifdef _DEBUG
			system("ECHO asked Gibbs sampling on invalid sub graph");
#endif // DEBUG
			return;
		}

		this->__SubGraph->Gibbs_Sampling_on_Hidden_set(result, N_samples, initial_sample_to_skip); 

	};

	void Node::Node_factory::_SubGraph::Get_All_variables_in_model(std::list<Categoric_var*>* result) {

		result->clear();
		if (this->__SubGraph == NULL) return;

		this->__SubGraph->Get_All_variables_in_model(result);

	}

	Categoric_var*	Node::Node_factory::_SubGraph::Find_Variable(const std::string& var_name) {

		if (this->__SubGraph == NULL) {
#ifdef _DEBUG
			system("ECHO asked varaible in a non valid subgraph");
#endif // DEBUG

			return NULL;
		}

		return this->__SubGraph->Find_Variable(var_name);

	}

}