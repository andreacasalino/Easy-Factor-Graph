//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_NODE_H__
#define __CRF_NODE_H__

#include "Potential.h"
#include "../../thirdy_part_lib/XML_reader/XML_Manager.h"

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
			* \details Returns NULL when the variable is not present in the graph.
			* @param[in] var_name name to search
			*/
			Categoric_var*			  Find_Variable(const std::string& var_name);
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
			/** \brief Returns the Maximum a Posteriori estimation of the hidden set. @latexonly\label{MAP_method}@endlatexonly
			* \details Values are ordered as returned by Node_factory::Get_Actual_Hidden_Set.
			* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
			*/
			void					  MAP_on_Hidden_set(std::list<size_t>* result);
			/** \brief Returns a set of samples of the conditional distribution P(hidden variables | model, observed variables). @latexonly\label{Gibbs_method}@endlatexonly 
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
			* passing a list instead of an array size_t*, a list<size_t> for describing the combination for which you want to evaluate the energy.
			*/
			void Eval_Log_Energy_function(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination);
			/** \brief Same as Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination),
			* passing a list of combinations: don't iterate yourself many times using 
			Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination), but call this function
			*/
			void Eval_Log_Energy_function(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination);

			/** \brief Similar as Eval_Log_Energy_function(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination),
			* but computing the Energy function normalized: E_norm = E(Y_1,2,....,n) / max possible { E }. E_norm is in [0,1]. The logarthmic value of E_norm is actually returned
			*/
			void Eval_Log_Energy_function_normalized(float* result, size_t* combination, const std::list<Categoric_var*>& var_order_in_combination);
			/** \brief Similar as Eval_Log_Energy_function(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination),
			* but computing the Energy function normalized. 
			*/
			void Eval_Log_Energy_function_normalized(float* result, const std::list<size_t>& combination, const std::list<Categoric_var*>& var_order_in_combination);
			/** \brief Similar as Eval_Log_Energy_function(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination),
			* but computing the Energy function normalized.
			*/
			void Eval_Log_Energy_function_normalized(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination);

			/** \brief Returns the attual values set observations. This function can be invokated after a call to void Set_Observation_Set_val(const std::list<size_t>& new_observed_vals)
			*/
			void					  Get_Observation_Set_val(std::list<size_t>* result);

			/*!
			 * \brief Returns the list of potentials constituting the net. Usefull for structural learning
			 */
			void					 Get_structure(std::list<const Potential*>* structure);
			/*!
			 * \brief Returns the number of potentials constituting the graph, no matter of their type (simple shape, exponential shape fixed or exponential shape tunable)
			 */
			size_t					 Get_structure_size();


			class _SubGraph {
			public:
				/*!
				 * \brief Builds a reduction of the actual net, considering the actual observation values.
				 * \details The subgraph is not automatically updated w.r.t. modifications of the originating net: in such cases just create a novel subgraph with the same sub_set of variables involved
				 */
				_SubGraph(Node_factory* Original_graph , const std::list<Categoric_var*>& sub_set_to_consider);
				~_SubGraph();

				/** \brief Returns a pointer to the variable in this graph with that name.
				* \details Returns NULL when the variable is not present in the graph.
				* @param[in] var_name name to search
				*/
				Categoric_var*			  Find_Variable(const std::string& var_name);

				/** \brief Returns the set of all variable contained in the net
				*/
				void					  Get_All_variables_in_model(std::list<Categoric_var*>* result);

				/** \brief Returns the marginal probabilty of a some particular combinations of values assumed by the variables in this subgraph.
				* \details The marginal probabilities computed are conditioned to the observations set when extracting this subgraph.
				* @param[out] result the computed marginal probabilities
				* @param[in] combinations combinations of values for which the marginals are computed: must have same size of var_order_in_combination.
				* @param[in] var_order_in_combination order of variables considered when assembling the combinations.
				*/
				void					Get_marginal_prob_combinations(std::list<float>* result, const std::list < std::list<size_t>>& combinations, const std::list<Categoric_var*>& var_order_in_combination);
				/** \brief Similar to Get_marginal_prob_combinations(std::list<float>* result, const std::list < std::list<size_t>>& combinations, const std::list<Categoric_var*>& var_order_in_combination),
				* passing the combinations as pointer arrays. 
				*/
				void					Get_marginal_prob_combinations(std::list<float>* result, const std::list<size_t*>& combinations, const std::list<Categoric_var*>& var_order_in_combination);
				/** \brief Returns the Maximum a Posteriori estimation of the hidden set in the sugraph. @latexonly\label{MAP_sub_method}@endlatexonly
				* \details Values are ordered as returned by _SubGraph::Get_All_variables. This MAP
				* is conditioned to the observations set at the time this subgraph was created.
				*/
				void					MAP(std::list<size_t>* result);
				/** \brief Returns a set of samples for the variables involved in this subgraph. @latexonly\label{Gibbs_sub_method}@endlatexonly
				* \details Sampling is done considering the marginal probability distribution of this cluster of 
				* variables, conditioned to the observations set at the time this subgraph was created.
				* Samples are obtained through Gibbs sampling.
				* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
				* @param[in] N_samples number of desired samples
				* @param[in] initial_sample_to_skip number of samples to skip for performing Gibbs sampling
				* @param[out] result returned samples: every element of the list is a combination of values for the hidden set, with the same
				* order returned when calling _SubGraph::Get_All_variables
				*/
				void					Gibbs_Sampling(std::list<std::list<size_t>>* result, const unsigned int& N_samples, const unsigned int& initial_sample_to_skip);
				/** \brief Returns the cluster of varaibles involved in this sub graph.
				*/
				void					Get_All_variables(std::list<Categoric_var*>* result) { this->__SubGraph->Get_All_variables_in_model(result); };
			private:
				_SubGraph(const _SubGraph&) { abort(); };
			// data
				Node_factory*		__SubGraph;
				float*						logZ; //normalization coefficient for computing marginal probabilities. It is a proxy
			};

		protected:
			Node_factory(const bool& use_cloning_Insert) : mState(0), Last_propag_info(NULL), Iterations_4_belief_propagation(1000), bDestroy_Potentials_and_Variables(use_cloning_Insert) {};

			//Import XML is not inlined in constructor since contains a call to Insert, which is virtual
			void Import_from_XML(XML_reader* xml_data, const std::string& prefix_config_xml_file);
			
			Node*					  __Find_Node(Categoric_var* var);
			size_t*					  __Get_observed_val(Categoric_var* var); //return NULL when the variable is not part of the observed set
			
		//methods having an effect on mState

			virtual void __Insert(Potential_Shape* pot) { this->___Insert(pot); };
			virtual Potential_Exp_Shape* __Insert(Potential_Exp_Shape* pot, const bool& weight_tunability) { return this->___Insert(pot); }; //the potential exponential actually inserted is returned
			void Insert(const std::list<Potential_Exp_Shape*>& exponential_potentials, const std::list<bool>& tunability);
			void Insert(const std::list<Potential_Shape*>&				    simple_potentials);

			/** \brief Set the values for the observations. Must call after calling Node_factory::Set_Observation_Set_val
			*/
			void					  Set_Observation_Set_var(const std::list<Categoric_var*>& new_observed_vars);
			/** \brief Set the observation set: which variables are treated like evidence when performing belief propagation
			*/
			void					  Set_Observation_Set_val(const std::list<size_t>& new_observed_vals); //assuming the same observed set last time
			void					  Belief_Propagation(const bool& sum_or_MAP, bool* is_propagation_possible);

		private:
			bool					  Belief_Propagation_Redo_checking(const bool& sum_or_MAP);
			void					  Recompute_clusters();

			bool					  Create_new_node(Categoric_var* var);
			template<typename T>
			Potential* Get_for_insertion(T* pot, T** actually_used, const std::list<Categoric_var*>& var_to_use) {

				*actually_used = pot;
				if (this->bDestroy_Potentials_and_Variables) 
					*actually_used = new T(pot, var_to_use);
				return new Potential(*actually_used);

			};
			template<typename T>
			T* ___Insert(T* pot) { //returns a reference to the real potential inserted

				if (!pot->get_validity()) {
#ifdef _DEBUG
					system("ECHO Insertion failed");
#endif // DEBUG
					return NULL;
				}

				T* inserted = NULL;
				auto var_involved = pot->Get_involved_var_safe();
				if (var_involved->size() == 1) {
//unary potential insertion
					auto node = this->__Find_Node(var_involved->front());
					if (node == NULL) {
						if (this->Create_new_node(var_involved->front()))
							this->Nodes.back()->Permanent_Unary.push_back(this->Get_for_insertion(pot, &inserted, { this->Nodes.back()->Get_var() }));
						else return NULL;
					}
					else  node->Permanent_Unary.push_back(this->Get_for_insertion(pot, &inserted, { node->Get_var() } ));
				}
				else if (var_involved->size() == 2) {
//binary potential insertion
					Node* peer_A = this->__Find_Node(pot->Get_involved_var_safe()->front());
					Node* peer_B = this->__Find_Node(pot->Get_involved_var_safe()->back());

					if ((peer_A != NULL) && (peer_B != NULL)) {
						// check whether this binary potential was already present
						auto var_A = peer_A->Get_var();
						auto var_B = peer_B->Get_var();
						const std::list<Categoric_var*>*	  vars_temp;
						for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
							vars_temp = (*it)->Get_involved_var_safe();
							if ((  (vars_temp->front()->Get_name().compare(peer_A->Get_var()->Get_name()) == 0)  &&  (vars_temp->back()->Get_name().compare(peer_B->Get_var()->Get_name()) == 0)   ) ||
								((vars_temp->front()->Get_name().compare(peer_B->Get_var()->Get_name()) == 0) && (vars_temp->back()->Get_name().compare(peer_A->Get_var()->Get_name()) == 0))) {
#ifdef _DEBUG
								system("ECHO found clone of an already inserted binary potential");
#endif // DEBUG
								return NULL;
							}
						}
					}

					if (peer_A == NULL) {
						if (!this->Create_new_node(pot->Get_involved_var_safe()->front()))
							return NULL;
						peer_A = this->Nodes.back();
					}
					if (peer_B == NULL) {
						if (!this->Create_new_node(pot->Get_involved_var_safe()->back()))
							return NULL;
						peer_B = this->Nodes.back();
					}

					auto new_pot = this->Get_for_insertion(pot, &inserted, { peer_A->Get_var() , peer_B->Get_var() });

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
#ifdef _DEBUG
					system("ECHO Only binary or unary potential can be inserted");
#endif // DEBUG
					return NULL;
				}

				if (this->Last_propag_info != NULL) delete this->Last_propag_info;
				this->Last_propag_info = NULL;
				this->mState = 0;
				return inserted;

			}


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
			bool															bDestroy_Potentials_and_Variables;

			size_t															mState;
			last_belief_propagation_info*				Last_propag_info;

			std::list<std::list<Node*>>					Last_hidden_clusters;
			std::list<observation_info>					Last_observation_set;

			std::list<Node*>										Nodes;
			std::list<Potential*>								Binary_potentials;

			unsigned int												Iterations_4_belief_propagation;
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
		Categoric_var*									 pVariable;
		std::list<Potential*>							 Permanent_Unary;
		std::list<Potential*>							 Temporary_Unary; //those coming from marginalizing some observed variables
		std::list<Neighbour_connection*>  Active_connections;
		std::list<Neighbour_connection*>  Disabled_connections;
	};



	typedef Node::Node_factory::_SubGraph  SubGraph;




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