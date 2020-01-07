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
		Graph(const std::list<Potential_Shape*>& potentials, const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert = true);

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one 
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input 
		* a potential which involves that variable).
		*/
		void Insert(Potential_Shape* pot) { this->__Insert(pot); };

		/** \brief The model is built considering the information contained in an xml configuration file
		* @param[in] the potential to insert. It can be a unary or a binary potential. In case it is binary, at least one
		* of the variable involved must be already inserted to the model before (with a previous Insert having as input
		* a potential which involves that variable).
		*/
		void Insert(Potential_Exp_Shape* pot) { this->__Insert(pot, false); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vars, new_observed_vals); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };


		/** \brief Absorbs all the variables and the potentials contained in the model passed as input
		* \details Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed. 
		*/
		void			  Absorb(Node_factory* to_absorb) { this->__Absorb(to_absorb); };
	};


	class I_Learning_handler {
	public:
		virtual ~I_Learning_handler() {};

		virtual void			Get_weight(float* w) = 0;
		virtual void			Set_weight(const float& w_new) = 0;

		virtual void			Get_grad_alfa_part(float* alfa, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var) = 0;
		virtual void			Get_grad_beta_part(float* beta) = 0; //according to last performed belief propagation
	};

	class atomic_Learning_handler : public I_Learning_handler, public I_Potential_Decorator<Potential_Exp_Shape>, public Potential_Exp_Shape::Getter_weight_and_shape {
	public:
		virtual void			Get_weight(float* w) { *w = *this->pWeight; };
		virtual void			Set_weight(const float& w_new) { *this->pWeight = w_new; };

		virtual void			Get_grad_alfa_part(float* alfa, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);

		bool			is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);
		Potential_Exp_Shape* Get_wrapped() { return this->pwrapped; };
	protected:
		atomic_Learning_handler(Potential_Exp_Shape* pot_to_handle);
		atomic_Learning_handler(atomic_Learning_handler* other) : atomic_Learning_handler(other->pwrapped) {  };

	// data
		float*															pWeight;
		// cache
		std::list<I_Distribution_value*>			  Extended_shape_domain; //for computing beta part of gradient
	};

	class composite_Learning_handler : public  I_Learning_handler {
	public:
		~composite_Learning_handler();
		composite_Learning_handler(atomic_Learning_handler* initial_A, atomic_Learning_handler* initial_B);

		virtual void			Get_weight(float* w) { this->Components.front()->Get_weight(w); };
		virtual void			Set_weight(const float& w_new);

		virtual void			Get_grad_alfa_part(float* alfa, const std::list<size_t*>& comb_in_train_set, const std::list<Categoric_var*>& comb_var);
		virtual void			Get_grad_beta_part(float* beta);

		void Append(atomic_Learning_handler* to_add);
		bool			is_here_Pot_to_share(const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);
		std::list<atomic_Learning_handler*>*	Get_Components() { return &this->Components; };
	private:
	// data
		std::list<atomic_Learning_handler*>  Components;
	};



	/*!
	 * \brief Interface for managing learnable graphs, i.e. graphs for which it is possible perform learning.
	 */
	class Graph_Learnable : public Node::Node_factory {
	public:
		~Graph_Learnable();

		struct Weights_Manager {
			friend class I_Trainer;
		public:
			/*!
			 * \brief Returns the values of the tunable weights, those that can vary when learning the model
			 */
			static void Get_tunable_w(std::list<float>* w, Graph_Learnable* model);
		private:
			static void Get_tunable_w_grad(std::list<float>* grad_w, Graph_Learnable* model, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order);
			static void Set_tunable_w(const std::list<float>& w, Graph_Learnable* model);
		};

		/*!
		 * \brief Returns the model size, i.e. the number of tunable parameters of the model, i.e. the number of weigths that can vary with learning.
		 */
		size_t Get_model_size() { return this->Model_handlers.size(); };

		/*
		* \brief Returns an estimation of the likelihood of the model
		* \details (weights describing the wrapped Potential_Exp_Shape), considering a particular training set as reference:
		* P(model | train_set). This method is called by an I_Trainer during the gradient descend performed when training the model
		* @param[in] comb_train_set samples contained in a training set, obtained calling Training_set::subset::Handler::Get_list
		* @param[in] comb_var_order list of variables describing how the values in comb_train_set are ordered (they must refere to the variables wrapped by this model)
		* @param[out] result logarithmic estimation of the likelihood
		*/
		void Get_Likelihood_estimation(float* result, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order);
	protected:
		virtual Potential_Exp_Shape* __Insert(Potential_Exp_Shape* pot, const bool& weight_tunability);

		Graph_Learnable(const bool& use_cloning_Insert) : Node::Node_factory(use_cloning_Insert), pLast_train_set(NULL) {};
		Graph_Learnable(const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert , const std::list<bool>& tunable_mask,
			const std::list<Potential_Shape*>& shapes);

	// data
		std::list<I_Learning_handler*>		Model_handlers; //contains both the atomic and the composite ones as generic handlers

		void Get_complete_atomic_handler_list(std::list<atomic_Learning_handler**>* atomic_list);  
		void Remove(atomic_Learning_handler* to_remove);
		void Share_weight(I_Learning_handler* pot_involved, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);
		void Import_XML_sharing_weight_info(XML_reader& reader);

		virtual void			  __Absorb(Node_factory* to_absorb);
	private:

		virtual void			  __Get_exponential_shapes(std::list<std::list<Potential_Exp_Shape*>>* learnable_exp, std::list<Potential_Exp_Shape*>* constant_exp);

		//as baseline behaviour the alfa part of gradient is recomputed in case train set has changed, and is added to the result
		virtual void Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order);

    // data for managing weight sharing for exponential potentials
		template<typename T>
		struct Learner_info {
			T*			Ref_to_learner;
			size_t		pos_in_Model_handlers;
		};
		std::list<Learner_info<atomic_Learning_handler>>				 Atomic_Learner; 
		std::list<Learner_info<composite_Learning_handler>>		 Composite_Learner;

	// cache for gradient computation
		struct proxy_gradient_info { 
			proxy_gradient_info(const std::list<size_t*>& l) : Last_set(l) {};

			const std::list<size_t*>& Last_set; 
		};

		std::list<float>					Alfa_part_gradient;
		proxy_gradient_info*		pLast_train_set;
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
		*
		* @param[in] potentials_exp the initial set of exponential potentials to insert (can be empty)
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called (this includes the passed potentials),
		* a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		* @param[in] tunable_mask when passed as non default value, it is must have the same size of potentials. Every value in this list is true if the corresponfing potential in the
		* potentials list is tunable, i.e. has a weight whose value can vary with learning
		* @param[in] shapes A list of additional non learnable potentials to insert in the model
		*/
		Random_Field(const std::list<Potential_Exp_Shape*>& potentials_exp, const bool& use_cloning_Insert = true, const std::list<bool>& tunable_mask = {},
			const std::list<Potential_Shape*>& shapes = {});

		/*!
		 * \brief Similar to Graph::Insert(Potential_Shape* pot)
		 */
		void Insert(Potential_Shape* pot) { this->Node_factory::__Insert(pot); };
		/*!
		 * \brief Similar to Graph::Insert(Potential_Exp_Shape* pot).  
		* @param[in] is_weight_tunable When true, you are specifying that this potential has a weight learnable, otherwise the value 
		* of the weight is assumed constant.
		 */
		void Insert(Potential_Exp_Shape* pot, const bool& is_weight_tunable = true) { this->Graph_Learnable::__Insert(pot, is_weight_tunable); };
		/*!
		 * \brief Insert a tunable exponential shape, whose weight is shared with another already inserted tunable shape.
		 * \details This allows having many exponential tunable potetials which share the value of the weight: this is automatically account for when
		 * performing learning.
		* @param[in] vars_of_pot_whose_weight_is_to_share the list of varaibles involved in a potential already inserted whose weight is to share with the 
		* potential passed. They must be references to the variables actually wrapped into the model.
		 */
		void Insert(Potential_Exp_Shape* pot, const std::list<Categoric_var*>& vars_of_pot_whose_weight_is_to_share);

		/*!
		 * \brief see Node::Node_factory::Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<Categoric_var*>& new_observed_vars, const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vars, new_observed_vals); };
		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };
	
		/** \brief Absorbs all the variables and the potentials contained in the model passed as input
		* \details Consistency checks are performed: it is possible that some inconsistent components in the model passed
		* will be not absorbed. 
		*/
		void			  Absorb(Node_factory* to_absorb) { this->__Absorb(to_absorb); };
	private:
		void Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order);
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

		/** \brief This constructor initializes the graph with the specified potentials passed as input, setting the variables passed as the one observed
		*
		*
		* @param[in] potentials the initial set of exponential potentials to insert (can be empty)
		* @param[in] observed_var the set of variables to assume as observations
		* @param[in] use_cloning_Insert when is true, every time an Insert of a novel potential is called (this includes the passed potentials),
		* a copy of that potential is actually inserted.
		* Otherwise, the passed potential is inserted as is: this can be dangerous, cause that potential cna be externally modified, but the construction of
		* a novel graph is faster.
		* @param[in] tunable_mask when passed as non default value, it is must have the same size of potentials. Every value in this list is true if the corresponfing potential in the 
		* potentials list is tunable, i.e. has a weight whose value can vary with learning
		* @param[in] shapes A list of additional non learnable potentials to insert in the model
		*/
		Conditional_Random_Field(const std::list<Potential_Exp_Shape*>& potentials, const std::list<Categoric_var*>& observed_var, const bool& use_cloning_Insert = true, const std::list<bool>& tunable_mask = {},
			const std::list<Potential_Shape*>& shapes = {});
		

		/*!
		 * \brief see Node::Node_factory::Set_Evidences( const std::list<size_t>& new_observed_vals)
		 */
		void					  Set_Evidences(const std::list<size_t>& new_observed_vals) { this->Node_factory::Set_Evidences(new_observed_vals); };
	private:
		void __remove_redudant();

		void Get_w_grad(std::list<float>* grad_w, const std::list<size_t*>& comb_train_set, const std::list<Categoric_var*>& comb_var_order);
	};

}

#endif