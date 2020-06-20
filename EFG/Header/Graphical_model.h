/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF_GRAPHICAL_MODEL_H__
#define __CRF_GRAPHICAL_MODEL_H__

#include "Node.h"

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
		Graph(const bool& use_cloning_Insert = true) : Node_factory(use_cloning_Insert) {};
		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{Graph_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Graph(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "");

		/** \brief This constructor initializes the graph with the specified potentials passed as input
		*
		*
		* @param[in] potentials the initial set of potentials to insert (can be empty)
		* @param[in] potentials_exp the initial set of exponential potentials to insert (can be empty)
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called (this includes the passed potentials), 
		* a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		*/
		Graph(const std::vector<Potential_Shape*>& potentials, const std::vector<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert = true);

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Graph to copy
		*/
		Graph(const Graph& o) : Node::Node_factory(o) { this->__copy(o); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one 
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input 
		* a potential which involves that variable).
		*/
		void Insert(Potential_Shape& pot) { this->__Insert(&pot); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		*/
		void Insert(Potential_Exp_Shape& pot) { this->__Insert(&pot); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vars, new_observed_vals); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };


		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed.
		*/
		void			  Absorb(const Node_factory& to_absorb) { this->__Absorb(to_absorb); };
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
		void Get_tunable(std::vector<float>* w_vector) const;

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
		void Get_tunable_grad(std::vector<float>* w_grad, const I_Potential::combinations& training_set, const bool& force_alpha_recomputation = false);

		/*!
		 * \brief Returns the model size, i.e. the number of tunable parameters of the model, i.e. the number of weigths that can vary.
		 */
		size_t Get_model_size() const { return this->Model_handlers.size(); };

		/*
		* \brief Returns an estimation of the likelihood of the model
		* \details considering a particular training set as reference:
		* P(model | train_set). This method is called by an I_Trainer during the gradient descend performed when training the model
		* @param[in] training_set the training set to consider, i.e. the series of samples, wrapped in a combinations object
		* @param[out] result estimation of the likelihood
		*/
		void Get_TrainingSet_Likelihood(float* result, const I_Potential::combinations& training_set) const;

		/* \brief Enables the regularization term when computing the gradient.
		\details The regularion term penalizes big values for the weights, adding to the
		function to maximise when performing learning the following term: -w'*w.
		When enabled, that term is added when computing the gradient.
		 */
		void Activate_Regularization_term() { this->Use_regularization = true; };

		/* \brief Disables the regularization term when computing the gradient,
		\details see also Activate_Regularization_term.
		When disabled, the regularization term is not considered when computing the gradient.
		 */
		void Deactivate_Regularization_term() { this->Use_regularization = false; };
	protected:
		Graph_Learnable(const bool& use_cloning_Insert) : Node::Node_factory(use_cloning_Insert), Last_training_set_used(NULL) , Use_regularization(false) {};
		Graph_Learnable(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const bool& use_cloning_Insert);
		Graph_Learnable(const Graph_Learnable& o) : Node_factory(o) , Last_training_set_used(NULL), Use_regularization(o.Use_regularization) {};

		class I_Learning_handler {
		public:
			virtual ~I_Learning_handler() {};

			virtual const float&	Get_weight() = 0;
			virtual void			Set_weight(const float& w_new) = 0;

			virtual void			Recompute_grad_alfa_part(const I_Potential::combinations& train_set) = 0;
			virtual const float&	Get_grad_alfa_part() = 0;
			virtual float			Get_grad_beta_part() = 0; //according to last performed belief propagation
		};

		class atomic_Learning_handler : public I_Learning_handler, public I_Potential_Decorator<Potential_Exp_Shape>, public Potential_Exp_Shape::weigth_modifier {
		public:
			virtual const float&	Get_weight() { return *this->pWeight; };
			virtual void			Set_weight(const float& w_new) { *this->pWeight = w_new; };

			virtual void			Recompute_grad_alfa_part(const I_Potential::combinations& train_set);
			virtual const float&    Get_grad_alfa_part() { return this->alfa_part; };

			bool			is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);
			Potential_Exp_Shape* Get_wrapped() { return this->pwrapped; };
		protected:
			atomic_Learning_handler(Potential_Exp_Shape* pot_to_handle);
			atomic_Learning_handler(atomic_Learning_handler** other);
		// cache
			float										  alfa_part;
		private:
			static Potential_Exp_Shape*				__extract_weight_hndlr(atomic_Learning_handler** other);
		};

		class Unary_handler : public atomic_Learning_handler {
		public:
			Unary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle) : atomic_Learning_handler(pot_to_handle), pNode(model->__Find_Node(pot_to_handle->Get_involved_var()->front())) {};
		private:
			virtual float			Get_grad_beta_part();
		// data
			Node* pNode;
		};

		class Binary_handler : public atomic_Learning_handler {
		public:
			Binary_handler(Graph_Learnable* model, Potential_Exp_Shape* pot_to_handle);
		private:
			virtual float			Get_grad_beta_part();
		// data
			Node* pNode1;
			Node* pNode2;
			std::vector<const I_Distribution_value*>		E_shape;
			std::vector<const I_Distribution_value*>		E_exp_shape;
		};

		class composite_Learning_handler : public  I_Learning_handler {
		public:
			~composite_Learning_handler();
			composite_Learning_handler(atomic_Learning_handler* initial_A, atomic_Learning_handler* initial_B);

			virtual const float&    Get_weight() { return this->Components.front()->Get_weight(); };
			virtual void			Set_weight(const float& w_new);

			virtual void			Recompute_grad_alfa_part(const I_Potential::combinations& train_set);
			virtual const float&    Get_grad_alfa_part() { return this->alfa_part; };
			virtual float			Get_grad_beta_part();

			void Append(atomic_Learning_handler* to_add);
			bool			is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);
			std::list<atomic_Learning_handler*>* Get_Components() { return &this->Components; };
		private:
		// data
			std::list<atomic_Learning_handler*>  Components;		
		// cache
			float										  alfa_part;
		};

		virtual void			     __Get_structure(std::vector<Potential_Shape*>* shapes, std::vector<std::list<Potential_Exp_Shape*>>* learnable_exp, std::vector<Potential_Exp_Shape*>* constant_exp) const;
		virtual void			     __Insert(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp);
		Potential_Exp_Shape*		 __Insert(Potential_Exp_Shape* pot, const bool& weight_tunability);

		void Remove(atomic_Learning_handler* to_remove);
		void Share_weight(I_Learning_handler* pot_involved, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);

		virtual void				 __Get_beta_part(std::vector<float>* betas, const I_Potential::combinations& training_set) = 0;

		void						 __Get_Atomic_Learner_complete_list(std::list<atomic_Learning_handler**>* atomic_list); //in the same list are put all the atomic single and the atomic contained in the composite
	private:
    // data for managing weight sharing for exponential potentials
		template<typename T>
		struct Learner_info {
			T*			Ref_to_learner;
			size_t		pos_in_Model_handlers;
		};

	// data
		std::list<Learner_info<atomic_Learning_handler>>				 Atomic_Learner; 
		std::list<Learner_info<composite_Learning_handler>>		         Composite_Learner;

	protected:
		std::list<I_Learning_handler*>									 Model_handlers; //contains both the atomic and the composite ones as generic handlers
	private:
		bool															 Use_regularization;
		const I_Potential::combinations*								 Last_training_set_used;
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
		Random_Field(const bool& use_cloning_Insert = true) : Graph_Learnable(use_cloning_Insert) {};

		/** \brief The model is built considering the information contained in an xml configuration file. @latexonly\label{RF_XML}@endlatexonly
		* \details @latexonly  See Section \ref{00_XML_format} of the documentation for the syntax to adopt. @endlatexonly
		* @param[in] configuration file
		* @param[in] prefix to use. The file prefix_config_xml_file/config_xml_file is searched.
		*/
		Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "");

		/** \brief This constructor initializes the graph with the specified potentials passed as input
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
		*/
		Random_Field(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const bool& use_cloning_Insert = true);

		/** \brief Copy constructor.
		\details All the potentials of the graph to copy are absorbed, by copying it.
		Also all the additional future potentials inserted will be copied.
		* @param[in] o the Random_Field to copy
		*/
		Random_Field(const Random_Field& o) : Graph_Learnable(o) { this->__copy(o); };

		/*!
		 * \brief Similar to Graph::Insert(Potential_Shape* pot)
		 */
		void Insert(Potential_Shape& pot) { this->Node_factory::__Insert(&pot); };
		/*!
		 * \brief Similar to Graph::Insert(Potential_Exp_Shape* pot).  
		* @param[in] is_weight_tunable When true, you are specifying that this potential has a weight learnable, otherwise the value 
		* @param[in] pot the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		* of the weight is assumed constant.
		 */
		void Insert(Potential_Exp_Shape& pot, const bool& is_weight_tunable = true) { this->Graph_Learnable::__Insert(&pot, is_weight_tunable); };
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
		void Insert(Potential_Exp_Shape& pot, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);

		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vars, new_observed_vals); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };
	
		/** \brief Absorbs all the variables and the potentials contained in the model passed as input.
		* \details The model receiving the potentials must be defined to do copying insertion.
		Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed. 
		*/
		void			  Absorb(const Node_factory& to_absorb) { this->__Absorb(to_absorb); };
	private:
		virtual void			  __Get_beta_part(std::vector<float>* betas, const I_Potential::combinations& training_set);
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
		Conditional_Random_Field(const std::string& config_xml_file, const std::string& prefix_config_xml_file = "");

		/** \brief Copy constructor.
		\details The same evidence set is assumed, considering as initial observations a set of null values.
		* @param[in] o the Conditional_Random_Field to copy
		*/
		Conditional_Random_Field(const Conditional_Random_Field& o);

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
		Conditional_Random_Field(const std::vector<Potential_Shape*>& shapes, const std::vector<std::list<Potential_Exp_Shape*>>& learnable_exp, const std::vector<Potential_Exp_Shape*>& constant_exp, const std::list<std::string>& observed_var, const bool& use_cloning_Insert = true);
		
		~Conditional_Random_Field() { if (this->pos_observations_in_training_set != NULL) free(this->pos_observations_in_training_set); };

		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };
	private:
		class Binary_handler_with_Observation : public atomic_Learning_handler {
		public:
			static void Create(Node* Hidden_var, size_t* observed_val, atomic_Learning_handler** handle_to_substitute);
		private:
			Binary_handler_with_Observation(Node* Hidden_var, size_t* observed_val, atomic_Learning_handler** handle_to_substitute);
			virtual float			Get_grad_beta_part();
		// data
			Node*			pNode_hidden;
			size_t*			ref_to_val_observed;
		// cache
			size_t			pos_observed;
			size_t			pos_hidden;
		};

		void					  __remove_redudant();

		virtual void			  __Get_beta_part(std::vector<float>* betas, const I_Potential::combinations& training_set);

	// cache for setting observations from training set
		const std::list<Categoric_var*>*				vars_order_training_set;
		size_t											pos_observations_size;
		size_t*											pos_observations_in_training_set;
	};

}

#endif