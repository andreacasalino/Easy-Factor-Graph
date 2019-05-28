//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_NODE_H__
#define __CRF_NODE_H__

#include "Potential.h"
#include "../XML_reader/XML_importer.h"

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


		class Node_factory {
		public:
			virtual ~Node_factory();

			Categoric_var*			  Find_Variable(const std::string& var_name);
			void					  Get_Actual_Hidden_Set(std::list<Categoric_var*>* result);
			void					  Get_Actual_Observation_Set(std::list<Categoric_var*>* result);
			void					  Get_All_variables_in_model(std::list<Categoric_var*>* result);

			void					  Get_marginal_distribution(std::list<float>* result, Categoric_var* var); //on the basis of last observed values set
			void					  MAP_on_Hidden_set(std::list<size_t>* result); //Maximum A Posteriori Estimation considering all the hidden variables, on the basis of last observed values set
			void					  Gibbs_Sampling_on_Hidden_set(std::list<std::list<size_t>>* result, const unsigned int& N_samples, const unsigned int& initial_sample_to_skip); //on the basis of last observed values set

			unsigned int			  Get_Iteration_4_belief_propagation() { return this->Iterations_4_belief_propagation; };
			void					  Set_Iteration_4_belief_propagation(const unsigned int& iter_to_use);

		protected:
			Node_factory() : mState(0), Last_propag_info(NULL), Iterations_4_belief_propagation(1000) {};
			//Import XML is not inlined in constructor since contains a call to Insert, which is virtual
			void Import_from_XML(XML_reader* xml_data, const std::string& prefix_config_xml_file);

			//when the passed potential involves two variable is interpreted as a new edge, when containing a single a variable is assumed as a new unary potential;
			//in any other cases is a an error
			virtual void Insert(Potential_Shape* pot) { this->Insert_with_size_check<Potential_Shape>(pot); };
			virtual void Insert(Potential_Exp_Shape* pot) { this->Insert_with_size_check<Potential_Exp_Shape>(pot); };

			void Insert(const std::list<Potential_Shape*>& set_to_insert);
			void Insert(const std::list<Potential_Exp_Shape*>& set_to_insert);

			Node*					  Find_Node(const std::string& var_name);

		//methods having an effect on mState
			void					  Set_Observation_Set_var(const std::list<Categoric_var*>& new_observed_vars);
			void					  Set_Observation_Set_val(const std::list<size_t>& new_observed_vals); //assuming the same observed set last time
			void					  Belief_Propagation(const bool& sum_or_MAP);


			size_t*					  Get_observed_val_in_case_is_in_observed_set(Categoric_var* var); //return NULL in case the involved variable is hidden
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


			void Insert(Potential* pot, Categoric_var* varA, Categoric_var* varB);
			void Insert(Potential* pot, Categoric_var* varU);
		//methods having an effect on mState
			template<typename T>
			void Insert_with_size_check(T* pot) {

				auto var_involved = pot->Get_involved_var_safe();

				size_t var_numb = var_involved->size();
				if (var_numb == 1) {
					this->Insert(new Potential(pot), var_involved->front());
				}
				else if (var_numb == 2) {
					auto new_bin = new Potential(pot);
					this->Insert(new_bin, var_involved->front(), var_involved->back());
				}
				else {
					system("ECHO invalid component to insert in a graph");
					abort();
				}

				if (this->Last_propag_info != NULL) delete this->Last_propag_info;
				this->Last_propag_info = NULL;
				this->mState = 0;

			};

		// data
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
		Node(Categoric_var* var) { this->pVariable = var; };
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