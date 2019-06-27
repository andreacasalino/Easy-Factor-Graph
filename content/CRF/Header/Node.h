//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_NODE_H__
#define __CRF_NODE_H__

#include "Potential.h"
#include "../XML_reader/XML_Manager.h"

namespace Segugio {

	class I_belief_propagation_strategy;
	class Node {
	public:
		~Node();


		struct Neighbour_connection {
			friend class Node;
			friend class I_belief_propagation_strategy;

			~Neighbour_connection() { if (this->Message_to_this_node != NULL) delete this->Message_to_this_node; };
		private:
			static void Recompute_Neighboorhoods(std::list<Neighbour_connection*>& connections);
		// data
			Node*								This_Node;
			Node*								Neighbour;

			Message_Unary*						Message_to_this_node;       //NULL when the message is not already available
			Message_Unary**						Message_to_neighbour_node;

			Potential*							Shared_potential;

			std::list<Neighbour_connection*>    Neighbourhood;
		};

		/*!
		 * \brief Interface for describing a net: set of nodes representing random variables
		 */
		class Node_factory {
		public:
			virtual ~Node_factory();

			/** \brief Returns a pointer to the variable in this graph with that name.
			* \details Returns NULL when the variable is not present in the graph
			* @param[in] var_name name to search
			*/
			Categoric_var*			  Find_Variable(const std::string& var_name);
			/** \brief Returns a pointer to the variable in this graph with the same name of the variable passed as input
			* \details Returns NULL when the variable is not present in the graph
			* @param[in] var_with_same_name variable having the same of name of the variable to search
			*/
			Categoric_var*			  Find_Variable(Categoric_var* var_with_same_name) { return this->Find_Variable(var_with_same_name->Get_name()); };
			/** \brief Returns the current set of hidden variables
			*/
			void					  Get_Actual_Hidden_Set(std::list<Categoric_var*>* result);
			/** \brief Returns the current set of observed variables
			*/
			void					  Get_Actual_Observation_Set(std::list<Categoric_var*>* result);
			/** \brief Returns the set of all variable contained in the net
			*/
			void					  Get_All_variables_in_model(std::list<Categoric_var*>* result);
			/** \brief Returns the marginal probabilty of the variable passed P(var|model, observations), 
			* \details on the basis of the last observations set (see Node_factory::Set_Observation_Set_var)
			*/
			void					  Get_marginal_distribution(std::list<float>* result, Categoric_var* var);
			/** \brief Returns the Maximum a Posteriori estimation of the hidden set.
			* \details Values are ordered as returned by Node_factory::Get_Actual_Hidden_Set.
			* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
			*/
			void					  MAP_on_Hidden_set(std::list<size_t>* result);
			/** \brief Returns a set of samples of the conditional distribution P(hidden variables | model, observed variables).
			* \details Samples are obtained through Gibbs sampling. 
			* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
			* @param[in] N_samples number of desired samples
			* @param[in] initial_sample_to_skip number of samples to skip for performing Gibbs sampling
			* @param[out] result returned samples: every element of the list is a combination of values for the hidden set, with the same
			* order returned when calling Node_factory::Get_Actual_Hidden_Set
			*/
			void					  Gibbs_Sampling_on_Hidden_set(std::list<std::list<size_t>>* result, const unsigned int& N_samples, const unsigned int& initial_sample_to_skip); //on the basis of last observed values set
			/** \brief Returns the current value adopted when performing a loopy belief propagation
			*/
			unsigned int			  Get_Iteration_4_belief_propagation() { return this->Iterations_4_belief_propagation; };
			/** \brief Returns the value to adopt when performing a loopy belief propagation
			*/
			void					  Set_Iteration_4_belief_propagation(const unsigned int& iter_to_use);
			
			/** \brief Returns the logartihmic value of the energy function.
			* \details Energy function E=Pot_1(Y_1,2,....,n) * Pot_2(Y_1,2,....,n) .... * Pot_m(Y_1,2,....,n). The combinations passed as input
			* must contains values for all the variables present in this graph.
			* @param[out] result
			* @param[in] combination set of values in the combination for which the energy function has to be eveluated
			* @param[in] var_order_in_combination order of variables considered when assembling combination. They must be references 
			* to the variables actually wrapped by this graph.
			*/
			void Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination);
			/** \brief Same as Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination), 
			* passing a list instead of an array Node_factory::Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination)
			*/
			void Eval_Log_Energy_function(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination);

			/** \brief Similar as Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination),
			* but computing the Energy function normalized: E_norm = E(Y_1,2,....,n) / max possible { E }. E_norm is in [0,1]. The logarthmic value of E_norm is actually returned
			*/
			void Eval_Log_Energy_function_normalized(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination);

			/** \brief Similar as Eval_Log_Energy_function(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination),
			* but computing the Energy function normalized. 
			*/
			void Eval_Log_Energy_function_normalized(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination);

			/** \brief Returns the logarithmic value of the ripartition function Z. Prob(comb) = E(comb) / Z. E is the energy function see Node_factory::Eval_Energy_function. 
			* \details For generic graphs, Z is a function Z(model), while for conditional random field is a function Z(model, X_observations). 
			* Z is always recomputed, considering actual structure of the net: avoid calling multiple times for generic graphs don't varying between
			* two intermediate calls.
			*/
			virtual void			  Get_Log_Z(float* Z);

			/** \brief Returns the attual values set observations. This function can be invokated after a call to void Set_Observation_Set_val(const std::list<size_t>& new_observed_vals)
			*/
			void					  Get_Observation_Set_val(std::list<size_t>* result);
		protected:
			Node_factory(const bool& use_cloning_Insert) : mState(0), Last_propag_info(NULL), Iterations_4_belief_propagation(1000), bDestroy_Potentials_and_Variables(use_cloning_Insert) {};

			//Import XML is not inlined in constructor since contains a call to Insert, which is virtual
			void Import_from_XML(XML_reader* xml_data, const std::string& prefix_config_xml_file);

			//when the passed potential involves two variable is interpreted as a new edge, when containing a single a variable is assumed as a new unary potential;
			//in any other cases is a an error
			virtual void Insert(Potential_Shape* pot) = 0;
			virtual void Insert(Potential_Exp_Shape* pot) = 0;

			void Insert(const std::list<Potential_Shape*>& set_to_insert);
			void Insert(const std::list<Potential_Exp_Shape*>& set_to_insert);
			void Insert(const std::list<Potential_Exp_Shape*>& set_exp_to_insert, const std::list<Potential_Shape*>& set_to_insert);

			template<typename T>
			T* Insert_with_size_check(T* pot) { //returns the potential actually inserted in the graph (shape or exponential) cloned or not

				T* inserted_to_return = NULL;

				size_t var_numb = pot->Get_involved_var_safe()->size();
				if (var_numb == 1) {
					//unary potential insertion
					Categoric_var* varU = pot->Get_involved_var_safe()->front();
					auto itN = this->Nodes.begin();
					bool node_found = false;

					for (itN; itN != this->Nodes.end(); itN++) {
						if ((*itN)->Get_var()->Get_name().compare(varU->Get_name()) == 0) {

							if (this->bDestroy_Potentials_and_Variables)
								inserted_to_return = new T(pot, { (*itN)->Get_var() });
							else
								inserted_to_return = pot;

							(*itN)->Permanent_Unary.push_back(new Potential(inserted_to_return));
							node_found = true;
							break;
						}
					}

					if (!node_found) {
						system("ECHO the unary potential provided refers to an inexistent node");
						abort();
					}
				}
				else if (var_numb == 2) {
					//binary potential insertion
					Node* peer_A = NULL;
					Node* peer_B = NULL;

					Categoric_var* varA = pot->Get_involved_var_safe()->front();
					auto itN = this->Nodes.begin();
					for (itN; itN != this->Nodes.end(); itN++) {
						if ((*itN)->Get_var()->Get_name().compare(varA->Get_name()) == 0) {
							peer_A = *itN;
							break;
						}
					}
					Categoric_var* varB = pot->Get_involved_var_safe()->back();
					itN = this->Nodes.begin();
					for (itN; itN != this->Nodes.end(); itN++) {
						if ((*itN)->Get_var()->Get_name().compare(varB->Get_name()) == 0) {
							peer_B = *itN;
							break;
						}
					}

					if ((peer_A != NULL) && (peer_B != NULL)) {
						//check this potential was not already inserted
						const std::list<Categoric_var*>* temp_var;
						for (auto it_bb = this->Binary_potentials.begin();
							it_bb != this->Binary_potentials.end(); it_bb++) {
							temp_var = (*it_bb)->Get_involved_var_safe();

							if (((peer_A->Get_var() == temp_var->front()) && (peer_B->Get_var() == temp_var->back())) ||
								((peer_A->Get_var() == temp_var->back()) && (peer_B->Get_var() == temp_var->front()))) {
								system("ECHO found clone of an already inserted binary potential");
								abort();
							}
						}
					}

					if (peer_A == NULL) {
						if (this->bDestroy_Potentials_and_Variables)
							this->Nodes.push_back(new Node(varA));
						else
							this->Nodes.push_back(new Node(varA, true));
						peer_A = this->Nodes.back();
					}
					if (peer_B == NULL) {
						if (this->bDestroy_Potentials_and_Variables)
							this->Nodes.push_back(new Node(varB));
						else
							this->Nodes.push_back(new Node(varB, true));
						peer_B = this->Nodes.back();
					}

					if (this->bDestroy_Potentials_and_Variables)
						inserted_to_return = new T(pot, { peer_A->Get_var(), peer_B->Get_var() });
					else
						inserted_to_return = pot;

					Potential* new_pot = new Potential(inserted_to_return);
					//create connection
					Node::Neighbour_connection* A_B = new Node::Neighbour_connection();
					A_B->This_Node = peer_A;
					A_B->Neighbour = peer_B;
					A_B->Shared_potential = new_pot;
					A_B->Message_to_this_node = NULL;

					Node::Neighbour_connection* B_A = new Node::Neighbour_connection();
					B_A->This_Node = peer_B;
					B_A->Neighbour = peer_A;
					B_A->Shared_potential = new_pot;
					B_A->Message_to_this_node = NULL;

					A_B->Message_to_neighbour_node = &B_A->Message_to_this_node;
					B_A->Message_to_neighbour_node = &A_B->Message_to_this_node;

					peer_A->Active_connections.push_back(A_B);
					peer_B->Active_connections.push_back(B_A);

					this->Binary_potentials.push_back(new_pot);

				}
				else {
					system("ECHO invalid component to insert in a graph");
					abort();
				}

				if (this->Last_propag_info != NULL) delete this->Last_propag_info;
				this->Last_propag_info = NULL;
				this->mState = 0;

				return inserted_to_return;

			};

			Node*					  Find_Node(const std::string& var_name);
			
		//methods having an effect on mState

			/** \brief Set the values for the observations. Must call after calling Node_factory::Set_Observation_Set_val
			*/
			void					  Set_Observation_Set_var(const std::list<Categoric_var*>& new_observed_vars);
			/** \brief Set the observation set: which variables are treated like evidence when performing belief propagation
			*/
			void					  Set_Observation_Set_val(const std::list<size_t>& new_observed_vals); //assuming the same observed set last time
			void					  Belief_Propagation(const bool& sum_or_MAP);

			size_t*					  Get_observed_val_in_case_is_in_observed_set(Categoric_var* var); //return NULL in case the involved variable is hidden

			void					  Recompute_Log_Z(float* result, std::list<size_t>& new_observed_vals, std::list<Categoric_var*>& new_observed_vars);
		private:
			bool					  Belief_Propagation_Redo_checking(const bool& sum_or_MAP);
			void					  Recompute_clusters();

			struct observation_info {
				Node*  Involved_node;
				size_t Value;
			};

			struct last_belief_propagation_info {
				bool          Terminate_within_iter;
				unsigned int  Iterations_perfomed;
				bool          Last_was_SumProd_or_MAP;
			};

		// data
			bool								bDestroy_Potentials_and_Variables;

			size_t								mState;
			last_belief_propagation_info*		Last_propag_info;

			std::list<std::list<Node*>>			Last_hidden_clusters;
			std::list<observation_info>			Last_observation_set;

			std::list<Node*>					Nodes;
			std::list<Potential*>				Binary_potentials;

			unsigned int						Iterations_4_belief_propagation;
		};


		Categoric_var*							Get_var() { return this->pVariable; };
		void									Gather_all_Unaries(std::list<Potential*>* result); 
		void									Append_temporary_permanent_Unaries(std::list<Potential*>* result); //append to existing list all the temporary and permanent uanry potentials, excluding messages
		void									Append_permanent_Unaries(std::list<Potential*>* result);

		const std::list<Neighbour_connection*>* Get_Active_connections() { return &this->Active_connections; };
		void									Compute_neighbour_set(std::list<Node*>* Neigh_set);
		void									Compute_neighbour_set(std::list<Node*>* Neigh_set, std::list<Potential*>* binary_involved);
		void									Compute_neighbourhood_messages(std::list<Potential*>* messages, Node* node_involved_in_connection);
	private:
		Node(Categoric_var* var) { this->pVariable = new Categoric_var(var->size(), var->Get_name()); };
		Node(Categoric_var* var, const bool& dont_clone_var) { this->pVariable = var; };
	// data
		Categoric_var*					  pVariable;
		std::list<Potential*>			  Permanent_Unary;
		std::list<Potential*>			  Temporary_Unary; //those coming from marginalizing some observed variables
		std::list<Neighbour_connection*>  Active_connections;
		std::list<Neighbour_connection*>  Disabled_connections;
	};






	class I_belief_propagation_strategy {
	public:
		virtual ~I_belief_propagation_strategy() {};

		static bool Propagate(std::list<Node*>& cluster, const bool& sum_or_MAP = true, const unsigned int& Iterations = 1000);
	protected:
		void Instantiate_message(Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP);
		void Update_message(float* variation_to_previous, Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP);
		void Gather_incoming_messages(std::list<Potential*>* result, Node::Neighbour_connection* outgoing_mex_to_compute);
	// getter
		std::list<Node::Neighbour_connection*>* Get_Neighbourhood(Node::Neighbour_connection* conn) { return &conn->Neighbourhood; };
		Message_Unary**							Get_Mex_to_This(Node::Neighbour_connection* conn) { return &conn->Message_to_this_node; };
		Message_Unary**							Get_Mex_to_Neigh(Node::Neighbour_connection* conn) { return conn->Message_to_neighbour_node; };
	};

}

#endif