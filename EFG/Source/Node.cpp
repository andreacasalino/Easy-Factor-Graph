/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include "../Header/Node.h"
#include <time.h>
#include <cmath>
#include <iostream> 
using namespace std;


namespace EFG {

	template<typename T>
	void exist_in_list(bool* result, const list<T>& L, const T& candidate) {

		*result = false;
		auto it_end = L.end();
		for (auto it = L.begin(); it != it_end; it++) {
			if (*it == candidate) {
				*result = true;
				return;
			}
		}

	};

	template<typename T>
	void list_2_vector(vector<T>* V, const list<T>& L) {

		V->clear();
		V->reserve(L.size());
		auto it_end = L.end();
		for (auto it = L.begin(); it != it_end; it++)
			V->push_back(*it);

	}





	void Node::Neighbour_connection::Recompute_Neighboorhoods(list<Neighbour_connection*>& connections) {

		auto it = connections.begin();
		auto it_end = connections.end();
		for (it; it != connections.end(); it++)
			(*it)->Neighbourhood.clear();

		list<Neighbour_connection*> processed;
		list<Neighbour_connection*>::iterator it2;
		for (it = connections.begin(); it != it_end; it++) {
			(*it)->Neighbourhood = processed;

			for (it2 = processed.begin(); it2 != processed.end(); it2++)
				(*it2)->Neighbourhood.push_back(*it);

			processed.push_back(*it);
		}

	}





	Node::Node(Categoric_var* var, const bool& dont_clone_var) {

		if (dont_clone_var)
			this->pVariable = var;
		else
			this->pVariable = new Categoric_var(var->size(), var->Get_name());

	}

	Node::~Node() {

		for (auto it = this->Temporary_Unary.begin(); it != this->Temporary_Unary.end(); it++)
			delete *it;

		//for (auto it = this->Permanent_Unary.begin(); it != this->Permanent_Unary.end(); it++)
		//	delete *it; //this is detroyed by the Node_Factory

		for (auto it = this->Active_connections.begin(); it != this->Active_connections.end(); it++)
			delete *it;

		for (auto it = this->Disabled_connections.begin(); it != this->Disabled_connections.end(); it++)
			delete *it;

		//delete this->pVariable; //this is detroyed by the Node_Factory

	}

	void Node::Gather_all_Unaries(std::list<Potential*>* result) {

		result->clear();

		this->Append_temporary_permanent_Unaries(result);

		auto it_end = this->Active_connections.end();
		for (auto it = this->Active_connections.begin(); it != it_end; it++) {
#ifdef _DEBUG
			if ((*it)->Message_to_this_node == NULL) abort();  //Found NULL incoming messages not computed, report this bug to andrecasa91@gmail.com
#endif // _DEBUG
			result->push_back((*it)->Message_to_this_node);
		}

#ifdef _DEBUG
		if (result->empty()) abort(); //empty set of messages found
#endif // _DEBUG

	}

	void Node::Append_temporary_permanent_Unaries(list<Potential*>* result) {

		auto it = this->Permanent_Unary.begin();
		auto it_end = this->Permanent_Unary.end();
		for (it; it != it_end; it++)
			result->push_back(*it);

		it_end = this->Temporary_Unary.end();
		for (it = this->Temporary_Unary.begin(); it != it_end; it++)
			result->push_back(*it);

	}

	void Node::Append_permanent_Unaries(std::list<Potential*>* result) {

		auto it = this->Permanent_Unary.begin();
		auto it_end = this->Permanent_Unary.end();
		for (it; it != it_end; it++)
			result->push_back(*it);

	}

	void Node::Compute_neighbour_set(std::list<Node*>* Neigh_set) {

		Neigh_set->clear();
		auto itn_end = this->Active_connections.end();
		for (auto itn = this->Active_connections.begin(); itn != itn_end; itn++)
			Neigh_set->push_back((*itn)->Neighbour);

	}

	void Node::Compute_neighbour_set(std::list<Node*>* Neigh_set, std::list<Potential*>* binary_in_Neigh_set) {

		Neigh_set->clear();
		binary_in_Neigh_set->clear();
		auto it_end = this->Active_connections.end();
		for (auto itn = this->Active_connections.begin(); itn != it_end; itn++) {
			Neigh_set->push_back((*itn)->Neighbour);
			binary_in_Neigh_set->push_back((*itn)->Shared_potential);
		}

	}

	void Node::Compute_neighbourhood_messages(std::list<Potential*>* messages, Node* node_involved_in_connection) {

		messages->clear();

		this->Append_temporary_permanent_Unaries(messages);

		auto it_end = this->Active_connections.end();
		list<Node::Neighbour_connection*>::iterator it_N, it_N_end;
		for (auto it = this->Active_connections.begin(); it != it_end; it++) {
			if ((*it)->Neighbour == node_involved_in_connection) {
				it_N_end= (*it)->Neighbourhood.end();
				for (it_N = (*it)->Neighbourhood.begin(); it_N != it_N_end; it_N++) {
#ifdef _DEBUG
					if ((*it_N)->Message_to_this_node == NULL) abort(); // Found NULL incoming messages not computed, report this bug to andrecasa91@gmail.com
#endif // _DEBUG
					messages->push_back((*it_N)->Message_to_this_node);
				}
				return;
			}
		}

		abort(); //inexsistent neighbour node, report this bug to andrecasa91@gmail.com

	}





	Node::Node_factory::~Node_factory() {

		auto ito_end = this->Potential_observers.end();
		for (auto ito = this->Potential_observers.begin(); ito != ito_end; ito++)
			ito->detach();

		auto itN = this->Nodes.begin();
		list<Categoric_var*> var_to_delete;
		if (this->bDestroy_Potentials_and_Variables) {
			for (auto itP = this->Binary_potentials.begin(); itP != this->Binary_potentials.end(); itP++)
				delete* itP;

			for (itN; itN != this->Nodes.end(); itN++) {
				for (auto itU = (*itN)->Permanent_Unary.begin(); itU != (*itN)->Permanent_Unary.end(); itU++)
					delete *itU;

				var_to_delete.push_back((*itN)->pVariable);
			}
		}

		for (itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++)
			delete *itN;
		for (auto itV = var_to_delete.begin(); itV != var_to_delete.end(); itV++)
			delete* itV;

		if (this->Last_propag_info != NULL) delete this->Last_propag_info;

	}

	Node::Node_factory::Node_factory(const Node_factory& o) :
		Last_propag_info(NULL), Iterations_4_belief_propagation(o.Iterations_4_belief_propagation), bDestroy_Potentials_and_Variables(true) {};

	void Node::Node_factory::__copy(const Node_factory& o) {

		vector<Potential_Shape*> shapes;
		vector<std::list<Potential_Exp_Shape*>> learn_clst;
		vector<Potential_Exp_Shape*> const_exp;
		o.__Get_structure(&shapes, &learn_clst, &const_exp);
		this->__Insert(shapes, learn_clst, const_exp);

		list<size_t> vals;
		o.Get_Actual_Observation_Set_Val(&vals);
		list<Categoric_var*> o_vars;
		o.Get_Actual_Observation_Set_Var(&o_vars);
		list<Categoric_var*> vars;
		auto it_end = o_vars.end();
		for (auto it = o_vars.begin(); it != it_end; it++)
			vars.push_back(this->Find_Variable((*it)->Get_name()));
		this->Set_Evidences(vars, vals);

	}

	void Node::Node_factory::Set_Iteration_4_belief_propagation(const unsigned int& iter_to_use) {

		unsigned int temp = iter_to_use;
		if (temp < 10)
			temp = 10;

		this->Iterations_4_belief_propagation = temp;

	}

	Categoric_var* Find_by_name(list<Categoric_var>& vars, const string& name) {

		auto it_end = vars.end();
		for (auto it = vars.begin(); it != it_end; it++) {
			if (it->Get_name().compare(name) == 0)
				return &(*it);
		}
		return NULL;

	};
	void parse_Variable(list<Categoric_var>* variables, XML_reader::Tag_readable& tag) {

		const string* Size = tag.Get_Attribute_first_found("Size");
		const string* Name = tag.Get_Attribute_first_found("name");
		if (Find_by_name(*variables, *Name) != NULL)  throw 0; //  found multiple variables with the same name		
		variables->emplace_back((size_t)atoi(Size->c_str()), *Name);

	}
	Potential_Shape* Import_shape(const string& prefix, XML_reader::Tag_readable& tag,  list<Categoric_var>& vars) {

		list<Categoric_var*> var_involved;
		list<string> names;
		tag.Get_Attributes("var", &names);
		if (names.size() == 1) {
			var_involved.push_back(Find_by_name(vars, names.front()));
			if (var_involved.front() == NULL) throw 0; // found potential with invalid var set
		}
		else if (names.size() == 2) {
			var_involved.push_back(Find_by_name(vars, names.front()));
			if (var_involved.back() == NULL) throw 0; // found potential with invalid var set

			var_involved.push_back(Find_by_name(vars, names.back()));
			if (var_involved.back() == NULL) throw 0; // found potential with invalid var set
		}
		else throw 0; //found potential with invalid var set

		const string* val = NULL;
		try { val = tag.Get_Attribute_first_found("Source"); }
		catch (int) { val = NULL; }
		if (val != NULL)  return new Potential_Shape(var_involved, prefix + *val);

		try { val = tag.Get_Attribute_first_found("Correlation"); }
		catch (int) { val = NULL; }
		if (val != NULL) {
			if (val->compare("T") == 0) 
				return new Potential_Shape(var_involved, true);
			else if (val->compare("F") == 0)
				return new Potential_Shape(var_involved, false);
			else throw 1; //found potential with invalid options
		}

		auto shape = new Potential_Shape(var_involved);
		list<XML_reader::Tag_readable> distr_vals;
		tag.Get_Nested("Distr_val", &distr_vals);
		list<string> indices_raw;
		list<size_t> indices;
		const string* temp_D;
		while (!distr_vals.empty()) {
			distr_vals.front().Get_Attributes("v", &indices_raw);
			temp_D = distr_vals.front().Get_Attribute_first_found("D");
			indices.clear();
			while (!indices_raw.empty()) {
				indices.push_back((size_t)atoi(indices_raw.front().c_str()));
				indices_raw.pop_front();
			}
			shape->Add_value(indices, (float)atof(temp_D->c_str()));
			distr_vals.pop_front();
		}
		return shape;

	};
	struct tunab_info {
		Potential_Exp_Shape* tunab_exp;
		list<Categoric_var*> vars_to_share;
		size_t				 cluster_id;
	};
	bool operator==(const tunab_info& lhs, const tunab_info& rhs)
	{

		size_t S = lhs.vars_to_share.size();
		if (S != rhs.vars_to_share.size()) return false;

		if (S == 1) return (lhs.vars_to_share.front() == rhs.vars_to_share.front());
		else {
			if ((rhs.vars_to_share.front() == lhs.vars_to_share.front())
				&& (rhs.vars_to_share.back() == lhs.vars_to_share.back())) return true;
			if ((rhs.vars_to_share.front() == lhs.vars_to_share.back())
				&& (rhs.vars_to_share.back() == lhs.vars_to_share.front())) return true;
			return false;
		}

	}
	void compute_tunab_clusters(vector<list<Potential_Exp_Shape*>>* cluster, list<tunab_info> pots) {

		if (pots.empty()) return;

		list<tunab_info> processed;
		size_t C = 0;
		list<tunab_info>::iterator it, it_end;
		bool found;
		while (!pots.empty()) {
			if (pots.front().vars_to_share.empty()) {
				pots.front().cluster_id = C;
				C++;
			}
			else {
			// find twin in processed
				found = false;
				it_end = processed.end();
				for (it = processed.begin(); it != it_end; it++) {
					if (*it == pots.front()) {
						pots.front().cluster_id = it->cluster_id;
						found = true;
						break;
					}
				}
				if (!found) {
					found = false;
					it_end = pots.end();
					it = pots.begin();
					it++;
					for (it; it != it_end; it++) {
						if (*it == pots.front()) {
							found = true;
							break;
						}
					}
					if (!found) throw 0; //found inexistent potential whose weight is to share
					pots.front().cluster_id = C;
					C++;
				}
			}
			processed.push_back(pots.front());
			pots.pop_front();
		}

		cluster->reserve(C);
		for (size_t k = 0; k < C; k++)
			cluster->push_back(list<Potential_Exp_Shape*>());
		it = processed.begin();
		it_end = processed.end();
		for (it; it != it_end; it++) 
			(*cluster)[it->cluster_id].push_back(it->tunab_exp);

	}
	void Node::Node_factory::Import_from_XML(XML_reader* reader, const std::string& prefix_config_xml_file) {

		this->bDestroy_Potentials_and_Variables = true;

	//import variables
		list<Categoric_var>	variables;
		list<XML_reader::Tag_readable> Nested;
		list<string> hidden_var_names;
		list<size_t> hidden_vals;
		reader->Get_root().Get_Nested("Variable", &Nested);
		const string* var_flag = NULL;
		for (auto it = Nested.begin(); it != Nested.end(); it++) {
			parse_Variable(&variables, *it);
			try { var_flag = it->Get_Attribute_first_found("flag"); }
			catch (int) { var_flag = NULL; }
			if (var_flag != NULL) {
				if (var_flag->compare("O") == 0) {
					hidden_var_names.push_back(variables.back().Get_name());
					hidden_vals.push_back(0);
				}
				else if (var_flag->compare("H") != 0) throw 0; //unrecognized flag value for var tag
			}
		}

	//import potentials
		vector<Potential_Shape*>						shapes;
		vector<Potential_Exp_Shape*>					const_exp;
		list<tunab_info>						    tunab_exp;

		Potential_Shape* temp_shape;
		reader->Get_root().Get_Nested("Potential", &Nested);
		const string* w_temp;
		float w_val_temp;
		const string* tun_temp;
		bool is_const;
		list<string> names_to_share;
		for (auto it = Nested.begin(); it != Nested.end(); it++) {
			temp_shape = Import_shape(prefix_config_xml_file, *it, variables);
			try { w_temp = it->Get_Attribute_first_found("weight"); }
			catch (int) { w_temp = NULL; }

			if (w_temp == NULL)
				shapes.push_back(temp_shape);
			else {
				w_val_temp = (float)atof(w_temp->c_str());
				auto temp_exp_shape = new Potential_Exp_Shape(*temp_shape, w_val_temp);
				try { tun_temp = it->Get_Attribute_first_found("tunability"); }
				catch (int) { tun_temp = NULL; }
				is_const = false;
				if (tun_temp != NULL) {
					if (tun_temp->compare("Y") == 0) is_const = false;
					else if (tun_temp->compare("N") == 0) is_const = true;
					else throw 0; //not recognized parameter
				}

				if(is_const)
					const_exp.push_back(temp_exp_shape);
				else {
					tunab_exp.push_back(tunab_info());
					tunab_exp.back().tunab_exp = temp_exp_shape;
					if (it->Exist_Nested_tag("Share")) {
						it->Get_Nested_first_found("Share").Get_Attributes("var", &names_to_share);
						for (auto itn = names_to_share.begin(); itn != names_to_share.end(); itn++) {
							tunab_exp.back().vars_to_share.push_back(Find_by_name(variables, *itn));
							if (tunab_exp.back().vars_to_share.back() == NULL) throw 1;
						}
						if (temp_exp_shape->Get_involved_var()->size() != tunab_exp.back().vars_to_share.size()) throw 1;
					}
				}
			}
		}

	// create clusters of tunable potentials
		vector<list<Potential_Exp_Shape*>> tunab_clusters;
		compute_tunab_clusters(&tunab_clusters , tunab_exp);
		this->__Insert(shapes, tunab_clusters, const_exp);

		for (auto it = shapes.begin(); it != shapes.end(); it++) delete* it;
		for (auto it = const_exp.begin(); it != const_exp.end(); it++) delete* it;
		for (auto it = tunab_exp.begin(); it != tunab_exp.end(); it++) delete it->tunab_exp;

	// set hidden set
		if (!hidden_var_names.empty()) {
			list<Categoric_var*> hidden_vars;
			for (auto it = hidden_var_names.begin(); it != hidden_var_names.end(); it++)
				hidden_vars.push_back(this->Find_Variable(*it));
			this->Set_Evidences(hidden_vars, hidden_vals);
		}

	}

	Node* Node::Node_factory::__Find_Node(Categoric_var* var) const {

		auto it_end = this->Nodes.end();
		for (auto it = this->Nodes.begin(); it != it_end; it++) {
			if ((*it)->Get_var() == var)
				return *it;
		}

		if (this->bDestroy_Potentials_and_Variables) {
			for (auto it = this->Nodes.begin(); it != it_end; it++) {
				if ((*it)->Get_var()->Get_name().compare(var->Get_name()) == 0) {
					if ((*it)->Get_var()->size() == var->size()) return *it;
				}
			}
		}

		return NULL;

	}

	void Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) {

		if (new_observed_vars.size() != new_observed_vals.size())  throw 0; //Inconsistent number of observations

		auto itN = this->Nodes.begin();
		list<Neighbour_connection*>::iterator it_neigh;
		auto itN_end = this->Nodes.end();
		for (itN; itN != itN_end; itN++) {
			for (it_neigh = (*itN)->Disabled_connections.begin(); it_neigh != (*itN)->Disabled_connections.end(); it_neigh++)
				(*itN)->Active_connections.push_back(*it_neigh);
			(*itN)->Disabled_connections.clear();
		}

		bool not_managed;
		bool found_not_managed = false;
		list<Neighbour_connection*>::iterator it_neigh2;
		this->Last_observation_set.clear();
		for (auto it_var = new_observed_vars.begin(); it_var != new_observed_vars.end(); it_var++) {
			not_managed = true;
			for (itN = this->Nodes.begin(); itN != itN_end; itN++) {
				if ((*itN)->pVariable == *it_var) {
					for (it_neigh = (*itN)->Active_connections.begin(); it_neigh != (*itN)->Active_connections.end(); it_neigh++) {
						(*itN)->Disabled_connections.push_back(*it_neigh);

						for (it_neigh2 = (*it_neigh)->Neighbour->Active_connections.begin();
							it_neigh2 != (*it_neigh)->Neighbour->Active_connections.end(); it_neigh2++) {
							if ((*it_neigh2)->Neighbour == *itN) {
								(*it_neigh)->Neighbour->Disabled_connections.push_back(*it_neigh2);
								it_neigh2 = (*it_neigh)->Neighbour->Active_connections.erase(it_neigh2);
								break;
							}
						}
					}
					(*itN)->Active_connections.clear();

					this->Last_observation_set.push_back(observation_info());
					this->Last_observation_set.back().Involved_node = *itN;
					this->Last_observation_set.back().Value = 0;

					not_managed = false;
					break;
				}
			}

			if (not_managed) found_not_managed = true;
		}

		//recompute all neighbourhood
		for (itN = this->Nodes.begin(); itN != itN_end; itN++)
			Neighbour_connection::Recompute_Neighboorhoods((*itN)->Active_connections);

		this->Recompute_clusters();

		if (found_not_managed) throw 2; //detected at least one variable passed as input, not present in this graph

		this->Set_Evidences(new_observed_vals);

	}

	void Node::Node_factory::Set_Evidences(const std::list<size_t>& new_observed_vals) {

		if (new_observed_vals.size() != this->Last_observation_set.size())  throw 0; //Inconsistent number of observations

		auto it_val = new_observed_vals.begin();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			if (*it_val >= it->Involved_node->pVariable->size()) throw 1;//found at least one inconsistent evidence, i.e. with a value greater then the variable size (0 was assumed)
			it_val++;
		}
		it_val = new_observed_vals.begin();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			it->Value = *it_val;
			it_val++;
		}
		if (this->Last_propag_info != NULL) delete this->Last_propag_info;
		this->Last_propag_info = NULL;

		//delete all previous temporary messages
		list<Potential*>::iterator it_temporary;
		for (auto itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++) {
			for (it_temporary = (*itN)->Temporary_Unary.begin();
				it_temporary != (*itN)->Temporary_Unary.end(); it_temporary++) {
				delete *it_temporary;
			}
			(*itN)->Temporary_Unary.clear();
		}

		Potential* message_reduced;
		list<Neighbour_connection*>::iterator it_conn;
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			//compute the temporary messages produced by this observation
			for (it_conn = it->Involved_node->Disabled_connections.begin(); it_conn != it->Involved_node->Disabled_connections.end(); it_conn++) {
				message_reduced = new Potential({ it->Value }, { it->Involved_node->pVariable }, *(*it_conn)->Shared_potential);
				(*it_conn)->Neighbour->Temporary_Unary.push_back(message_reduced);
			}
		}

	}

	void Node::Node_factory::Get_Actual_Hidden_Set(std::list<Categoric_var*>* result) const {

		result->clear();

		if (this->Last_hidden_clusters.empty() && this->Last_observation_set.empty() ) {
			for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) result->push_back((*it)->pVariable);
		}
		else {
			list<Node*>::const_iterator itN;
			for (auto itC = this->Last_hidden_clusters.begin(); itC != this->Last_hidden_clusters.end(); itC++) {
				for (itN = itC->begin(); itN != itC->end(); itN++)
					result->push_back((*itN)->pVariable);
			}
		}

	}

	void Node::Node_factory::Get_Actual_Observation_Set_Var(std::list<Categoric_var*>* result) const {

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) 
			result->push_back(it->Involved_node->pVariable);

	}

	void Node::Node_factory::Get_Actual_Observation_Set_Val(std::list<size_t>* result) const {

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++)
			result->push_back(it->Value);

	}

	void Node::Node_factory::Get_All_variables_in_model(std::list<Categoric_var*>* result) const {

		result->clear();
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			result->push_back((*it)->pVariable);
		}

	}

	void Node::Node_factory::Belief_Propagation(const bool& sum_or_MAP) {

		if (this->Nodes.empty()) {
			cout << "warning: asked belief propagation on an empty structure\n";
			return; 
		}

		if (this->Last_hidden_clusters.empty()) this->Recompute_clusters(); 
			
		if (this->Belief_Propagation_Redo_checking(sum_or_MAP)) return;

	//reset all messages for nodes in the hidden set
		auto it_cl = this->Last_hidden_clusters.begin();
		list<Node*>::iterator it_it_cl;
		list<Neighbour_connection*>::iterator it_conn;
		for (it_cl; it_cl != this->Last_hidden_clusters.end(); it_cl++) {
			for (it_it_cl = it_cl->begin(); it_it_cl != it_cl->end(); it_it_cl++) {
				for (it_conn = (*it_it_cl)->Active_connections.begin(); it_conn != (*it_it_cl)->Active_connections.end(); it_conn++) {
					if ((*it_conn)->Message_to_this_node != NULL) {
						delete (*it_conn)->Message_to_this_node;
						(*it_conn)->Message_to_this_node = NULL;
					}
				}
			}
		}

     //perform belief propagation
		bool all_done_within_iter = true;
		for (it_cl = this->Last_hidden_clusters.begin(); it_cl != this->Last_hidden_clusters.end(); it_cl++) {
			if (!I_belief_propagation_strategy::Propagate(*it_cl, sum_or_MAP, this->Iterations_4_belief_propagation))
				all_done_within_iter = false;
		}

		if (this->Last_propag_info == NULL) this->Last_propag_info = new last_belief_propagation_info();
		this->Last_propag_info->Iterations_perfomed = this->Iterations_4_belief_propagation;
		this->Last_propag_info->Terminate_within_iter = all_done_within_iter;
		this->Last_propag_info->Last_was_SumProd_or_MAP = sum_or_MAP;

	}

	bool Node::Node_factory::Belief_Propagation_Redo_checking(const bool& sum_or_MAP) {

		if (this->Last_propag_info != NULL) {
			if (this->Last_propag_info->Last_was_SumProd_or_MAP == sum_or_MAP) {
				if (this->Last_propag_info->Terminate_within_iter) return true;
				else if (this->Last_propag_info->Iterations_perfomed == this->Iterations_4_belief_propagation) return true;
			}
		}
		
		return false;

	}

	void Node::Node_factory::Recompute_clusters() {

		this->Last_hidden_clusters.clear();
		list<Node*> open_set = this->Nodes;
		list<Node*>::iterator it_cluster;
		list<Node*>::iterator it_neigh;
		list<Node*> temp_neigh;
		bool temp;
		while (!open_set.empty()) {
			this->Last_hidden_clusters.push_back(list<Node*>());

			this->Last_hidden_clusters.back().push_back(open_set.front());
			open_set.pop_front();
			it_cluster = this->Last_hidden_clusters.back().begin();
			while (it_cluster != this->Last_hidden_clusters.back().end()) {

				(*it_cluster)->Compute_neighbour_set(&temp_neigh);

				for (it_neigh = temp_neigh.begin(); it_neigh != temp_neigh.end(); it_neigh++) {

					exist_in_list(&temp, this->Last_hidden_clusters.back(), *it_neigh);

					if (!temp) {
						this->Last_hidden_clusters.back().push_back(*it_neigh);
						open_set.remove(*it_neigh);
					}
				}
				it_cluster++;
			}
		}

		auto itt = this->Last_hidden_clusters.begin();
		list<observation_info>::iterator it_last;
		while (itt != this->Last_hidden_clusters.end()) {
			if (itt->size() == 1) {
				temp = false;
				for (it_last = this->Last_observation_set.begin(); it_last != this->Last_observation_set.end(); it_last++) {
					if (it_last->Involved_node == itt->front()) {
						temp = true;
						break;
					}
				}

				if (temp) itt = this->Last_hidden_clusters.erase(itt);
				else itt++;
			}
			else itt++;
		}

	}

	void Node::Node_factory::Get_marginal_distribution(std::vector<float>* result, Categoric_var* var) {

		result->clear();
		this->Belief_Propagation(true);

		list<Node*>::iterator itN;
		for (auto it_cluster = this->Last_hidden_clusters.begin(); it_cluster != this->Last_hidden_clusters.end(); it_cluster++) {
			for (itN = it_cluster->begin(); itN != it_cluster->end(); itN++) {
				if ((*itN)->Get_var() == var) {
					list<Potential*> messages_union;
					(*itN)->Gather_all_Unaries(&messages_union);
					Potential P(messages_union, false);
					P.Get_marginals(result);
					return;
				}
			}
		}

		throw 1; //  You asked the marginals of a variable not in the hidden set
	}

	void Node::Node_factory::MAP_on_Hidden_set(std::list<size_t>* result) {

		result->clear();
		this->Belief_Propagation(false);
		
		vector<float> marginals;
		vector<float>::iterator it_m;
		float max;
		size_t k;

		list<Potential*> messages_union;
		list<Node*>::iterator itN;
		for (auto it = this->Last_hidden_clusters.begin(); it != this->Last_hidden_clusters.end(); it++) {
			for (itN = it->begin(); itN != it->end(); itN++) {
				(*itN)->Gather_all_Unaries(&messages_union);
				Potential P(messages_union, false);
				P.Get_marginals(&marginals);

				//find values maximising marginals
				result->push_back(0);
				it_m = marginals.begin();
				max = *it_m;
				it_m++;
				k = 1;
				for (it_m; it_m != marginals.end(); it_m++) {
					if (*it_m > max) {
						max = *it_m;
						result->back() = k;
					}
					k++;
				}
			}
		}

	}

	void sample_from_discrete(size_t* result, const vector<float>& distr) {

		float r = (float)rand() / (float)RAND_MAX;

		auto it = distr.begin();
		if (r <= *it) {
			*result = 0;
			return;
		}

		float cum = *it;
		it++;
		size_t k = 1;
		for (it; it != distr.end(); it++) {
			cum += *it;
			if (r <= cum) {
				*result = k;
				return;
			}
			k++;
		}

		*result = (distr.size() - 1);
		return;

	}
	struct info_neighbourhood {
		struct info_neigh {
			Potential*     shared_potential;
			Categoric_var* Var;
			size_t	       Var_pos;
		};

		size_t              Involved_var_pos;
		list<info_neigh>	Info;
		list<Potential*>	Unary_potentials;
	};
	void Evolve_samples(size_t* X, list<info_neighbourhood>& Infoes, const int& iterations) {

		list<Potential*> temp;
		list<Potential*>::iterator it_temp;
		list<info_neighbourhood::info_neigh>::iterator it_info;
		vector<float> marginal;
		size_t original_pot;
		auto it = Infoes.begin();

		for (int k = 0; k < iterations; k++) {
			for (it = Infoes.begin(); it != Infoes.end(); it++) {
				temp = it->Unary_potentials;
				original_pot = temp.size();

				for (it_info = it->Info.begin(); it_info != it->Info.end(); it_info++)
					temp.push_back(new  Potential({ X[it_info->Var_pos] }, { it_info->Var }, *it_info->shared_potential));

				Potential temp_pot(temp, false);
				temp_pot.Get_marginals(&marginal);

				sample_from_discrete(&X[it->Involved_var_pos], marginal);

				it_temp = temp.begin();
				advance(it_temp, original_pot);
				for (it_temp; it_temp != temp.end(); it_temp++)
					delete *it_temp;
			}
		}

	};
	void Node::Node_factory::Gibbs_Sampling_on_Hidden_set(std::list<std::list<size_t>>* result, const unsigned int& N_samples, const unsigned int& initial_sample_to_skip) {

		srand((unsigned int)time(NULL));

		result->clear();

		this->Belief_Propagation(true);

		list<info_neighbourhood> Infoes;

		list<Node*>				 neigh_temp;
		list<Node*>::iterator    neigh_temp_it;
		list<Potential*>		 neigh_bin_temp;
		list<Potential*>::iterator neigh_bin_temp_it;
		size_t k;

		list<Node*>::iterator itN;
		list<Categoric_var*> hidden_vars;
		this->Get_Actual_Hidden_Set(&hidden_vars);
		list<Categoric_var*>::iterator itN2;
		for (auto itcl = this->Last_hidden_clusters.begin(); itcl != this->Last_hidden_clusters.end(); itcl++) {
			for (itN = itcl->begin(); itN != itcl->end(); itN++) {
				Infoes.push_back(info_neighbourhood());

				k = 0;
				for (itN2 = hidden_vars.begin(); itN2 != hidden_vars.end(); itN2++) {
					if (*itN2 == (*itN)->pVariable) {
						Infoes.back().Involved_var_pos = k;
						break;
					}
					k++;
				}
				(*itN)->Append_temporary_permanent_Unaries(&Infoes.back().Unary_potentials);

				(*itN)->Compute_neighbour_set(&neigh_temp, &neigh_bin_temp);


				neigh_bin_temp_it = neigh_bin_temp.begin();
				for (neigh_temp_it = neigh_temp.begin(); neigh_temp_it != neigh_temp.end(); neigh_temp_it++) {
					Infoes.back().Info.push_back(info_neighbourhood::info_neigh());

					Infoes.back().Info.back().shared_potential = *neigh_bin_temp_it;
					Infoes.back().Info.back().Var = (*neigh_temp_it)->pVariable;
					k = 0;
					for (itN2 = hidden_vars.begin(); itN2 != hidden_vars.end(); itN2++) {
						if (*itN2 == (*neigh_temp_it)->pVariable) {
							Infoes.back().Info.back().Var_pos = k;
							break;
						}
						k++;
					}
					neigh_bin_temp_it++;
				}
			}
		}

		////////////////////// debug
		//for (auto iitt = Infoes.begin(); iitt != Infoes.end(); iitt++) {
		//	cout << iitt->Involved_var_pos << " ";
		//	for (auto iitt2 = iitt->Info.begin(); iitt2 != iitt->Info.end(); iitt2++) {
		//		cout << iitt2->Var_pos << " ";
		//	}

		//	cout << endl;
		//}
		//////////////////

		size_t hidded_set_size = this->Nodes.size() - this->Last_observation_set.size();

		size_t* X = (size_t*)malloc(sizeof(size_t)*hidded_set_size);
		for (k = 0; k < hidded_set_size; k++)
			X[k] = 0;

		//skip initial samples
		unsigned int n;
		unsigned int n_delta = (int)floorf(initial_sample_to_skip*0.1f);
		if (n_delta == 0) n_delta = 1;

		Evolve_samples(X, Infoes, initial_sample_to_skip);

		for (n = 0; n < N_samples; n++) {
			Evolve_samples(X, Infoes, n_delta);
			result->push_back(list<size_t>());
			for (k = 0; k < hidded_set_size; k++)
				result->back().push_back(X[k]);
		}

		free(X);

	}
	
	Categoric_var* Node::Node_factory::Find_Variable(const std::string& var_name) {

		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			if ((*it)->Get_var()->Get_name().compare(var_name) == 0) 
				return (*it)->Get_var();
		}
		return NULL;

	}

	void Node::Node_factory::__Create_new_node(Categoric_var* var) {

		if (this->bDestroy_Potentials_and_Variables)
			this->Nodes.push_back(new Node(var, false));
		else {
			for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
				if ((*it)->pVariable->Get_name().compare(var->Get_name()) == 0) throw 0;
			}
			this->Nodes.push_back(new Node(var, true));
		}

	}

	void Node::Node_factory::__Insert(Potential_Shape* shape) {

		Potential_Shape* temp = NULL;
		try { temp = this->___Insert(shape); }
		catch (int) {
			temp = NULL;
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != NULL) {
			this->__Simple_shapes.push_back(temp);
		}

	}

	Potential_Exp_Shape* Node::Node_factory::__Insert(Potential_Exp_Shape* exp_shape) {

		Potential_Exp_Shape* temp = NULL;
		try { temp = this->___Insert(exp_shape); }
		catch (int) {
			temp = NULL;
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != NULL) {
			this->__Exponential_shapes.push_back(temp);
		}
		return temp;

	}

	void Node::Node_factory::__Insert(const std::vector< Potential_Shape*>& shapes, const std::vector<std::list< Potential_Exp_Shape*>>& learnable_exp, const std::vector< Potential_Exp_Shape*>& constant_exp) {

		size_t k, K = shapes.size();
		for (k = 0; k < K; k++)
			this->__Insert(shapes[k]);

		K = constant_exp.size();
		for (k = 0; k < K; k++)
			this->__Insert(constant_exp[k]);

		K = learnable_exp.size();
		list<Potential_Exp_Shape*>::const_iterator it, it_end;
		for (k = 0; k < K; k++) {
			it_end = learnable_exp[k].end();
			for (it = learnable_exp[k].begin(); it != it_end; it++)
				this->__Insert(*it);
		}

	}

	void Node::Node_factory::__Absorb(const Node_factory& o) {

		if (&o == this) return;

		if (!this->bDestroy_Potentials_and_Variables) throw 0;

		vector<Potential_Shape*> shapes;
		vector<list<Potential_Exp_Shape*>> learn_exp;
		vector<Potential_Exp_Shape*> const_exp;
		o.__Get_structure(&shapes, &learn_exp, &const_exp);
		this->__Insert(shapes, learn_exp, const_exp);
		
	}

	void Node::Node_factory::__Get_structure(std::vector<Potential_Shape*>* shapes, std::vector<std::list<Potential_Exp_Shape*>>* learnable_exp, std::vector<Potential_Exp_Shape*>* constant_exp) const {

		list_2_vector(shapes, this->__Simple_shapes);
		learnable_exp->clear();
		list_2_vector(constant_exp, this->__Exponential_shapes);

	}

	void Node::Node_factory::__Get_factors_4_energy_eval(std::list<I_Potential*>* pots) const {

		pots->clear();
		auto itB_end = this->Binary_potentials.end();
		for (auto itB = this->Binary_potentials.begin(); itB != itB_end; itB++)
			pots->push_back(*itB);
		list<Potential*>::iterator itP, itP_end;
		auto itN_end = this->Nodes.end();
		for (auto itN = this->Nodes.begin(); itN != itN_end; itN++) {
			itP_end = (*itN)->Permanent_Unary.end();
			for (itP = (*itN)->Permanent_Unary.begin(); itP != itP_end; itP++)
				pots->push_back(*itP);
		}

	}

	size_t	Node::Node_factory::Get_structure_size() const {

		return this->__Exponential_shapes.size() + this->__Simple_shapes.size();

	}

	size_t* Node::Node_factory::__Get_observed_val(Categoric_var* var) {

		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			if (it->Involved_node->pVariable == var) {
				return &it->Value;
				break;
			}
		}
		return NULL;

	}

	void Node::Node_factory::Eval_Energy_function(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval) const {

		std::list<I_Potential*> pots;
		this->__Get_factors_4_energy_eval(&pots);

		__Eval_energy(E_result, comb_to_eval, pots, false);

	}

	void Node::Node_factory::Eval_Energy_function_normalized(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval) const {

		std::list<I_Potential*> pots;
		this->__Get_factors_4_energy_eval(&pots);

		__Eval_energy(E_result, comb_to_eval, pots, true);

	}

	void Node::Node_factory::__Eval_energy(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval, const list<I_Potential*>& factors, const bool& normalized) {

		E_result->clear();
		size_t k, K = comb_to_eval.get_number_of_combinations();
		E_result->reserve(K);
		for (k = 0; k < K; k++) E_result->push_back(1.f);
		vector<const I_Potential::I_Distribution_value*> match;
		list<I_Potential*>::const_iterator it,  it_end = factors.end();
		if (normalized) {
			float E_max;
			for (it = factors.begin(); it != it_end; it++) {
				E_max = (*it)->max_in_distribution();
				comb_to_eval.Find_images_single_matches(&match, **it);
				for (k = 0; k < K; k++) {
					if (match[k] == NULL) (*E_result)[k] = 0.f;
					else (*E_result)[k] *= match[k]->Get_val() / E_max;
				}
			}
		}
		else {
			for (it = factors.begin(); it != it_end; it++) {
				comb_to_eval.Find_images_single_matches(&match, **it);
				for (k = 0; k < K; k++) {
					if (match[k] == NULL) (*E_result)[k] = 0.f;
					else (*E_result)[k] *= match[k]->Get_val();
				}
			}
		}

	}

	void Print_distribution(XML_reader::Tag_readable& pot_tag, const Potential_Shape* pot) {
		
		vector<vector<size_t>> combinations;
		vector<float> vals;
		pot->Get_domain(&combinations);
		pot->Get_images(&vals);
		auto it2 = vals.begin();
		vector<size_t>::iterator it_comb;
		for (auto it = combinations.begin(); it != combinations.end(); it++) {
			auto temp = pot_tag.Add_Nested_and_return_created("Distr_val");
			for (it_comb = it->begin(); it_comb != it->end(); it_comb++) {
				temp.Add_Attribute("v", to_string(*it_comb));
			}
			temp.Add_Attribute("D", to_string(*it2));
			it2++;
		}

	}
	void Node::Node_factory::Reprint(const std::string& target_file) const {

		if (this->Nodes.empty())  throw 0; // asked to print an empty model
		
		ofstream f(target_file);
		if (!f.is_open()) {
			f.close();
			throw 1; // unable to write on the specified file
		}
		f.close();
		
		XML_reader exporter;
		auto exp_root = exporter.Get_root();
	
		list<Categoric_var*> hidden_set;
		list<Categoric_var*> observed_set;
		this->Get_Actual_Hidden_Set(&hidden_set);
		this->Get_Actual_Observation_Set_Var(&observed_set);
		for (auto it = hidden_set.begin(); it != hidden_set.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Variable");
			temp.Add_Attribute("name", (*it)->Get_name());
			temp.Add_Attribute("Size", to_string((*it)->size()));
		}
		for (auto it = observed_set.begin(); it != observed_set.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Variable");
			temp.Add_Attribute("name", (*it)->Get_name());
			temp.Add_Attribute("Size", to_string((*it)->size()));
			temp.Add_Attribute("flag", "O");
		}

		const list<Categoric_var*>* involved_vars = NULL;
		list<Categoric_var*>::const_iterator it_involved_vars;

		vector<Potential_Shape*> shp;
		vector<list<Potential_Exp_Shape*>> clusters;
		vector<Potential_Exp_Shape*> exp_const;
		this->__Get_structure(&shp, &clusters, &exp_const);

		for (auto it = shp.begin(); it != shp.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			involved_vars = (*it)->Get_involved_var();
			for (it_involved_vars = involved_vars->begin(); it_involved_vars != involved_vars->end(); it_involved_vars++) {
				temp.Add_Attribute("var", (*it_involved_vars)->Get_name());
			}
			Print_distribution(temp, *it);
		}

		for (auto it = exp_const.begin(); it != exp_const.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			involved_vars = (*it)->Get_involved_var();
			for (it_involved_vars = involved_vars->begin(); it_involved_vars != involved_vars->end(); it_involved_vars++) {
				temp.Add_Attribute("var", (*it_involved_vars)->Get_name());
			}
			temp.Add_Attribute("weight", to_string((*it)->get_weight()));
			temp.Add_Attribute("tunability", "N");
			Print_distribution(temp, (*it)->Get_wrapped_Shape());
		}

		for (auto it_cl = clusters.begin(); it_cl != clusters.end(); it_cl++) {
			for (auto it = it_cl->begin(); it != it_cl->end(); it++) {
				auto temp = exp_root.Add_Nested_and_return_created("Potential");
				involved_vars = (*it)->Get_involved_var();
				for (it_involved_vars = involved_vars->begin(); it_involved_vars != involved_vars->end(); it_involved_vars++) {
					temp.Add_Attribute("var", (*it_involved_vars)->Get_name());
				}
				temp.Add_Attribute("weight", to_string((*it)->get_weight()));
				Print_distribution(temp, (*it)->Get_wrapped_Shape());

				if (it!=it_cl->begin()) {
					 auto temp2 = temp.Add_Nested_and_return_created("Share");
					 for (auto itV = it_cl->front()->Get_involved_var()->begin();
						 itV != it_cl->front()->Get_involved_var()->end(); itV++) {
						 temp2.Add_Attribute("var", (*itV)->Get_name());
					 }
				}
			}
		}

		exporter.Reprint(target_file);

	}

}