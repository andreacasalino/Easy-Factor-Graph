/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_NODE_H__
#define __EFG_NODE_H__

#include <unordered_set>
#include <memory>
#include <univocal_map.h>
#include <Potential.h>
#include <XML_Manager.h>


namespace EFG {

	class Node {
	public:
		~Node();

		class Neighbour_connection;

		class Node_factory;

		inline Categoric_var*								Get_var() const { return this->pVariable; };
		inline const std::list<const I_Potential*>*			Get_Permanent() const { return &this->Permanent_Unary; };
		inline const std::list<Potential_Shape>*			Get_Temporary() const { return &this->Temporary_Unary; };

		/** \brief returns in a unique collection: the permanent potentials, the temporary potentials and all the messages incoming from the other nodess 
		*/
		std::vector<const I_Potential*>						Get_all_Unaries() const;

		inline const std::list<Neighbour_connection*>* 		Get_Active_connections() const { return &this->Active_connections; };
	private:
		Node(Categoric_var* var, const bool& dont_clone_var);

		Node(const Node&) = delete;
		void operator()(const Node& ) = delete;
	// data												
		Categoric_var*									 pVariable;
		std::list<const I_Potential*>					 Permanent_Unary;
		std::list<Potential_Shape>					 	 Temporary_Unary; //those coming from marginalizing some observed variables
		std::list<Neighbour_connection*>				 Active_connections;
		std::list<Neighbour_connection*>				 Disabled_connections;
	};



	class Node::Neighbour_connection {
	public:
		static void init_connection(Node* peer_A, Node* peer_B, const I_Potential& pot_shared);
		~Neighbour_connection() { delete this->Message_to_this_node; };

		inline const I_Potential*		Get_pot() const { return this->Shared_potential; };
		inline Node*					Get_Neighbour() const { return this->Neighbour; };
		inline const I_Potential*		Get_IncomingMessage() const { return this->Message_to_this_node; };
		inline Neighbour_connection*    Get_Linked() const { return this->Linked; };

		void 							Disable();

		inline void						Reset_OutgoingMessage() { delete this->Linked->Message_to_this_node; this->Linked->Message_to_this_node = nullptr; this->Neighbourhood.clear(); this->was_Neighbourhood_updated = false; };

		bool							Is_OutgoingMessage_Recomputation_possible();
		float							Recompute_OutgoingMessage(const bool& Sum_or_MAP); //returns the difference 

		void							Set_IncomingMessage_ones();
	private:
		Neighbour_connection() {};
		Neighbour_connection(const Neighbour_connection&) = delete;
		void operator()(const Neighbour_connection& ) = delete;

		inline void						__update_Neighbourhood();

		class Message : public I_Potential_Decorator {
		public:
			/* Creates a Message with all 1 as values for the image
			*/
			Message(Categoric_var* var_involved);
			/* Firstly, all potential_to_merge are merged together using Potential::Potential(potential_to_merge, false) obtaining a merged potential.
			* Secondly, the product of binary_to_merge and the merged potential is obtained.
			* Finally the message is obtained by marginalizing from the second product, the variable of potential_to_merge, adopting a sum or a MAP.
			* Exploited by message passing algorithms
			*/
			Message(const I_Potential& binary_pot, const std::vector<const I_Potential*>& unary_to_merge, const bool& Sum_or_MAP) { this->Update(binary_pot, unary_to_merge, Sum_or_MAP); };
			/* Same as Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true),
			* but in the case potential_to_merge is empty
			*/
			Message(const I_Potential& binary_pot, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP){ this->Update(binary_pot, var_to_marginalize, Sum_or_MAP); };

			float Update(const I_Potential& binary_pot, const std::vector<const I_Potential*>& unary_to_merge, const bool& Sum_or_MAP); //returns the difference w.r.t. the previous message
			float Update(const I_Potential& binary_pot, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP); //returns the difference w.r.t. the previous message
		};

	// data
		Node*											Neighbour;
		I_Potential*									Message_to_this_node;       //nullptr when the message is not already available
		Neighbour_connection*							Linked;
		const I_Potential*								Shared_potential;
	// cache
		bool											was_Neighbourhood_updated;
		std::list<Neighbour_connection*>				Neighbourhood;
	};



	class I_belief_propagation_strategy {
	public:
		virtual ~I_belief_propagation_strategy() {};
		virtual bool operator()(std::list< std::unordered_set<Node*, std::function<size_t(const Node*)> > >& cluster, const bool& sum_or_MAP, const unsigned int& max_iterations) = 0; //returns true in case the propagation terminated
		virtual std::unique_ptr<I_belief_propagation_strategy>	copy() const = 0;
	};



	class Node::Node_factory {
	public:
		virtual ~Node_factory();

		/** \brief Returns a pointer to the variable in this graph with that name.
		* \details Returns NULL when the variable is not present in the graph.
		* @param[in] var_name name to search
		*/
		Categoric_var*			  Find_Variable(const std::string& var_name) const;
		/** \brief Returns the current set of hidden variables 
		*/
		std::vector<Categoric_var*> Get_Actual_Hidden_Set() const;
		/** \brief Returns the current set of observed variables
		*/
		std::vector<std::pair<Categoric_var*, size_t>> Get_Actual_Observation_Set() const;
		/** \brief Returns the set of all variable contained in the net
		*/
		std::vector<Categoric_var*> Get_All_variables_in_model() const;

		class Energy_Evaluator;
		inline Energy_Evaluator Get_Energy_Evaluator(const std::vector<Categoric_var*>& vars_order) const;

		typedef std::tuple<std::vector<Potential_Shape*>, std::vector<std::vector<Potential_Exp_Shape*>>, std::vector<Potential_Exp_Shape*>> structure;
		/*!
		* \brief Returns the list of potentials constituting the net.
		\details The potentials returned cannot be used for initializing a model. For performing such a task
		* you can build an empty  model and then use Absorb.
		* 
		* @param[out] shapes list of Simple shapes contained in the  model
		* @param[out] learnable_exp list of Exponential tunable potentials contained in the model: every sub group share the same weight
		* @param[out] constant_exp list of Exponential constant potentials contained in the model
		*/
		virtual const structure Get_structure() const;

		/*!
		* \brief Returns the number of potentials constituting the graph, no matter of their type (simple shape, exponential shape fixed or exponential shape tunable)
		*/
		inline size_t					 Get_structure_size() const { return (this->__Simple_shapes.size() + this->__Exponential_shapes.size()); };

		/** \brief Returns the current value adopted when performing a loopy belief propagation
		*/
		inline const unsigned int&		Get_Iteration_4_ropagation()  const { return this->Propagation_max_iter; };
		/** \brief Returns the value to adopt when performing a loopy belief propagation
		*/
		inline void					  	Set_Iteration_4_propagation(const unsigned int& iter_to_use);

		inline bool						Propagation_completed() const { if(this->Propagation_Last != nullptr) return this->Propagation_Last->Terminated_within_iter; return true; };

		/** \brief Returns the marginal probabilty of the variable passed P(var|model, observations), 
		* \details on the basis of the last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		std::vector<float>				Get_marginal_distribution(const std::string& var);
		/** \brief Returns the Maximum a Posteriori estimation of the hidden set. @latexonly\label{MAP_method}@endlatexonly
		* \details Values are ordered as returned by Node_factory::Get_Actual_Hidden_Set.
		* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
		*/
		size_t							Get_MAP(const std::string& var);

		/** \brief Returns a set of samples of the conditional distribution P(hidden variables | model, observed variables). @latexonly\label{Gibbs_method}@endlatexonly 
		* \details Samples are obtained through Gibbs sampling. 
		* Calculations are done considering the last last observations set (see Node_factory::Set_Observation_Set_var)
		* @param[in] N_samples number of desired samples
		* @param[in] initial_sample_to_skip number of samples to skip for performing Gibbs sampling
		* @param[out] result returned samples: every element of the list is a combination of values for the hidden set, with the same
		* order returned when calling Node_factory::Get_Actual_Hidden_Set
		*/
		std::list<std::vector<size_t>> Gibbs_Sampling_on_Hidden_set(const unsigned int& N_samples, const unsigned int& initial_sample_to_skip, const int& seed = -1); //on the basis of last observed values set

		void							Reprint(const std::string& file_name) const;

		std::unique_ptr<Potential_Shape> 				Get_joint_marginal_distribution(const std::vector<std::string>& subgroup); //the returned shape has the variable in the same order as the ones passed
	protected:

		Node_factory(const bool& use_cloning_Insert, const I_belief_propagation_strategy& propagator);
		Node_factory(const Node_factory& o) = delete;
		void operator=(const Node_factory& ) = delete;

		Node*			  				_Find_Node(const std::string& var_name);

		size_t*							_Find_Observation(const std::string& var_name);

		void		  					_Insert(Potential_Shape* shape); //returns the potetial actually inserted in this graph.
		Potential_Exp_Shape*			_Insert(Potential_Exp_Shape* exp_shape); //returns the potetial actually inserted in this graph.
		virtual void 					_Insert(const structure& strct);

		class Structure_importer;

		/** \brief Set the evidences: identify the variables in the hidden set and the values assumed
		\details When passing both input as empty list, all the evidences are deleted.
			*
			* @param[in] new_observed_vars the set of variables to consider as evidence
			* @param[in] new_observed_vals the values assumed as evidences
		*/
		void					  		_Set_Evidences(const std::vector<std::pair<std::string, size_t>>& new_observations);
		
		/** \brief Similar to Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		\details Here only the evidence values are set, assuming the last oberved set imposed
		*/
		void					  		_Set_Evidences(const std::vector<size_t>& new_observations);

		inline static const I_belief_propagation_strategy& _Get_porpagator(const Node_factory& factory){ return *factory.Propagator; };

		void					  		_Belief_Propagation(const bool& sum_or_MAP); //in the protected version, the propagation is forced to be always re-done
    private:

		template<typename P>
		P*								__Insert(P* to_insert);

		void					  		__Recompute_clusters();		
		void					  		__Belief_Propagation(const bool& sum_or_MAP);

		class Obsv_container : public univocal_map<Node*, std::pair<Node*, size_t>, std::function<Node*(std::pair<Node*, size_t>&)>, std::function<size_t(const Node*)>> {
			std::vector<Node*>  order;
		public:
			Obsv_container();

			void set_vars(const std::vector<Node*>& vars);
			void set_vals(const std::vector<size_t>& vals);
			inline const std::vector<Node*>& get_order() const { return this->order; };
		};
		
		struct belief_propagation_info{
			bool Terminated_within_iter;
			bool Was_Sum_or_MAP;
		};
		
	// data
		Subject_multiObservers															subject_4_subgraphs;
		bool																			bDestroy_Potentials_and_Variables;

		unsigned int 																	Propagation_max_iter;

		belief_propagation_info* 														Propagation_Last;

		std::unique_ptr<I_belief_propagation_strategy>									Propagator;

		std::list<
			std::unordered_set<Node*, std::function<size_t(const Node*)> >
		>																				Last_hidden_clusters;

		Obsv_container																	Last_observation_set;

		univocal_map<const std::string*, Node,
			std::function<const std::string*(Node&)>,
			std::function<size_t(const std::string*)>,
			std::function<bool(const std::string*, const std::string*)>
		>																				Nodes;

		univocal_map< std::pair<const std::string*, const std::string*>, const I_Potential*,
			std::function<std::pair<const std::string*, const std::string*>(const I_Potential*)>,
			std::function<size_t(const std::pair<const std::string*, const std::string*>&)>,
			std::function<bool(const std::pair<const std::string*, const std::string*>&, 
							   const std::pair<const std::string*, const std::string*>&)>
		>																				Binary_potentials;

		std::list<Subject::Observer>													Potential_observers;

	//these list are used only for having a direct reference to the differet kind of potentials in the model
		std::list<Potential_Shape*>														__Simple_shapes;
		std::list<Potential_Exp_Shape*>													__Exponential_shapes;
    };

	template<typename P>
	P* Node::Node_factory::__Insert(P* to_insert){

		class Node_hndl{
			Node_factory*	Source;
		public:
			Node_hndl(Node_factory*	source) : Source(source) {};

			Node* 		  Find(Categoric_var* var){
				auto node = this->Source->_Find_Node(var->Get_name());
				if (node != nullptr) {
					if ((!this->Source->bDestroy_Potentials_and_Variables) && (node->Get_var() != var)) {
						throw std::runtime_error("when using non cloning insertion, you must refer to exactly the same variables already in the model");
					}
				}
				return node;
			};
			Node*		Create(Categoric_var* var){
				if(this->Source->bDestroy_Potentials_and_Variables) this->Source->Nodes.emplace_with_builder(*this , var , false);
				else 												this->Source->Nodes.emplace_with_builder(*this ,var , true);
				return this->Source->_Find_Node(var->Get_name());
			};
			Node*		operator()(Categoric_var* v, const bool& f) const { return new Node(v,f); };
		};
		Node_hndl Hndl(this);

		P* inserted = nullptr;
		auto var_involved = to_insert->Get_Distribution().Get_Variables();
		if (var_involved.size() == 1) {
	//unary potential insertion
			Node* node = Hndl.Find(var_involved.front());
			if (node == nullptr)  node = Hndl.Create(var_involved.front());

			inserted = to_insert;
			if(this->bDestroy_Potentials_and_Variables) inserted = new P(*to_insert, std::vector<Categoric_var*>{node->Get_var()});

			node->Permanent_Unary.push_back(inserted);
		}

		else if (var_involved.size() == 2) {

	//binary potential insertion
			Node* peer_A = Hndl.Find(var_involved.front());
			Node* peer_B = Hndl.Find(var_involved.back());

			if ((peer_A != nullptr) && (peer_B != nullptr)) {
	// check whether this binary potential was already present
				auto it_B = this->Binary_potentials.get_map()->find(std::make_pair(&peer_A->Get_var()->Get_name() , &peer_B->Get_var()->Get_name()));
				if(it_B != this->Binary_potentials.get_map()->end()) throw std::runtime_error("found clone of an already inserted binary potential");
			}
			else{
				if (peer_A == nullptr)  peer_A = Hndl.Create(var_involved.front());
				if (peer_B == nullptr)  peer_B = Hndl.Create(var_involved.back());
			}

			inserted = to_insert;
			if(this->bDestroy_Potentials_and_Variables) inserted = new P(*to_insert, std::vector<Categoric_var*>{peer_A->Get_var(), peer_B->Get_var()});
			this->Binary_potentials.emplace(inserted);

	//create connection
			Node::Neighbour_connection::init_connection( peer_A, peer_B, *inserted);
		}
		
		else throw std::runtime_error("Only binary or unary potential can be inserted");

		delete this->Propagation_Last;
		this->Propagation_Last = nullptr;

		this->Potential_observers.emplace_back( inserted->Get_as_Subject());
		return inserted;
		
	};



	class Node::Node_factory::Energy_Evaluator {
		friend class Node::Node_factory;
	public:
		template<typename Array>
		float		 			  Eval(const Array& combination) const{

			size_t f, F = this->Finders.size();
			float E = 1.f;
			for(f=0; f<F; ++f){
				auto val = (*this->Finders[f])(combination);
				if(val == nullptr) return 0.f;
				else E *= val->Get_val();
			}
			return E;

		};

		template<typename Array>
		std::vector<float>		  Eval_normalized(const std::vector<Array>& combinations) const{

			if(combinations.empty()) throw std::runtime_error("empty combinations vector");

			struct max_extractor{ 
				float operator()(const Discrete_Distribution& distr) const {  
					auto it  = distr.get_iter();					
					if(!it.is_not_at_end()) return 1.f;

					float max = it->Get_val(), att;
					++it;
					while(it.is_not_at_end()){
						att = it->Get_val();
						if(att > max) max =  att;
						++it;
					}
					return max;
				}; 
			} extractor;

			std::vector<float> vals;
			vals.reserve(combinations.size());

			size_t f, F = this->Finders.size();
			std::vector<float> E_max;
			E_max.reserve(this->Potentials.size());
			for(auto it = this->Potentials.begin(); it!=this->Potentials.end(); ++it) E_max.push_back(1.f / extractor((*it)->Get_Distribution()));
			for(size_t c =0; c<combinations.size(); ++c){
				vals.push_back(1.f);
				for(f=0; f<F; ++f){
					auto val = (*this->Finders[f])(combinations[c]);
					if(val == nullptr) {
						vals.back() = 0.f;
						break;
					}
					else vals.back() *= val->Get_val() * E_max[f];
				}
			}
			return vals;

		};

		/*
		* \brief Returns an estimation of the likelihood of the model
		* \details considering a particular training set as reference:
		* P(model | train_set). This method is called by an I_Trainer during the gradient descend performed when training the model
		* @param[in] training_set the training set to consider, i.e. the series of samples, wrapped in a combinations object
		* @param[out] result estimation of the likelihood
		*/
		template<typename Array>
			float Get_TrainingSet_Likelihood(const std::vector<Array>& combinations) const{

			std::vector<float> L = this->Eval_normalized(combinations);
			float result = 0.f;
			float coeff = 1.f / (float)combinations.size();
			auto it_end = L.end();
			for (auto it = L.begin(); it != it_end; ++it)
				result += coeff * *it;
			return result;

		};

		~Energy_Evaluator() { for(auto it=this->Finders.begin(); it!=this->Finders.end(); ++it) delete *it; };
	private:
		Energy_Evaluator(const Node_factory& factory, const std::vector<Categoric_var*>& vars_order);
	// data
		std::vector<const I_Potential*>								 Potentials;
		std::vector<Discrete_Distribution::const_Full_Match_finder*> Finders;
	};
	Node::Node_factory::Energy_Evaluator Node::Node_factory::Get_Energy_Evaluator(const std::vector<Categoric_var*>& vars_order) const { return Energy_Evaluator(*this, vars_order); };



	class Node::Node_factory::Structure_importer{
	public:
		Structure_importer(XML_reader& reader, const std::string& prefix_config_xml_file);
		~Structure_importer() { this->Shapes.clear(); this->Exp_Shapes.clear(); this->Vars.clear(); };

		inline const structure& 									Get_structure() const { return this->Parsed_structure; };
		inline const std::vector<std::pair<std::string, size_t>>&   Get_Observations() const { return this->Observations; };
	private:									
		Categoric_var*					__Find_Var(const std::string& name);
		void						    __Import_Shape(const std::string& prefix, XML_reader::Tag_readable& tag);
	// data
		std::list<Categoric_var>						Vars;
		std::list<Potential_Shape>						Shapes;
		std::list<Potential_Exp_Shape> 					Exp_Shapes;

		std::vector<std::pair<std::string, size_t>>		Observations;
		structure										Parsed_structure;
	};

}

#endif
