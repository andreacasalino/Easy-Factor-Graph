/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF_NODE_H__
#define __CRF_NODE_H__

#include "Potential.h"
#include "../XML_Manager/XML_Manager.h"

namespace EFG {

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
			Node*											This_Node;
			Node*											Neighbour;

			Potential*										Message_to_this_node;       //NULL when the message is not already available
			Potential**										Message_to_neighbour_node;

			Potential*										Shared_potential;

			std::list<Neighbour_connection*>				Neighbourhood;
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
			void					  Get_Actual_Hidden_Set(std::list<Categoric_var*>* result) const;
			/** \brief Returns the current set of observed variables
			*/
			void					  Get_Actual_Observation_Set_Var(std::list<Categoric_var*>* result) const;
			/** \brief Returns the current  observations
			*/
			void					  Get_Actual_Observation_Set_Val(std::list<size_t>* result) const;
			/** \brief Returns the set of all variable contained in the net
			*/
			void					  Get_All_variables_in_model(std::list<Categoric_var*>* result) const;
			/** \brief Returns the marginal probabilty of the variable passed P(var|model, observations), 
			* \details on the basis of the last observations set (see Node_factory::Set_Observation_Set_var)
			*/
			void					  Get_marginal_distribution(std::vector<float>* result, Categoric_var* var);
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
			const unsigned int&	Get_Iteration_4_belief_propagation()  const { return this->Iterations_4_belief_propagation; };
			/** \brief Returns the value to adopt when performing a loopy belief propagation
			*/
			void					  Set_Iteration_4_belief_propagation(const unsigned int& iter_to_use);
			
			/** \brief Returns the energy function values in when considering the passed combinations.
			* \details Clearly, the variables in this model and the ones in the passed combination must have a correspondance.
			The values assumed by all the variables in the model, also the observed ones, must be specified in the combinations.
			* @param[in] comb_to_eval the combinations for which the energy function must be computed
			* @param[out] E_result the computed values for the energy function
			*/
			void			          Eval_Energy_function(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval) const;

			/** \brief Similar to Node_factory::Eval_Energy_function, but computing the normalized energy function.
			* \details Normally, the energy function is obtained as a product of coefficients E1 x E2 X E3..., where each Ei is the 
			value assumed by the i-th potential, for a certain combination of values. When dealing with the normalized version,
			each term is normalize by the maximal possible value assumed by i-th potential. The normalized energy function can 
			be used as an estimate of the porbability of a certain combination.
			* @param[in] comb_to_eval the combinations for which the normalized energy function must be computed
			* @param[out] E_result the computed values for the energy function
			*/
			void		   Eval_Energy_function_normalized(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval) const;

			/*!
			 * \brief Returns the list of potentials constituting the net.
			 \details The potentials returned cannot be used for initializing a model. For performing such a task
			 * you can build an empty  model and then use Absorb.
			 * 
			 * @param[out] shapes list of Simple shapes contained in the  model
			 * @param[out] learnable_exp list of Exponential tunable potentials contained in the model: every sub group share the same weight
			 * @param[out] constant_exp list of Exponential constant potentials contained in the model
			 */
			void					 Get_structure(std::vector<Potential_Shape*>* shapes, std::vector<std::list<Potential_Exp_Shape*>>* learnable_exp, std::vector<Potential_Exp_Shape*>* constant_exp) const { this->__Get_structure(shapes, learnable_exp, constant_exp); };
			/*!
			 * \brief Returns the number of potentials constituting the graph, no matter of their type (simple shape, exponential shape fixed or exponential shape tunable)
			 */
			size_t					 Get_structure_size() const;

			/*!
			 * \brief Print an xml file describing the actual structure of the net.
			 *
			 * @param[in] target_file the name of the file were to print the net
			 */
			void					Reprint(const std::string& target_file) const;

			class _SubGraph_t;

		protected:
			Node_factory(const bool& use_cloning_Insert) : Last_propag_info(NULL), Iterations_4_belief_propagation(1000), bDestroy_Potentials_and_Variables(use_cloning_Insert) {};
			Node_factory(const Node_factory& o);

			void					  __copy(const Node_factory& o); //not put in the copy constructor because inside there is call to a virtual function

			virtual void			  __Get_structure(std::vector<Potential_Shape*>* shapes, std::vector<std::list<Potential_Exp_Shape*>>* learnable_exp, std::vector<Potential_Exp_Shape*>* constant_exp) const;

			virtual void			  __Insert(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp);
			void					  __Insert(Potential_Shape* shape);
			Potential_Exp_Shape*	  __Insert(Potential_Exp_Shape* exp_shape); //returns the potetial actually inserted in this graph.

			void					  __Absorb(const Node_factory& o);
			//Import XML is not inlined in constructor since contains a call to Insert, which is virtual
			void					  Import_from_XML(XML_reader* reader, const std::string& prefix_config_xml_file);
			Node*					  __Find_Node(Categoric_var* var) const;
			size_t*					  __Get_observed_val(Categoric_var* var); //return NULL when the variable is not part of the observed set
		
			/** \brief Set the evidences: identify the variables in the hidden set and the values assumed
			\details When passing both input as empty list, all the evidences are deleted.
			 *
			 * @param[in] new_observed_vars the set of variables to consider as evidence
			 * @param[in] new_observed_vals the values assumed as evidences
			*/
			void					  Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals);
			/** \brief Similar to Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
			\details Here only the evidence values are set, assuming the last oberved set imposed
			*/
			void					  Set_Evidences(const std::list<size_t>& new_observed_vals);

			void					  Belief_Propagation(const bool& sum_or_MAP);

		private:
			bool					  Belief_Propagation_Redo_checking(const bool& sum_or_MAP);
			void					  Recompute_clusters();

			void					  __Create_new_node(Categoric_var* var);
			template<typename T>
			Potential*				  __Get_for_insertion(T* pot, T** actually_used, const std::list<Categoric_var*>& var_to_use) {

				*actually_used = pot;
				if (this->bDestroy_Potentials_and_Variables) 
					*actually_used = new T(*pot, var_to_use);
				return new Potential(**actually_used);

			};
			template<typename T>
			T*						  ___Insert(T* pot) { //returns a reference to the real potential inserted

				T* inserted = NULL;
				auto var_involved = pot->Get_involved_var();
				if (var_involved->size() == 1) {
//unary potential insertion
					auto node = this->__Find_Node(var_involved->front());
					if (node == NULL) {
						try { this->__Create_new_node(var_involved->front()); }
						catch (int) { return NULL; }
						this->Nodes.back()->Permanent_Unary.push_back(this->__Get_for_insertion(pot, &inserted, { this->Nodes.back()->Get_var() }));
					}
					else  node->Permanent_Unary.push_back(this->__Get_for_insertion(pot, &inserted, { node->Get_var() } ));
				}
				else if (var_involved->size() == 2) {
					if (pot->Get_involved_var()->front()->Get_name().compare(pot->Get_involved_var()->back()->Get_name()) == 0) throw 1; //you tried to insert binary referring to the same variable

//binary potential insertion
					Node* peer_A = this->__Find_Node(pot->Get_involved_var()->front());
					Node* peer_B = this->__Find_Node(pot->Get_involved_var()->back());

					if ((peer_A != NULL) && (peer_B != NULL)) {
						// check whether this binary potential was already present
						auto var_A = peer_A->Get_var();
						auto var_B = peer_B->Get_var();
						const std::list<Categoric_var*>*	  vars_temp;
						for (auto it = this->Binary_potentials.begin(); it != this->Binary_potentials.end(); it++) {
							vars_temp = (*it)->Get_involved_var();
							if ((  (vars_temp->front()->Get_name().compare(peer_A->Get_var()->Get_name()) == 0)  &&  (vars_temp->back()->Get_name().compare(peer_B->Get_var()->Get_name()) == 0)   ) ||
								((vars_temp->front()->Get_name().compare(peer_B->Get_var()->Get_name()) == 0) && (vars_temp->back()->Get_name().compare(peer_A->Get_var()->Get_name()) == 0))) 
								throw 2; //found clone of an already inserted binary potential
						}
					}

					if (peer_A == NULL) {
						try { this->__Create_new_node(pot->Get_involved_var()->front()); }
						catch (int) { return NULL; }
						peer_A = this->Nodes.back();
					}
					if (peer_B == NULL) {
						try { this->__Create_new_node(pot->Get_involved_var()->back()); }
						catch (int) { return NULL; }
						peer_B = this->Nodes.back();
					}

					auto new_pot = this->__Get_for_insertion(pot, &inserted, { peer_A->Get_var() , peer_B->Get_var() });

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
				else throw 3; //Only binary or unary potential can be inserted

				if (this->Last_propag_info != NULL) delete this->Last_propag_info;
				this->Last_propag_info = NULL;
				this->Potential_observers.emplace_back(inserted->Cast_to_Subject(), (void*)this);
				return inserted;

			};

			void			  __Get_factors_4_energy_eval(std::list<I_Potential*>* pots) const; // used for evaluating the energy function

			static void		  __Eval_energy(std::vector<float>* E_result, const I_Potential::combinations& comb_to_eval, const std::list<I_Potential*>& factors, const bool& normalized = false);

			struct observation_info {
				Node*  Involved_node;
				size_t Value;
			};

			struct last_belief_propagation_info {
				bool					Terminate_within_iter;
				unsigned int		Iterations_perfomed;
				bool					Last_was_SumProd_or_MAP;
			};

		// data
			Subject												subject_4_subgraphs;
			bool												bDestroy_Potentials_and_Variables;

			last_belief_propagation_info*						Last_propag_info;

			std::list<std::list<Node*>>							Last_hidden_clusters;
			std::list<observation_info>							Last_observation_set;

			std::list<Node*>									Nodes;
			std::list<Potential*>								Binary_potentials;

			unsigned int										Iterations_4_belief_propagation;

			std::list<Subject::Observer>						Potential_observers;

		//these list are used only for having a direct reference to the differet kind of potentials in the model
			std::list<Potential_Shape*>							__Simple_shapes;
			std::list<Potential_Exp_Shape*>						__Exponential_shapes;
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
		Node(Categoric_var* var, const bool& dont_clone_var);
	// data												
		Categoric_var*									 pVariable;
		std::list<Potential*>							 Permanent_Unary;
		std::list<Potential*>							 Temporary_Unary; //those coming from marginalizing some observed variables
		std::list<Neighbour_connection*>				 Active_connections;
		std::list<Neighbour_connection*>				 Disabled_connections;
	};



	class I_belief_propagation_strategy {
	public:
		virtual ~I_belief_propagation_strategy() {};

		static bool Propagate(std::list<Node*>& cluster, const bool& sum_or_MAP = true, const unsigned int& Iterations = 1000);
	protected:
		class Message_Unary : public Potential {
		public:
			// Creates a Message with all 1 as values for the image
			Message_Unary(Categoric_var* var_involved);
			/* Firstly, all potential_to_merge are merged together using Potential::Potential(potential_to_merge, false) obtaining a merged potential.
			* Secondly, the product of binary_to_merge and the merged potential is obtained.
			* Finally the message is obtained by marginalizing from the second product, the variable of potential_to_merge, adopting a sum or a MAP.
			* Exploited by message passing algorithms
			*/
			Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
			/* Same as Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true),
			* but in the case potential_to_merge is empty
			*/
			Message_Unary(Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true); //use this when the unary set is empty

			void Update(float* diff_to_previous, Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
			void Update(float* diff_to_previous, Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true);
		private:
			Potential_Shape* merge_binary_and_unary(Potential* binary_to_merge, Potential* unary, const bool& Sum_or_MAP);
			static void eval_diff(float* diff, std::list<I_Potential::I_Distribution_value*>* F1, std::list<I_Potential::I_Distribution_value*>* F2);
		};

		void Instantiate_message(Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP);
		void Update_message(float* variation_to_previous, Node::Neighbour_connection* outgoing_mex_to_compute, const bool& sum_or_MAP);
		void Gather_incoming_messages(std::list<Potential*>* result, Node::Neighbour_connection* outgoing_mex_to_compute);
	// getter
		std::list<Node::Neighbour_connection*>* Get_Neighbourhood(Node::Neighbour_connection* conn) { return &conn->Neighbourhood; };
		Potential** Get_Mex_to_This(Node::Neighbour_connection* conn) { return &conn->Message_to_this_node; };
		Potential** Get_Mex_to_Neigh(Node::Neighbour_connection* conn) { return conn->Message_to_neighbour_node; };
	};


}

#endif