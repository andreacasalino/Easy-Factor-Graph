//Author: Andrea Casalino
//mail: andrecasa91@gmail.com

#pragma once
#ifndef __CRF_POTENTIAL_H__
#define __CRF_POTENTIAL_H__

#include <list>
#include <fstream>


namespace Segugio {
	
	/*!
	 * \brief Describes a categoric variable
	 * \details , having a finite set as domain,
	 * assumed by default as {0,1,2,3,...,size}
	 */
	class Categoric_var {
		/** \brief domain is assumed to be {0,1,2,3,...,size}
		 */
	public:
		/**
		* @param[in] size domain size of this variable
		* @param[in] name name to attach to this variable. It cannot be an empty string ""
		*/
		Categoric_var(const size_t& size, const std::string& name);
		Categoric_var(const Categoric_var& to_copy) { abort(); };
		virtual ~Categoric_var() {};

		const size_t& size() const { return this->Size; };
		const std::string& Get_name() { return this->Name; };
	protected:
	// data
		size_t		Size; /** domain size */
		std::string Name; /** name associated to the variable */
	};


	class Categoric_domain : public Categoric_var {
	public:
	//domain is assumed to be {Domain[0],Domain[1],...,Domain[size]}
		~Categoric_domain() { free(Domain); };

		const float& operator[](const size_t& pos);
	private:
		float*      Domain;
	};


	/*!
	 * \brief Abstract interface for potentials handled by graphs
	 */
	class I_Potential {
	public:
		I_Potential(const I_Potential& to_copy) { abort(); };
		virtual ~I_Potential() {};
		
		/*!
		 * \brief Abstract interface for describing a value in the domain of a potential
		 */
		struct I_Distribution_value {
			virtual ~I_Distribution_value() {};

			virtual void	Set_val(const float& v) = 0;
			virtual void	Get_val(float* result) = 0;
			virtual size_t* Get_indeces() = 0;
		};

		/**  \brief when print_entire_domain is true, the entire domain is printed, even though the potential has a sparse distribution
		* @param[in] f out stream to target
		* @param[in] print_entire_domain
		*/
		void Print_distribution(std::ostream& f, const bool& print_entire_domain = false); 
		/** \brief return list of references to the variables representing the domain of this Potential
		*/
		const std::list<Categoric_var*>*			Get_involved_var_safe() const {  return this->Get_involved_var(); };
		/**
		* @param[out] result the list of values matching the combinations to find sent as input
		*
		* @param[in] comb_to_search domain list of combinations (i.e. values of the domain) whose values are to find
		* @param[in] comb_to_search_var_order order of variables used for assembling the combinations to find
		*/
		void			Find_Comb_in_distribution(std::list<float>* result,
			const std::list<size_t*>& comb_to_search, const std::list<Categoric_var*>& comb_to_search_var_order);
		/** \brief get entire domain of a group of variables: list of possible combinations
		*
		* @param[out] domain the entire set of possible combinations
		* @param[in] Vars_in_domain variables involved whose domain has to be compute
		*/
		static void Get_entire_domain(std::list<std::list<size_t>>* domain, const std::list<Categoric_var*>& Vars_in_domain);
		
		/** \brief Returns the maximum value in the distribution describing this potential
		*/
		float max();
	protected:
		I_Potential() {};

		//This version returns the value in the distribution of this potential matching with the comb_to_search. When there is no match for 
		//a certain comb, a NULL is put into result. 
		static void			Find_Comb_in_distribution(std::list<I_Distribution_value*>* result,
			const std::list<size_t*>& comb_to_search, const std::list<Categoric_var*>& comb_to_search_var_order, I_Potential* pot);

		//This version returns for a partial combination (a combination involving a portion of the variables involved in this distribution), all the
		//values partially matching in the distribution of this pot. In case none of the value in the distribution of this pot matches, at least a NULL id put
		//into result
		static void			Find_Comb_in_distribution(std::list<I_Distribution_value*>* result,
			                size_t* partial_comb_to_search, const std::list<Categoric_var*>& partial_comb_to_search_var_order, I_Potential* pot);

		struct Getter_4_Decorator{
		protected:
			static const std::list<Categoric_var*>*       Get_involved_var(I_Potential* pot) { return pot->Get_involved_var(); };
			static std::list<I_Distribution_value*>*	  Get_distr(I_Potential* pot) { return pot->Get_distr(); };
		};

		virtual const std::list<Categoric_var*>*	  Get_involved_var() const = 0;
		virtual std::list<I_Distribution_value*>*	Get_distr() = 0;

		//computes all the indices constituting the domain of a group of categorical variables
		static void Get_entire_domain(std::list<size_t*>* domain, const std::list<Categoric_var*>& Vars_in_domain);
	};


	/*!
	 * \brief Abstract decorator of a Potential, wrapping an Abstract potential
	 */
	template <typename Wrapped_Type>
	class I_Potential_Decorator : public I_Potential, public I_Potential::Getter_4_Decorator {
	public:
		~I_Potential_Decorator() { if (Destroy_wrapped) delete this->pwrapped; };
	protected:
		I_Potential_Decorator(Wrapped_Type* to_wrap) :pwrapped(to_wrap), Destroy_wrapped(true) {};

		virtual const std::list<Categoric_var*>*    Get_involved_var() const { return this->I_Potential::Getter_4_Decorator::Get_involved_var(pwrapped); };
		virtual std::list<I_Distribution_value*>*	Get_distr() { return this->I_Potential::Getter_4_Decorator::Get_distr(pwrapped); };
	protected:
	// data
		bool			Destroy_wrapped; /** when false, the wrapped abstract potential is wrapped also in another decorator, whihc is in charge of deleting the wrapped potential */
		Wrapped_Type*	pwrapped;		 /** the object wrapped by this decorator*/
	};


	/*!
	 * \brief It's the only possible concrete potential. It contains the domain and the image of the potential
	 */
	class Potential_Shape : public I_Potential {
	public:
		/** \brief When building a new shape potential, all values of the image are assumed as all zeros
		*
		* @param[in] var_involved variables involved in the domain of this variables
		*/
		Potential_Shape(const std::list<Categoric_var*>& var_involved);
		
		/**
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		*/
		Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read);

		/** \brief Returns simple correlating or anti_correlating shapes. 
		* \details A simple correlating shape is a distribution having a value of 1 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 0 for all other combinations.
		* A simple anti_correlating shape is a distribution having a value of 0 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 1 for all other combinations.
		*
		* @param[in] var_involved variables involved in the domain of this variables: they must have all the same size
		* @param[in] correlated_or_not when true produce a simple correlating shape, when false produce a anti_correlating function
		*/
		Potential_Shape(const std::list<Categoric_var*>& var_involved, const bool& correlated_or_not);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have 
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Potential_Shape(const Potential_Shape* to_copy, const std::list<Categoric_var*>& var_involved);

		Potential_Shape(const Potential_Shape& to_copy) { abort(); };
		~Potential_Shape();

		/** \brief For populating the image of the domain with the values reported in the textual file
		*
		* @param[in] file_to_read textual file to read containing the values for the image
		*/
		void Import(const std::string& file_to_read);
		/** \brief Add a new value in the image set
		*
		* @param[in] new_indices combination related to the new value to add for the image
		* @param[in] new_val new val to insert
		*/
		void Add_value(const std::list<size_t>& new_indeces, const float& new_val);
		/** \brief All values in the image of the domain are set to 1
		*/
		void Set_ones();		
		/** \brief All values in the image of the domain are randomly set
		*/
		void Set_random(const float zeroing_threashold = 1.f);
		/** \brief All values in the image of the domain are multipled by a scaling factor, in order to 
		* to have maximal value equal to 1. 
		* Exploited for computing messages
		*/
		void Normalize_distribution();
		/** \brief Use this method for replacing the set of variables this potential must refer.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		void Substitute_variables(const std::list<Categoric_var*>& new_var);
	protected:
		void Check_add_value(const std::list<size_t>& indices); //check the specified combination is not already present in the distribution

		virtual const std::list<Categoric_var*>*	Get_involved_var() const { return &this->Involved_var; };
		virtual std::list<I_Distribution_value*>*	Get_distr() { return &this->Distribution; };
	private:
	// data
		std::list<Categoric_var*>			Involved_var; /** list of the involved variables in the domain of this potential */
		std::list<I_Distribution_value*>	Distribution; /** Every element describes: a combination in the domain and its corresponding value in the image */
	};


	/*!
	 * \brief Represents an exponential potential, wrapping a normal shape one: every value of the domain are assumed as exp(mWeight * val_in_shape_wrapped)
	*/
	class Potential_Exp_Shape : public I_Potential_Decorator<Potential_Shape> {
	public:
		/** \brief When building a new exponential shape potential, all the values of the domain are computed 
		* according to the new shape passed as input
		*
		* @param[in] shape shape distribution to wrap
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(Potential_Shape* shape, const float& w = 1.f);
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the potential shape to wrap, which is instantiated in the constructor by 
		* considering the textual file provided, see also Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read)
		*
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w = 1.f);
		/** \details Use this constructor for cloning an exponential shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Potential_Exp_Shape(const Potential_Exp_Shape* to_copy, const std::list<Categoric_var*>& var_involved);

		~Potential_Exp_Shape();

		struct Getter_weight_and_shape {
		protected:
			static float*           Get_weight(Potential_Exp_Shape* pot) { return &pot->mWeight; };
			static Potential_Shape*  Get_shape(Potential_Exp_Shape* pot) { return pot->pwrapped; };
		};

		/** \brief Use this method for replacing the set of variables this potential must refer.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		void Substitute_variables(const std::list<Categoric_var*>& new_var) { this->pwrapped->Substitute_variables(new_var); };
	protected:
		virtual std::list<I_Distribution_value*>*	Get_distr() { return &this->Distribution; };
		void										Wrap(Potential_Shape* shape);
	// data
		float								    mWeight;		/** Weight assumed for modulating the exponential (see description of the class) */
		std::list<I_Distribution_value*>		Distribution;   /** Every element describes: a combination in the domain and its corresponding value in the image */
	};


	/*! 
	\brief This class is mainly adopted for computing operations on potentials
	*/
	class Potential : public I_Potential_Decorator<I_Potential> {
	public:		
		/**
		* @param[in] pot potential shape to wrap
		*/
		Potential(Potential_Shape* pot) : I_Potential_Decorator(pot) { };
		/**
		* @param[in] pot exponential potential shape to wrap
		*/
		Potential(Potential_Exp_Shape* pot) : I_Potential_Decorator(pot) { };
		/** \brief The potential to create is obtained by merging a set of potentials referring to the same variables
		* (i.e. values in the image are obtained as a product of the ones in the potential_to_merge set)
		*
		* @param[in] potential_to_merge list of potential to merge, i.e. compute their product
		* @param[in] use_sparse_format when false, the entire domain is allocated even if some values are equal to 0
		*/
		Potential(const std::list<Potential*>& potential_to_merge, const bool& use_sparse_format = true);
		/** \brief The potential to create is obtained by marginalizing the observed variable passed as input
		*
		* @param[in] pot_to_reduce the potential from which the variables observed are marginalized
		* @param[in] var_observed variables observed in pot_to_reduce
		* @param[in] val_observed values observed (same oreder of var_observed)
		*/
		Potential(const std::list<size_t>& val_observed, const std::list<Categoric_var*>& var_observed, Potential* pot_to_reduce);

		/** \brief Obtain the marginal probabilities of the variables in the domain of this potential, 
		* when considering this potential only
		*
		* @param[in] prob_distr marginals
		*/
		void Get_marginals(std::list<float>* prob_distr);
	protected:
		Potential() : I_Potential_Decorator(NULL) {};
	};


	/*! 
	\brief This class is adopted by belief propagation algorithms. It is the message incoming to a node of the graph. Every node of a graph refers to a single Categorical variable.
	* Internally it keeps track of the difference in time of the messages produced, in order to arrest loopy belief propagation. 
	*/
	class Message_Unary : public Potential { //adopted by belief propagation algorithms
	public:
		/** \brief Creates a Message with all 1 as values for the image
		*
		* @param[in] var_involved the only variable in the domain
		*/
		Message_Unary(Categoric_var* var_involved);
		/** \brief Firstly, all potential_to_merge are merged together using Potential::Potential(potential_to_merge, false) obtaining a merged potential.
		* Secondly, the product of binary_to_merge and the merged potential is obtained. 
		* Finally the message is obtained by marginalizing from the second product, the variable of potential_to_merge, adopting a sum or a MAP.
		* Exploited by message passing algorithms
		*
		* @param[in] binary_to_merge binaty potential to consider
		* @param[in] potential_to_merge list of potentials to merge. The must be unary potentials
		*/
		Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
		/** \brief Same as Message_Unary::Message_Unary(Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true),
		* but in the case potential_to_merge is empty
		*/
		Message_Unary(Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true); //use this when the unary set is empty

		/** \brief Adopted by loopy belief propagation
		*
		* @param[out] diff_to_previous The difference with respect to the previous message camptation
		*/
		void Update(float* diff_to_previous , Potential* binary_to_merge, const std::list<Potential*>& potential_to_merge, const bool& Sum_or_MAP = true);
		/** \brief Adopted by loopy belief propagation
		*
		* @param[out] diff_to_previous The difference with respect to the previous message camptation
		*/
		void Update(float* diff_to_previous, Potential* binary_to_merge, Categoric_var* var_to_marginalize, const bool& Sum_or_MAP = true);
	private:
		Potential_Shape* merge_binary_and_unary(Potential* binary_to_merge, Potential* unary, const bool& Sum_or_MAP);
	};

}

#endif