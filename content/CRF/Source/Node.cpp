#include "../Header/Node.h"
#include <time.h>
using namespace std;

//#include <iostream> //only for debugging
namespace Segugio {

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
			if ((*it)->Message_to_this_node == NULL) {
				system("ECHO Found NULL incoming messages not computed");
				abort();
			}
#endif // _DEBUG
			result->push_back((*it)->Message_to_this_node);
		}

#ifdef _DEBUG
		if (result->empty()) {
			system("ECHO empty set of messages found");
			abort();
		}
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
					if ((*it_N)->Message_to_this_node == NULL) {
						system("ECHO Found NULL incoming messages not computed");
						abort();
					}
#endif // _DEBUG
					messages->push_back((*it_N)->Message_to_this_node);
				}
				return;
			}
		}

		system("ECHO inexsistent neighbour node");
		abort();


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

		if (iter_to_use < 10) {
			system("ECHO iterations for belief propagation are too low");
			abort();
		}

		this->Iterations_4_belief_propagation = iter_to_use;

	}

	Categoric_var* Find_by_name(list<Categoric_var*>& vars, const string& name) {

		for (auto it = vars.begin(); it != vars.end(); it++) {
			if ((*it)->Get_name().compare(name) == 0)
				return *it;
		}
		return NULL;

	};

	Potential_Shape* Import_shape(const string& prefix, XML_reader::Tag_readable& tag, const list<Categoric_var*>& vars) {

		if (tag.Exist_Field("Source"))
			return new Potential_Shape(vars, prefix + tag.Get_value("Source"));
		else if (tag.Exist_Field("Correlation")) {
			if (tag.Get_value("Correlation").compare("T") == 0)
				return new Potential_Shape(vars, true);
			else if(tag.Get_value("Correlation").compare("F") == 0)
				return new Potential_Shape(vars, false);
			else {
				system("ECHO invalid correlation type");
				abort();
			}
		}
		else {
			Potential_Shape* shape = new Potential_Shape(vars);
			list<XML_reader::Tag_readable> distr_vals;
			tag.Get_Nested("Distr_val", &distr_vals);
			list<string> indices_raw;
			list<size_t> indices;
			while (!distr_vals.empty()) {
				indices.clear();
				distr_vals.front().Get_values_specific_field_name("v", &indices_raw);

				while (!indices_raw.empty()) {
					indices.push_back((size_t)atoi(indices_raw.front().c_str()));
					indices_raw.pop_front();
				}

				shape->Add_value(indices, (float)atof(distr_vals.front().Get_value("D").c_str()));
				distr_vals.pop_front();
			}
			return shape;
		}

	};
	void Node::Node_factory::Import_from_XML(XML_reader* reader, const std::string& prefix_config_xml_file) {

		this->bDestroy_Potentials_and_Variables = false;

		auto root = reader->Get_root();

		//import variables
		list<Categoric_var*>	variables;
		Categoric_var* temp_clone, * temp_clone2;
		list<XML_reader::Tag_readable> Var_tags;
		root.Get_Nested("Variable", &Var_tags);
		string var_name;
		for (auto itV = Var_tags.begin(); itV != Var_tags.end(); itV++) {
			var_name = itV->Get_value("name");
			temp_clone = Find_by_name(variables, var_name);

			if (temp_clone != NULL) {
				system("ECHO found clone of a varibale when parsing xml graph");
				abort();
			}

			variables.push_back(new Categoric_var((size_t)atoi(itV->Get_value("Size").c_str()), var_name));

		}

		//import potentials
		list<XML_reader::Tag_readable> Pot_tags;
		root.Get_Nested("Potential", &Pot_tags);
		list<string> var_names;

		list<Potential_Shape*>	   una_pot;
		list<Potential_Shape*>	   bin_pot;
		list<Potential_Exp_Shape*> una_exp_pot;
		list<Potential_Exp_Shape*> bin_exp_pot;

		list<Categoric_var*> processed_vars;
		float w_temp;
		bool b_temp;
		for (auto itP = Pot_tags.begin(); itP != Pot_tags.end(); itP++) {
			itP->Get_values_specific_field_name("var", &var_names);
			if (var_names.size() == 1) { //new unary potential to read
				temp_clone = Find_by_name(variables, var_names.front());
				if (temp_clone == NULL) {
					system("ECHO potential refers to an inexistent variable");
					abort();
				}
				if (itP->Exist_Field("weight")) {
					w_temp = (float)atof(itP->Get_value("weight").c_str());
					if (w_temp < 0.f) {
						system("ECHO weight of potential must be positive");
						abort();
					}
					una_exp_pot.push_back(new Potential_Exp_Shape(Import_shape(prefix_config_xml_file, *itP, { temp_clone }), w_temp));
				}
				else
					una_pot.push_back(Import_shape(prefix_config_xml_file, *itP, { temp_clone }));

				exist_in_list(&b_temp, processed_vars, temp_clone);
				if (!b_temp) processed_vars.push_back(temp_clone);
			}
			else if (var_names.size() == 2) { //new binary potential to read
				temp_clone = Find_by_name(variables, var_names.front());
				if (temp_clone == NULL) {
					system("ECHO potential refers to an inexistent variable");
					abort();
				}
				exist_in_list(&b_temp, processed_vars, temp_clone);
				if (!b_temp) processed_vars.push_back(temp_clone);

				temp_clone2 = Find_by_name(variables, var_names.back());
				if (temp_clone2 == NULL) {
					system("ECHO potential refers to an inexistent variable");
					abort();
				}
				exist_in_list(&b_temp, processed_vars, temp_clone2);
				if (!b_temp) processed_vars.push_back(temp_clone2);

				if (itP->Exist_Field("weight")) {
					w_temp = (float)atof(itP->Get_value("weight").c_str());
					if (w_temp < 0.f) {
						system("ECHO weight of potential must be positive");
						abort();
					}
					bin_exp_pot.push_back(new Potential_Exp_Shape(Import_shape(prefix_config_xml_file, *itP, { temp_clone, temp_clone2 }), w_temp));
				}
				else
					bin_pot.push_back(Import_shape(prefix_config_xml_file, *itP, { temp_clone, temp_clone2 }));

			}
			else {
				system("ECHO valid potentials must refer only to 1 or 2 variables");
				abort();
			}
		}

		if (processed_vars.size() != variables.size()) {
			system("ECHO some variables declared in the config file were not included in any potentials");
			abort();
		}

		if (bin_pot.empty() && bin_exp_pot.empty()) {
			system("ECHO at least one binary potential must be present in a config file");
			abort();
		}

		this->Insert(bin_pot);
		this->Insert(bin_exp_pot);

		for (auto it = una_pot.begin(); it != una_pot.end(); it++) this->Insert(*it);
		for (auto it = una_exp_pot.begin(); it != una_exp_pot.end(); it++) this->Insert(*it);

		this->bDestroy_Potentials_and_Variables = true;

	}





	Categoric_var* Node::Node_factory::Find_Variable(const std::string& var_name) {

		return this->Find_Node(var_name)->pVariable;

	}

	Node* Node::Node_factory::Find_Node(const std::string& var_name) {

		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			if ((*it)->Get_var()->Get_name().compare(var_name) == 0)
				return *it;
		}

		system("ECHO Node::Node_factory::Find_Node, inexistent name");
		abort();

	}

	void Node::Node_factory::Set_Observation_Set_var(const std::list<Categoric_var*>& new_observed_vars) {

		if (this->mState == 0) {
			this->mState = 1;
		}
		/*else if (this->mState = 1) {

		}*/
		else if (this->mState == 2) {
			if (new_observed_vars.empty() && this->Last_observation_set.empty()) {
				return;
			}

			if (this->Last_propag_info != NULL) delete this->Last_propag_info;
			this->Last_propag_info = NULL;
			this->mState = 1;
		}

		auto itN = this->Nodes.begin();
		list<Neighbour_connection*>::iterator it_neigh;
		for (itN; itN != this->Nodes.end(); itN++) {
			for (it_neigh = (*itN)->Disabled_connections.begin(); it_neigh != (*itN)->Disabled_connections.end(); it_neigh++)
				(*itN)->Active_connections.push_back(*it_neigh);
			(*itN)->Disabled_connections.clear();
		}

		bool not_managed;
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

					not_managed = false;
					break;
				}
			}

			if (not_managed) {
				system("ECHO inexistent variable observed");
				abort();
			}
		}

		//recompute all neighbourhood
		for (itN = this->Nodes.begin(); itN != this->Nodes.end(); itN++)
			Neighbour_connection::Recompute_Neighboorhoods((*itN)->Active_connections);

		this->Recompute_clusters();

	}

	void Node::Node_factory::Set_Observation_Set_val(const std::list<size_t>& new_observed_vals) {

		if (this->mState == 0) {
			system("ECHO you cannot set values before defining observation set");
			abort();
		}
		else if (this->mState == 1) {
			this->mState = 2;
		}
		else if (this->mState == 2) {
			if (this->Last_propag_info != NULL) delete this->Last_propag_info;
			this->Last_propag_info = NULL;
		}


		if (new_observed_vals.size() != this->Last_observation_set.size()) {
			system("ECHO Inconsistent number of observations");
			abort();
		}

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
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			it->Value = *it_val;

			//compute the temporary messages produced by this observation
			for (it_conn = it->Involved_node->Disabled_connections.begin(); it_conn != it->Involved_node->Disabled_connections.end(); it_conn++) {
				message_reduced = new Potential({ it->Value }, { it->Involved_node->pVariable }, (*it_conn)->Shared_potential);
				(*it_conn)->Neighbour->Temporary_Unary.push_back(message_reduced);
			}

			it_val++;
		}

	}

	void Node::Node_factory::Get_Actual_Hidden_Set(std::list<Categoric_var*>* result) {

#ifdef _DEBUG
		if (this->mState == 0) {
			abort();
		}
#endif // DEBUG

		result->clear();
		list<Node*>::iterator itN;
		for (auto itC = this->Last_hidden_clusters.begin(); itC != this->Last_hidden_clusters.end(); itC++) {
			for (itN = itC->begin(); itN != itC->end(); itN++)
				result->push_back((*itN)->pVariable);
		}

	}

	void Node::Node_factory::Get_Actual_Observation_Set(std::list<Categoric_var*>* result) {

#ifdef _DEBUG
		if (this->mState == 0) {
			abort();
		}
#endif // DEBUG

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++)
			result->push_back(it->Involved_node->pVariable);

	}

	void Node::Node_factory::Get_All_variables_in_model(std::list<Categoric_var*>* result) {

		result->clear();
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			result->push_back((*it)->pVariable);
		}

	}

	void Node::Node_factory::Belief_Propagation(const bool& sum_or_MAP) {

		if (this->Nodes.empty()) {
			system("ECHO empty structure");
			abort();
		}

		if (this->mState == 0) {
			this->Set_Observation_Set_var({});
			this->Set_Observation_Set_val({});
			this->mState = 2;
		}
		else if (this->mState == 1) {
			system("ECHO You tried to make belief propoagation before setting observations");
			abort();
		}
		else if (this->mState == 2) {
			if (this->Belief_Propagation_Redo_checking(sum_or_MAP)) return;
		}

		//cout << "new propagation \n";

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

		system("ECHO You Asked to inference for a variable not in the hidden set");
		abort();

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

	size_t* Node::Node_factory::Get_observed_val_in_case_is_in_observed_set(Categoric_var* var) {

		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++) {
			if (it->Involved_node->Get_var() == var) return &it->Value;
		}

		return NULL;
		
	}

	template<typename T>
	void extract_consistent_order(list<T*>* result, const list<T*>& set_to_insert) {

		if (set_to_insert.empty()) return;

		auto open_set = set_to_insert;
		auto it = set_to_insert.begin();
		auto it2 = it;
		result->push_back(open_set.front());
		open_set.pop_front();
		size_t old_size;
		bool insert;
		bool matching[4];
		while (true) {
			if (open_set.empty()) break;

			old_size = open_set.size();

			it = open_set.begin();
			while (it != open_set.end()) {
				insert = false;
				for (it2 = result->begin(); it2 != result->end(); it2++) {
					matching[0] = ((*it2)->Get_involved_var_safe()->front() == (*it)->Get_involved_var_safe()->front());
					matching[1] = ((*it2)->Get_involved_var_safe()->back() == (*it)->Get_involved_var_safe()->front());
					matching[2] = ((*it2)->Get_involved_var_safe()->front() == (*it)->Get_involved_var_safe()->back());
					matching[3] = ((*it2)->Get_involved_var_safe()->back() == (*it)->Get_involved_var_safe()->back());

					if (matching[0] || matching[1] || matching[2] || matching[3]) {
						insert = true;
						break;
					}
				}

				if (insert) {
					result->push_back(*it);
					it = open_set.erase(it);
				}
				else it++;
			}

			if (old_size == open_set.size()) {
				system("ECHO inconsistent set to add CRF");
				abort();
			}
		}

	}

	void Node::Node_factory::Insert(const std::list<Potential_Shape*>& set_to_insert) {

		list<Potential_Shape*> temp;
		extract_consistent_order(&temp, set_to_insert);
		for (auto it = temp.begin(); it != temp.end(); it++)
			this->Insert(*it);

	}

	void Node::Node_factory::Insert(const std::list<Potential_Exp_Shape*>& set_to_insert) {

		list<Potential_Exp_Shape*> temp;
		extract_consistent_order(&temp, set_to_insert);
		for (auto it = temp.begin(); it != temp.end(); it++)
			this->Insert(*it);

	}

	void Node::Node_factory::Insert(const std::list<Potential_Exp_Shape*>& set_exp_to_insert, const std::list<Potential_Shape*>& set_to_insert) {

		struct temp_wrapper {
			temp_wrapper(Potential_Shape* to_wrap) : Shape_Exp(NULL), Shape(to_wrap) {};
			temp_wrapper(Potential_Exp_Shape* to_wrap) : Shape_Exp(to_wrap), Shape(NULL) {};

			Potential_Shape*      Shape;
			Potential_Exp_Shape*  Shape_Exp;

			const std::list<Categoric_var*>*			Get_involved_var_safe() const { return this->pwrapped->Get_involved_var_safe(); };
		private:
			I_Potential*		  pwrapped;
		};

		list<temp_wrapper*> temp, temp_ordered;
		for (auto it = set_exp_to_insert.begin(); it != set_exp_to_insert.end(); it++)
			temp.push_back(new temp_wrapper(*it));
		for (auto it = set_to_insert.begin(); it != set_to_insert.end(); it++)
			temp.push_back(new temp_wrapper(*it));

		extract_consistent_order(&temp_ordered, temp);

		auto it = temp_ordered.begin();
		for (it; it != temp_ordered.end(); it++) {
			if ((*it)->Shape == NULL)
				this->Insert((*it)->Shape_Exp);
			else
				this->Insert((*it)->Shape);
		}
		for (it = temp_ordered.begin(); it != temp_ordered.end(); it++)
			delete *it;

	}

	void Node::Node_factory::Get_Observation_Set_val(std::list<size_t>* result) {

		if (this->mState != 2) {
			system("ECHO you cannot get observation vals before setting the values");
			abort();
		}

		result->clear();
		for (auto it = this->Last_observation_set.begin(); it != this->Last_observation_set.end(); it++)
			result->push_back(it->Value);

	}

	void Node::Node_factory::Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination) {

		*result = 0.f;
		list<float> matching;

		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			(*it)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);
#ifdef _DEBUG
			if (matching.size() != 1) abort();
#endif
			if (matching.front() != 0.f) *result += logf(matching.front());
		}

		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				(*it_U)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);
#ifdef _DEBUG
				if (matching.size() != 1) abort();
#endif
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

	void Node::Node_factory::Eval_Log_Energy_function_normalized(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination) {

		*result = 0.f;
		list<float> matching;

		//binary potentials
		for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
			(*it)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);
#ifdef _DEBUG
			if (matching.size() != 1) abort();
#endif
			if (matching.front() != 0.f) *result += logf(matching.front() / (*it)->max());
		}

		//permanent unary potentials
		list<Potential*>::iterator it_U;
		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); it++) {
			for (it_U = (*it)->Permanent_Unary.begin(); it_U != (*it)->Permanent_Unary.end(); it_U++) {
				(*it_U)->Find_Comb_in_distribution(&matching, { combination }, var_order_in_combination);
#ifdef _DEBUG
				if (matching.size() != 1) abort();
#endif
				if (matching.front() != 0.f) *result += logf(matching.front() / (*it_U)->max());
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

	void Node::Node_factory::Get_Log_Z(float* Z) {

		list<size_t>		 val_ob_old;
		this->Get_Observation_Set_val(&val_ob_old);
		list<Categoric_var*> var_ob_old;
		this->Get_Actual_Observation_Set(&var_ob_old);

		list<size_t>		 fake_vals;
		list<Categoric_var*> fake_vars;
		this->Recompute_Log_Z(Z, fake_vals, fake_vars);

		this->Set_Observation_Set_var(var_ob_old);
		this->Set_Observation_Set_val(val_ob_old);

	}

	void get_pos_of_max(size_t* p, const list<float>& L) {

		*p = 0;
		const float* max_val = &L.front();
		auto it = L.begin(); it++;
		size_t k = 1;
		for (it; it != L.end(); it++) {
			if (*it > *max_val) {
				*p = k;
				max_val = &(*it);
			}
			k++;
		}

	}
	template<typename T>
	void append_b_to_a(list<T>* a, const list<T>& b) {

		for (auto it = b.begin(); it!=b.end(); it++)
			a->push_back(*it);

	}
	void Node::Node_factory::Recompute_Log_Z(float* result, std::list<size_t>& new_observed_vals, std::list<Categoric_var*>& new_observed_vars) {

		this->Set_Observation_Set_var(new_observed_vars);
		this->Set_Observation_Set_val(new_observed_vals);

		//take the first hidden variable in the set
		Categoric_var* Y = this->Last_hidden_clusters.front().front()->pVariable;

		//compute marginal probabilities P(Y)
		list<float> probs;
		this->Get_marginal_distribution(&probs, Y);
		size_t v_max;
		get_pos_of_max(&v_max, probs);

		//set Y = v_max as an additional evidence
		new_observed_vars.push_back(Y);
		new_observed_vals.push_back(v_max);
		this->Set_Observation_Set_var(new_observed_vars);
		this->Set_Observation_Set_val(new_observed_vals);

		float E;

		list<Categoric_var*> comb_order;
		if (this->Nodes.size() == new_observed_vars.size())
			this->Eval_Log_Energy_function(&E, new_observed_vals, new_observed_vars);		

		else {
			list<size_t> comb_val;
			this->MAP_on_Hidden_set(&comb_val);
			list<Categoric_var*> comb_var;
			this->Get_Actual_Hidden_Set(&comb_var); 

			append_b_to_a(&comb_val , new_observed_vals);
			append_b_to_a(&comb_var, new_observed_vars);

			this->Eval_Log_Energy_function(&E, comb_val, comb_var);
		}

		*result = E - logf(probs.front());

		//clean up
		new_observed_vars.pop_back();
		new_observed_vals.pop_back();
		this->Set_Observation_Set_var(new_observed_vars);
		this->Set_Observation_Set_val(new_observed_vals);

	}

}