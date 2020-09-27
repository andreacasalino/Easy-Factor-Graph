/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_GRAPHICAL_MODEL_H__
#define __EFG_GRAPHICAL_MODEL_H__

#include <Node.h>
#include <Belief_propagation.h>

namespace EFG {

	/*!
	 * \brief Interface for managing generic graphs.
	 * \details Both Exponential and normal shapes can be included
	 * into the model. Learning is not possible: all belief propagation 
	 * operations are performed assuming the mdoel as is.
	 * Every Potential_Shape or Potential_Exp_Shape is copied and that copy is 
	 * inserted into the model.
	 */
	class Graph : public Node::Node_factory {
	public:
		/** \brief empty constructor
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called, a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		*/
		Graph(const bool& use_cloning_Insert = true, const I_belief_propagation_strategy& propagator = Basic_strategy()) : Node_factory(use_cloning_Insert,propagator) {};
		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{Graph_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "", const I_belief_propagation_strategy& propagator = Basic_strategy());

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Graph to copy
		*/
		Graph(const Node_factory& o) : Node_factory(true , _Get_porpagator(o)) { this->_Insert(o.Get_structure()); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one 
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input 
		* a potential which involves that variable).
		*/
		inline void Insert(Potential_Shape& pot) { this->_Insert(&pot); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		*/
		inline void Insert(Potential_Exp_Shape& pot) { this->_Insert(&pot); };
		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed.
		*/
		inline void	Insert(const structure& strct) { this->_Insert(strct); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		inline void	Set_Evidences(const std::vector<std::pair<std::string, size_t>>& new_observations) { this->Node_factory::_Set_Evidences(new_observations); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void	Set_Evidences(const std::vector<size_t>& new_observations) { this->Node_factory::_Set_Evidences(new_observations); };
	};


	/*!
	 * \brief Interface for managing learnable graphs, i.e. graphs for which it is possible perform learning.
	 */
	class Graph_Learnable : public Node::Node_factory {
	public:
		~Graph_Learnable();

		/*!
		 * \brief Returns the actual values of the tunable weights, those of the exponential shapes		 
		 whose weights can vary.
		 */
		std::vector<float> Get_tunable() const;

		/*!
		 * \brief Set the values of the tunable weights, those of the exponential shapes		 
		 whose weights can vary.
		 */
		void Set_tunable(const std::vector<float>& w_vector);

		/*!
		 * \brief Returns the gradient of the likelihood of the model w.r.t the tunable weights and a certain trainig set.
		 * @param[out] w_grad the computed gradient
		 * @param[in] training_set the training set to consider, i.e. the series of samples, wrapped in a combinations object
		 */
		std::vector<float> Get_tunable_grad(const Discrete_Domain& training_set, const bool& force_alpha_recomputation = false);

		/*!
		 * \brief Returns the model size, i.e. the number of tunable parameters of the model, i.e. the number of weigths that can vary.
		 */
		inline size_t Get_model_size() const { return this->Atomic_Learner.size() + this->Composite_Learner.size(); };

		/* \brief Enables the regularization term when computing the gradient.
		\details The regularion term penalizes big values for the weights, adding to the
		function to maximise when performing learning the following term: -w'*w.
		When enabled, that term is added when computing the gradient.
		 */
		inline void Activate_Regularization_term() { this->Use_regularization = true; };

		/* \brief Disables the regularization term when computing the gradient,
		\details see also Activate_Regularization_term.
		When disabled, the regularization term is not considered when computing the gradient.
		 */
		inline void Deactivate_Regularization_term() { this->Use_regularization = false; };

		const structure Get_structure() const override;
	protected:
		Graph_Learnable(const bool& use_cloning_Insert,const I_belief_propagation_strategy& propagator) : Node::Node_factory(use_cloning_Insert, propagator), Last_training_set_used(nullptr) , Use_regularization(false) {};

		class I_Learning_handler;
		class atomic_Learning_handler;
		class Unary_handler;
		class Binary_handler;
		class composite_Learning_handler;

		void 				 						_Insert(const structure& strct) override;
		Potential_Exp_Shape*		 			_Insert(Potential_Exp_Shape* pot, const bool& weight_tunability);

		template<typename Handler , typename ... Args>
		inline void								_Add(Args ... args);

		void										_Share(const std::vector<Categoric_var*>& set1, const std::vector<Categoric_var*>& set2); //variable sets of the potetentials that must share the weight

		template<typename Handler, typename ... Args>
		void										_Replace(const std::vector<Categoric_var*>& vars, Args ... args);

		virtual std::vector<float>   						_Get_beta_part(const Discrete_Domain& training_set) = 0;

		inline const std::list<I_Learning_handler*>* _Get_learner_list() const { return &this->Learner_list; };

		std::list<atomic_Learning_handler*>				_Get_all_handlers() const; //both the ones isolated and the ones inside the composites
	private:
		std::list<atomic_Learning_handler*>::iterator																					__Find_in_Atomics(const std::vector<Categoric_var*>& vars);
		std::pair<std::list<composite_Learning_handler>::iterator, std::list<atomic_Learning_handler*>::iterator>	__Find_in_Composites(const std::vector<Categoric_var*>& vars);
		bool __Compare(const std::vector<Categoric_var*>& set1, const std::vector<Categoric_var*>& set2);
	// data
		std::list<atomic_Learning_handler*>								Atomic_Learner; 
		std::list<composite_Learning_handler>							Composite_Learner;

		std::list<I_Learning_handler*>								   Learner_list; //the list of atomic and composite elements (atomic inside composite does not apper, since are contained in composite)

		bool																		Use_regularization;
		const Discrete_Domain*								 				Last_training_set_used;
	};

	class Graph_Learnable::I_Learning_handler {
	public:
		virtual ~I_Learning_handler() {};

		virtual const float&					Get_weight() = 0;
		virtual void							Set_weight(const float& w_new) = 0;

		virtual void							Recompute_grad_alfa_part(const Discrete_Domain& train_set) = 0;
		virtual const float&					Get_grad_alfa_part() = 0;
		virtual float							Get_grad_beta_part() = 0; //according to last performed belief propagation
	};

	class Graph_Learnable::atomic_Learning_handler : public I_Learning_handler, public I_Potential_Decorator, public Potential_Exp_Shape::weigth_modifier {
	public:
		inline const float&					Get_weight() override { return this->Get_wrapped_exp()->Get_w(); };
		inline void								Set_weight(const float& w_new) override { this->Potential_Exp_Shape::weigth_modifier::set_w(w_new); };

		void										Recompute_grad_alfa_part(const Discrete_Domain& train_set) override;
		const float&    						Get_grad_alfa_part() override { return this->alfa_part; };

		Potential_Exp_Shape* 				Get_wrapped_exp() { return static_cast<Potential_Exp_Shape*>(this->Get_wrapped()); };

		~atomic_Learning_handler()			{ this->Replace_wrapped(); };
	protected:
		atomic_Learning_handler(Potential_Exp_Shape* pot_to_handle);
	// cache
		float										  alfa_part;
	};

	class Graph_Learnable::Unary_handler : public atomic_Learning_handler {
	public:
		Unary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle) : atomic_Learning_handler(pot_to_handle), pNode(model->_Find_Node(pot_to_handle->Get_Distribution().Get_Variables().front()->Get_name())) {};
	private:
		float										Get_grad_beta_part() override;
	// data
		Node* pNode;
	};

	class Graph_Learnable::Binary_handler : public atomic_Learning_handler {
	public:
		Binary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle);
	private:
		float										Get_grad_beta_part() override;
	// data
		Node* pNode1;
		Node* pNode2;
	};

	class Graph_Learnable::composite_Learning_handler : public  I_Learning_handler {
	public:
		~composite_Learning_handler();
		composite_Learning_handler(atomic_Learning_handler* initial_A, atomic_Learning_handler* initial_B);

		inline const float&    				Get_weight() override { return this->Components.front()->Get_weight(); };
		void										Set_weight(const float& w_new) override;

		void										Recompute_grad_alfa_part(const Discrete_Domain& train_set) override;
		inline const float&    				Get_grad_alfa_part() override { return this->alfa_part; };
		float										Get_grad_beta_part() override;

		void 																Append(atomic_Learning_handler* to_add);
		inline std::list<atomic_Learning_handler*>*		   Get_Components() { return &this->Components; };
		inline const std::list<atomic_Learning_handler*>*  Get_Components() const { return &this->Components; };
		inline size_t													size() const { return this->Components.size(); };
	private:
	// data
		std::list<atomic_Learning_handler*>  Components;		
	// cache
		float										  alfa_part;
	};

	template<typename Handler , typename ... Args>
	void	Graph_Learnable::_Add(Args ... args) { //the arguments needed for building the atomic handler
		this->Atomic_Learner.push_back(new Handler(args...));
		this->Learner_list.push_back(this->Atomic_Learner.back());
	};

	template<typename Handler, typename ... Args>
	void	Graph_Learnable::_Replace(const std::vector<Categoric_var*>& vars, Args ... args) {

		auto a = this->__Find_in_Atomics(vars);
		if (a != this->Atomic_Learner.end()) {
			Potential_Exp_Shape* pot = (*a)->Get_wrapped_exp();
			this->Learner_list.remove(*a);
			delete *a;
			*a = new Handler(pot, args...);
			this->Learner_list.push_back(*a);
			return;
		}

		auto c = this->__Find_in_Composites(vars);
		if (c.first != this->Composite_Learner.end()) {
			Potential_Exp_Shape* pot = (*c.second)->Get_wrapped_exp();
			delete *c.second;
			*c.second = new Handler(pot, args...);
			return;
		}

		throw std::runtime_error("inexistent potential");

	};




	/*!
	 * \brief This class describes a generic Random Field, not having a particular set of variables observed.
	 */
	class Random_Field : public Graph_Learnable {
	public:
		/** \brief empty constructor
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called, a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		*/
		Random_Field(const bool& use_cloning_Insert = true, const I_belief_propagation_strategy& propagator = Basic_strategy()) : Graph_Learnable(use_cloning_Insert, propagator) {};

		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{RF_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "", const I_belief_propagation_strategy& propagator = Basic_strategy());

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Random_Field to copy
		*/
		Random_Field(const Node_factory& o) : Graph_Learnable(true, _Get_porpagator(o)) { this->_Insert(o.Get_structure()); };

		/*!
		 * \brief Similar to Graph::Insert(Potential_Shape* pot)
		 */
		inline void Insert(Potential_Shape& pot) { this->Node_factory::_Insert(&pot); };
		/*!
		 * \brief Similar to Graph::Insert(Potential_Exp_Shape* pot).  
		* @param[in] is_weight_tunable When true, you are specifying that this potential has a weight learnable, otherwise the value 
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		* of the weight is assumed constant.
		 */
		inline void Insert(Potential_Exp_Shape& pot, const bool& is_weight_tunable = true) { this->Graph_Learnable::_Insert(&pot, is_weight_tunable); };
		/*!
		 * \brief Insert a tunable exponential shape, whose weight is shared with another already inserted tunable shape.
		 * \details This allows having many exponential tunable potetials which share the value of the weight: this is automatically account for when
		 * performing learning.
		* @param[in] vars_of_pot_whose_weight_is_to_share the list of varaibles involved in a potential already inserted whose weight is to share with the 
		* potential passed. They must be references to the variables actually wrapped into the model.
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		* of the weight is assumed constant.
		 */
		void Insert(Potential_Exp_Shape& pot, const std::vector<std::string>& vars_of_pot_whose_weight_is_to_share);

		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		inline void					  Set_Evidences(const std::vector<std::pair<std::string, size_t>>& new_observations) { this->Node_factory::_Set_Evidences(new_observations); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void					  Set_Evidences(const std::vector<size_t>& new_observations) { this->Node_factory::_Set_Evidences(new_observations); };
	
		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed. 
		*/
		inline void				Insert(const structure& strct) { this->_Insert(strct); };
	private:
		std::vector<float>   		 _Get_beta_part(const Discrete_Domain& training_set) override;
	};


	/*!
	 * \brief This class describes Conditional Random fields
	 * \details Set_Observation_Set_var is depracated: the observed set of variables cannot be changed after construction.
	 */
	class Conditional_Random_Field : public Graph_Learnable {
	public:
		/** \brief The model is built considering the information contained in an xml configuration file.  @latexonly\label{CRF_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "", const I_belief_propagation_strategy& propagator = Basic_strategy());

		/** \brief Copy constructor.
		\details The same evidence set is assumed, considering as initial observations a set of null values.
		* @param[in] o the Conditional_Random_Field to copy
		*/
		Conditional_Random_Field(const Node_factory& o);

		/** \brief This constructor initializes the graph with the specified potentials passed as input
		, setting the variables passed as the one observed
		*
		* @param[in] shapes the initial set of potentials to insert (can be empty)
		* @param[in] constant_exp the initial set of constant (non tunable weights) exponential potentials to insert (can be empty)
		* @param[in] learnable_exp the clusters of initial tunable potentials to insert (can be empty). Each cluster is a collection
		of tunable exponential potentials sharing the same weight. A single cluster can have a single element, representing a normal tunable
		exponential potential to insert in the graph, which does not share the weight with anyone.
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called (this includes the passed potentials),
		* a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		* @param[in] observed_var the name of the variables to assume as evidences. It cannot be empty
		*/
		Conditional_Random_Field(const structure& strct, const std::vector<std::string>& observed_var, const bool& use_cloning_Insert = true, const I_belief_propagation_strategy& propagator = Basic_strategy());
		
		~Conditional_Random_Field() { delete[] this->pos_observations_in_training_set; };

		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		inline void					  Set_Evidences(const std::vector<size_t>& new_observed_vals) { this->Node_factory::_Set_Evidences(new_observed_vals); };
	private:
		class Binary_handler_with_Observation : public atomic_Learning_handler {
		public:
			Binary_handler_with_Observation(Potential_Exp_Shape* pot , Node* Hidden_var, size_t* observed_val);
		private:
			float			Get_grad_beta_part() override;
		// data
			Node*			pNode_hidden;
			size_t*			ref_to_val_observed;
		// cache
			size_t			pos_observed;
			size_t			pos_hidden;
		};


		void 				 				 _Import(const structure& strct, const std::vector<std::string>& evidences);

		std::vector<float>   		 _Get_beta_part(const Discrete_Domain& training_set) override;

	// cache for setting observations from training set
		const std::vector<Categoric_var*>*				vars_order_training_set;
		size_t											pos_observations_size;
		size_t*											pos_observations_in_training_set;
	};

}

#endif