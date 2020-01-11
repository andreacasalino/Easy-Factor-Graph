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
		for (auto it = L.begin(); it != L.end(); it++) {
			if (*it == candidate) {
				*result = true;
				return;
			}
		}

	};






	void Node::Neighbour_connection::Recompute_Neighboorhoods(list<Neighbour_connection*>& connections) {

		auto it = connections.begin();
		for (it; it != connections.end(); it++)
			(*it)->Neighbourhood.clear();

		list<Neighbour_connection*> processed;
		list<Neighbour_connection*>::iterator it2;
		for (it = connections.begin(); it != connections.end(); it++) {
			(*it)->Neighbourhood = processed;

			for (it2 = processed.begin(); it2 != processed.end(); it2++)
				(*it2)->Neighbourhood.push_back(*it);

			processed.push_back(*it);
		}

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

		for (auto it = this->Active_connections.begin(); it != this->Active_connections.end(); it++) {
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
		for (it; it != this->Permanent_Unary.end(); it++)
			result->push_back(*it);

		for (it = this->Temporary_Unary.begin(); it != this->Temporary_Unary.end(); it++)
			result->push_back(*it);

	}

	void Node::Append_permanent_Unaries(std::list<Potential*>* result) {

		auto it = this->Permanent_Unary.begin();
		for (it; it != this->Permanent_Unary.end(); it++)
			result->push_back(*it);

	}

	void Node::Compute_neighbour_set(std::list<Node*>* Neigh_set) {

		Neigh_set->clear();
		for (auto itn = this->Active_connections.begin(); itn != this->Active_connections.end(); itn++)
			Neigh_set->push_back((*itn)->Neighbour);

	}

	void Node::Compute_neighbour_set(std::list<Node*>* Neigh_set, std::list<Potential*>* binary_in_Neigh_set) {

		Neigh_set->clear();
		binary_in_Neigh_set->clear();
		for (auto itn = this->Active_connections.begin(); itn != this->Active_connections.end(); itn++) {
			Neigh_set->push_back((*itn)->Neighbour);
			binary_in_Neigh_set->push_back((*itn)->Shared_potential);
		}

	}

	void Node::Compute_neighbourhood_messages(std::list<Potential*>* messages, Node* node_involved_in_connection) {

		messages->clear();

		this->Append_temporary_permanent_Unaries(messages);

		for (auto it = this->Active_connections.begin(); it != this->Active_connections.end(); it++) {
			if ((*it)->Neighbour == node_involved_in_connection) {
				for (auto it_N = (*it)->Neighbourhood.begin(); it_N != (*it)->Neighbourhood.end(); it_N++) {
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

		auto itN = this->Nodes.begin();
		if (this->bDestroy_Potentials_and_Variables) {
			for (itN; itN != this->Nodes.end(); itN++) {
				for (auto itU = (*itN)->Permanent_Unary.begin(); itU != (*itN)->Permanent_Unary.end(); itU++)
					delete *itU;

				delete (*itN)->pVariable;
			}

			for (auto itP = this->Binary_potentials.begin(); itP != this->Binary_potentials.end(); itP++)
				delete *itP;
		}

		for (itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++)
			delete *itN;

		if (this->Last_propag_info != NULL) delete this->Last_propag_info;

	}

	void Node::Node_factory::Set_Iteration_4_belief_propagation(const unsigned int& iter_to_use) {

		unsigned int temp = iter_to_use;
		if (temp < 10)
			temp = 10;

		this->Iterations_4_belief_propagation = temp;

	}

	Categoric_var* Find_by_name(list<Categoric_var*>& vars, const string& name) {

		for (auto it = vars.begin(); it != vars.end(); it++) {
			if ((*it)->Get_name().compare(name) == 0)
				return *it;
		}
		return NULL;

	};
	void parse_Variable(list<Categoric_var*>* variables, XML_reader::Tag_readable& tag) {

		const string* val;
		try { val = tag.Get_Attribute_first_found("Size"); }
		catch (int) { throw 0; } //  found unsized variable 
		
		size_t Size = (size_t)atoi(val->c_str());
		if (Size == 0)  throw 1; // found zero dimension variable

		try { val = tag.Get_Attribute_first_found("name"); }
		catch (int) { throw 2; } //  found unamed variable 		
		if (Find_by_name(*variables, *val) != NULL)  throw 3; //  found multiple variables with the same name
		
		variables->push_back(new Categoric_var(Size, *val));

	}
	Potential_Shape* Import_shape(const string& prefix, XML_reader::Tag_readable& tag,  list<Categoric_var*>& vars) {

		list<Categoric_var*> var_involved;
		list<string> names;
		tag.Get_Attributes("var", &names);
		if (names.empty()) throw 0; //found potential with invalid var set
		else if (names.size() == 1) {
			var_involved.push_back(Find_by_name(vars, names.front()));
			if (var_involved.front() == NULL) throw 1; // found potential with invalid var set
		}
		else if (names.size() == 2) {
			var_involved.push_back(Find_by_name(vars, names.front()));
			if (var_involved.back() == NULL) throw 2; // found potential with invalid var set
			
			var_involved.push_back(Find_by_name(vars, names.back()));
			if (var_involved.back() == NULL) throw 3; // found potential with invalid var set
		}

		const string* val;
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
			else throw 4; //found potential with invalid options
		}

		auto shape = new Potential_Shape(var_involved);
		list<XML_reader::Tag_readable> distr_vals;
		tag.Get_Nested("Distr_val", &distr_vals);
		list<string> indices_raw;
		list<size_t> indices;
		const string* temp_D;
		while (!distr_vals.empty()) {
			indices.clear();
			distr_vals.front().Get_Attributes("v", &indices_raw);

			try { temp_D = distr_vals.front().Get_Attribute_first_found("D"); }
			catch (int) { temp_D = NULL; }
			if (temp_D != NULL) {
				while (!indices_raw.empty()) {
					indices.push_back((size_t)atoi(indices_raw.front().c_str()));
					indices_raw.pop_front();
				}

				try { shape->Add_value(indices, (float)atof(temp_D->c_str())); }
				catch (int) { throw 5;  } //inconsistent distribution value
				distr_vals.pop_front();
			}
		}
		return shape;

	};
	void Node::Node_factory::Import_from_XML(XML_reader* reader, const std::string& prefix_config_xml_file) {

		this->bDestroy_Potentials_and_Variables = true;

	//import variables
		list<Categoric_var*>	variables;
		list<XML_reader::Tag_readable> Nested;
		reader->Get_root().Get_Nested("Variable", &Nested);
		for (auto it = Nested.begin(); it != Nested.end(); it++) {
			try { parse_Variable(&variables, *it); }
			catch (int) { throw 0;  } //invalid variable found
		}

	//import potentials
		list<Potential_Shape*>			shp_list;
		list<Potential_Exp_Shape*> exp_list;
		list<bool>									tunability;
		Potential_Shape* temp_shape;
		reader->Get_root().Get_Nested("Potential", &Nested);
		const string* w_temp;
		float w_val_temp;
		const string* tun_temp;
		for (auto it = Nested.begin(); it != Nested.end(); it++) {
			try { temp_shape = Import_shape(prefix_config_xml_file, *it, variables); }
			catch (int) { temp_shape = NULL; }
			
			if (temp_shape == NULL)  throw 0; // found invalid potential
			else if (!temp_shape->get_validity()) throw 1; //found invalid potential
			else {
				try { w_temp = it->Get_Attribute_first_found("weight"); }
				catch (int) { w_temp = NULL; }
				if (w_temp ==NULL) 
					shp_list.push_back(temp_shape);
				else {
					w_val_temp = (float)atof(w_temp->c_str());
					auto temp_exp_shape = new Potential_Exp_Shape(temp_shape, w_val_temp);
					if (!temp_exp_shape->get_validity()) throw 2; // found invalid potential
					else {
						exp_list.push_back(temp_exp_shape);
						tunability.push_back(true);
						try { tun_temp = it->Get_Attribute_first_found("tunability"); }
						catch (int) { tun_temp = NULL; }
						if (tun_temp != NULL) {
							if (tun_temp->compare("N") == 0)
								tunability.back() = false;
						}
					}
				}
			}
		}

		this->Insert(shp_list);
		this->Insert(exp_list, tunability);

		for (auto it = shp_list.begin(); it != shp_list.end(); it++)
			delete *it;
		for (auto it = exp_list.begin(); it != exp_list.end(); it++)
			delete *it;
		for (auto it = variables.begin(); it != variables.end(); it++)
			delete *it;

	}

	Node* Node::Node_factory::__Find_Node(Categoric_var* var) {

		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			if ((*it)->Get_var() == var)
				return *it;
		}

		if (this->bDestroy_Potentials_and_Variables) {
			for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
				if ((*it)->Get_var()->Get_name().compare(var->Get_name()) == 0) {
					if ((*it)->Get_var()->size() == var->size()) return *it;
				}
			}
		}

		return NULL;

	}

	void Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) {

		if (new_observed_vars.size() != new_observed_vals.size())  throw 0; //Inconsistent number of observations

		if (this->Last_propag_info != NULL) delete this->Last_propag_info;
		this->Last_propag_info = NULL;

		auto itN = this->Nodes.begin();
		list<Neighbour_connection*>::iterator it_neigh;
		for (itN; itN != this->Nodes.end(); itN++) {
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
			for (itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++) {
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
		for (itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++)
			Neighbour_connection::Recompute_Neighboorhoods((*itN)->Active_connections);

		this->Recompute_clusters();

		if (found_not_managed) throw 2; //detected at least one variable passed as input, not present in this graph

		this->Set_Evidences(new_observed_vals);

	}

	void Node::Node_factory::Set_Evidences(const std::list<size_t>& new_observed_vals) {

		if (new_observed_vals.size() != this->Last_observation_set.size())  throw 0; //Inconsistent number of observations

		//delete all previous temporary messages
		list<Potential*>::iterator it_temporary;
		for (auto itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++) {
			for (it_temporary = (*itN)->Temporary_Unary.begin();
				it_temporary != (*itN)->Temporary_Unary.end(); it_temporary++) {
				delete *it_temporary;
			}
			(*itN)->Temporary_Unary.clear();
		}

		auto it_val = new_observed_vals.begin();
		Potential* message_reduced;
		list<Neighbour_connection*>::iterator it_conn;
		bool found_inconsistent_evidence = false;
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			if (*it_val < it->Involved_node->pVariable->size()) it->Value = *it_val;

			//compute the temporary messages produced by this observation
			for (it_conn = it->Involved_node->Disabled_connections.begin(); it_conn != it->Involved_node->Disabled_connections.end(); it_conn++) {
				message_reduced = new Potential({ it->Value }, { it->Involved_node->pVariable }, (*it_conn)->Shared_potential);
				(*it_conn)->Neighbour->Temporary_Unary.push_back(message_reduced);
			}

			it_val++;
		}

		if (found_inconsistent_evidence) throw 1; //found at least one inconsistent evidence, i.e. with a value greater then the variable size (0 was assumed)

	}

	void Node::Node_factory::Get_Actual_Hidden_Set(std::list<Categoric_var*>* result) {

		result->clear();

		if (this->Last_hidden_clusters.empty() && this->Last_observation_set.empty() ) {
			for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) result->push_back((*it)->pVariable);
		}
		else {
			list<Node*>::iterator itN;
			for (auto itC = this->Last_hidden_clusters.begin(); itC != this->Last_hidden_clusters.end(); itC++) {
				for (itN = itC->begin(); itN != itC->end(); itN++)
					result->push_back((*itN)->pVariable);
			}
		}

	}

	void Node::Node_factory::Get_Actual_Observation_Set_Var(std::list<Categoric_var*>* result) {

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) 
			result->push_back(it->Involved_node->pVariable);

	}

	void Node::Node_factory::Get_Actual_Observation_Set_Val(std::list<size_t>* result) {

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++)
			result->push_back(it->Value);

	}

	void Node::Node_factory::Get_All_variables_in_model(std::list<Categoric_var*>* result) {

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

	void Node::Node_factory::Get_marginal_distribution(std::list<float>* result, Categoric_var* var) {

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
		
		list<float> marginals;
		list<float>::iterator it_m;
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

	void sample_from_discrete(size_t* result, const list<float>& distr) {

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
		list<float> marginal;
		size_t original_pot;
		auto it = Infoes.begin();

		for (int k = 0; k < iterations; k++) {
			for (it = Infoes.begin(); it != Infoes.end(); it++) {
				temp = it->Unary_potentials;
				original_pot = temp.size();

				for (it_info = it->Info.begin(); it_info != it->Info.end(); it_info++)
					temp.push_back(new  Potential({ X[it_info->Var_pos] }, { it_info->Var }, it_info->shared_potential));

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

	void Node::Node_factory::Create_new_node(Categoric_var* var) {

		if (this->bDestroy_Potentials_and_Variables)
			this->Nodes.push_back(new Node(var));
		else {
			for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
				if ((*it)->pVariable->Get_name().compare(var->Get_name()) == 0) throw 0;
			}
			this->Nodes.push_back(new Node(var, true));
		}

	}

	void Node::Node_factory::Insert(const std::list<Potential_Exp_Shape*>& exponential_potentials, const std::list<bool>& tunability) {

		if (exponential_potentials.size() != tunability.size()) throw 0; //tunability flags mismatch with number of potentials

		auto it_tun = tunability.begin();
		for (auto it = exponential_potentials.begin(); it != exponential_potentials.end(); it++) {
			this->__Insert(*it, *it_tun);
			it_tun++;
		}

	}

	void Node::Node_factory::Insert(const std::list<Potential_Shape*>& simple_potentials) {

		for (auto it = simple_potentials.begin(); it != simple_potentials.end(); it++) 
			this->__Insert(*it);

	}

	void Node::Node_factory::Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination) {

		*result = 0.f;
		list<float> matching;

		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			(*it)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);

			if (matching.size() != 1) throw 0; //combination not found
	
			if (matching.front() != 0.f) *result += logf(matching.front());
		}

		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				(*it_U)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);

				if (matching.size() != 1) throw 1; //combination not found
				
				if (matching.front() != 0.f) *result += logf(matching.front());
			}
		}

	}

	void Node::Node_factory::Eval_Log_Energy_function(float* result, const list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination) {

		size_t* temp = (size_t*)malloc(combination.size() * sizeof(size_t));
		size_t k = 0;
		for (auto it = combination.begin(); it != combination.end(); it++) {
			temp[k] = *it;
			k++;
		}

		this->Eval_Log_Energy_function(result, temp, var_order_in_combination);

		free(temp);

	}

	void Node::Node_factory::Eval_Log_Energy_function(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination) {

		list<list<float>> distr_vals;
		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			distr_vals.push_back(list<float>());
			(*it)->Find_Comb_in_distribution(&distr_vals.back(), combinations, var_order_in_combination);
		}
		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				distr_vals.push_back(list<float>());
				(*it_U)->Find_Comb_in_distribution(&distr_vals.back(), combinations, var_order_in_combination);
			}
		}

		result->clear();
		auto it_val = distr_vals.begin();
		float temp;
		while (!distr_vals.front().empty()) {
			temp = 0.f;
			for (it_val = distr_vals.begin(); it_val != distr_vals.end(); it_val++) {
				temp += logf(it_val->front());
				it_val->pop_front();
			}
			result->push_back(temp);
		}

	};

	void Node::Node_factory::Eval_Log_Energy_function_normalized(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination) {

		list<list<float>> distr_vals;
		list<float> max_vals;
		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			distr_vals.push_back(list<float>());
			(*it)->Find_Comb_in_distribution(&distr_vals.back(), combinations, var_order_in_combination);
			max_vals.push_back((*it)->max_in_distribution());
		}
		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				distr_vals.push_back(list<float>());
				(*it_U)->Find_Comb_in_distribution(&distr_vals.back(), combinations, var_order_in_combination);
				max_vals.push_back((*it_U)->max_in_distribution());
			}
		}

		result->clear();
		auto it_val = distr_vals.begin();
		auto it_val_max = max_vals.begin();
		float temp;
		while (!distr_vals.front().empty()) {
			temp = 0.f;
			it_val_max = max_vals.begin();
			for (it_val = distr_vals.begin(); it_val != distr_vals.end(); it_val++) {
				temp += logf(it_val->front()) - logf(*it_val_max);
				it_val->pop_front();
				it_val_max++;
			}
			result->push_back(temp);
		}

	}

	void Node::Node_factory::Eval_Log_Energy_function_normalized(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination) {

		*result = 0.f;
		list<float> matching;

		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			(*it)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);

			if (matching.size() != 1) throw 0; // combination not found

			if (matching.front() != 0.f) *result += logf(matching.front()) - logf((*it)->max_in_distribution());
		}

		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				(*it_U)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);

				if (matching.size() != 1) throw 1; // combination not found

				if (matching.front() != 0.f) *result += logf(matching.front()) - logf((*it_U)->max_in_distribution());
			}
		}

	}

	void Node::Node_factory::Eval_Log_Energy_function_normalized(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination) {

		size_t* temp = (size_t*)malloc(combination.size() * sizeof(size_t));
		size_t k = 0;
		for (auto it = combination.begin(); it != combination.end(); it++) {
			temp[k] = *it;
			k++;
		}

		this->Eval_Log_Energy_function_normalized(result, temp, var_order_in_combination);

		free(temp);

	}

	void Node::Node_factory::Get_structure(std::list<const Potential_Shape*>* shapes, std::list<std::list<const Potential_Exp_Shape*>>* learnable_exp, std::list<const Potential_Exp_Shape*>* constant_exp) {

		shapes->clear();
		list<Potential_Shape*> temp_shp;
		this->__Get_simple_shapes(&temp_shp);
		for (auto it = temp_shp.begin(); it != temp_shp.end(); it++)
			shapes->push_back(*it);

		learnable_exp->clear();
		constant_exp->clear();
		list<list<Potential_Exp_Shape*>> temp_clusters;
		list<Potential_Exp_Shape*>				temp_exp;
		this->__Get_exponential_shapes(&temp_clusters, &temp_exp);
		if (!temp_clusters.empty()) {
			auto it2 = temp_clusters.front().begin();
			for (auto it = temp_clusters.begin(); it != temp_clusters.end(); it++) {
				learnable_exp->push_back(list<const Potential_Exp_Shape*>());
				for (it2 = it->begin(); it2 != it->end(); it2++)
					learnable_exp->back().push_back(*it2);
			}
		}
		for (auto it = temp_exp.begin(); it != temp_exp.end(); it++)
			constant_exp->push_back(*it);

	}

	size_t	Node::Node_factory::Get_structure_size() {

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

	void Node::Node_factory::__Absorb(Node_factory* to_absorb) {

		if (to_absorb == this)
			return;

		for (auto it = to_absorb->__Simple_shapes.begin(); it != to_absorb->__Simple_shapes.end(); it++)
			this->__Insert(*it);

		list<list<Potential_Exp_Shape*>> tunable_clusters;
		list<Potential_Exp_Shape*>			constant_exp;
		to_absorb->__Get_exponential_shapes(&tunable_clusters, &constant_exp);

		if (!tunable_clusters.empty()) {
			auto it2 = tunable_clusters.front().begin();
			for (auto it = tunable_clusters.begin(); it != tunable_clusters.end(); it++) {
				for (it2 = it->begin(); it2 != it->end(); it2++) {
					this->__Insert(*it2, true);
				}
			}
		}

		for (auto it = constant_exp.begin(); it != constant_exp.end(); it++)
			this->__Insert(*it, false);

	}

	void Node::Node_factory::__Insert(Potential_Shape* pot) {

		Potential_Shape* temp = NULL;
		try { temp = this->___Insert(pot); }
		catch (int) { 
			temp = NULL; 
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != NULL)
			this->__Simple_shapes.push_back(temp);

	};

	Potential_Exp_Shape* Node::Node_factory::__Insert(Potential_Exp_Shape* pot, const bool& weight_tunability) {

		Potential_Exp_Shape* temp = NULL;
		try { temp = this->___Insert(pot); }
		catch (int) {
			temp = NULL;
			cout << "warning: invalid potential to insert detected\n";
		}
		if (temp != NULL)
			this->__Exponential_shapes.push_back(temp);
		return temp;

	};

	void Node::Node_factory::__Get_exponential_shapes(std::list<std::list<Potential_Exp_Shape*>>* learnable_exp, std::list<Potential_Exp_Shape*>* constant_exp) {

		learnable_exp->clear();
		*constant_exp = this->__Exponential_shapes;

	}

	void Print_distribution(XML_reader::Tag_readable& pot_tag, const Potential_Shape* pot) {
		
		list<list<size_t>> combinations;
		list<float> vals;
		try { pot->Copy_Distribution(&combinations, &vals); }
		catch (int) { throw 0; }
		auto it2 = vals.begin();
		list<size_t>::iterator it_comb;
		for (auto it = combinations.begin(); it != combinations.end(); it++) {
			auto temp = pot_tag.Add_Nested_and_return_created("Distr_val");
			for (it_comb = it->begin(); it_comb != it->end(); it_comb++) {
				temp.Add_Attribute("v", to_string(*it_comb));
			}
			temp.Add_Attribute("D", to_string(*it2));
			it2++;
		}

	}
	void Node::Node_factory::Reprint(const std::string& target_file) {

		if (this->Nodes.empty())  throw 0; // asked to print an empty model
		
		ofstream f(target_file);
		if (!f.is_open()) throw 1; // unable to write on the specified file
		
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

		list<Potential_Shape*> shp;
		this->__Get_simple_shapes(&shp);
		for (auto it = shp.begin(); it != shp.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			involved_vars = (*it)->Get_involved_var_safe();
			for (it_involved_vars = involved_vars->begin(); it_involved_vars != involved_vars->end(); it_involved_vars++) {
				temp.Add_Attribute("var", (*it_involved_vars)->Get_name());
			}
			Print_distribution(temp, *it);
		}

		list<list<Potential_Exp_Shape*>> clusters;
		list<Potential_Exp_Shape*> exp_const;
		this->__Get_exponential_shapes(&clusters, &exp_const);
		for (auto it = exp_const.begin(); it != exp_const.end(); it++) {
			auto temp = exp_root.Add_Nested_and_return_created("Potential");
			involved_vars = (*it)->Get_involved_var_safe();
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
				involved_vars = (*it)->Get_involved_var_safe();
				for (it_involved_vars = involved_vars->begin(); it_involved_vars != involved_vars->end(); it_involved_vars++) {
					temp.Add_Attribute("var", (*it_involved_vars)->Get_name());
				}
				temp.Add_Attribute("weight", to_string((*it)->get_weight()));
				Print_distribution(temp, (*it)->Get_wrapped_Shape());

				if (it!=it_cl->begin()) {
					 auto temp2 = temp.Add_Nested_and_return_created("Share");
					 for (auto itV = it_cl->front()->Get_involved_var_safe()->begin();
						 itV != it_cl->front()->Get_involved_var_safe()->end(); itV++) {
						 temp2.Add_Attribute("var", (*itV)->Get_name());
					 }
				}
			}
		}

		exporter.Reprint(target_file);

	}

}