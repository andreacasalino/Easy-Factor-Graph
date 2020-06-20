/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __CRF_POTENTIAL_H__
#define __CRF_POTENTIAL_H__

#include "Observed_Subject.h"
#include <ostream>
#include <vector>


namespace EFG {

	/*!
	 * \brief Describes a categoric variable
	 * \details , having a finite set as domain,
	 * assumed by default as {0,1,2,3,...,size}.
	 A Categoric_var cannot be destroyed before destroying 
	 all the potentials referring to it.
	 */
	class Categoric_var : public Subject {
		/** \brief domain is assumed to be {0,1,2,3,...,size}
		 */
	public:
		/**
		* @param[in] size domain size of this variable
		* @param[in] name name to attach to this variable. It cannot be an empty string ""
		*/
		Categoric_var(const size_t& size, const std::string& name);

		const size_t& size() const { return this->Size; };
		const std::string& Get_name() const { return this->Name; };
	protected:
	// data
		size_t		Size; /** domain size */
		std::string Name; /** name associated to the variable */
	};
	void check_all_vars_are_different(const std::list<Categoric_var*>& variables); //throws an exception if some clones are present



	/*!
	 * \brief Abstract interface for potentials handled by graphs
	 */
	class I_Potential {
	public:
		virtual ~I_Potential() {};

		/**  \brief when print_entire_domain is true, the entire domain is printed, even though the potential has a sparse distribution
		* @param[in] f out stream to target
		* @param[in] print_entire_domain
		*/
		void Print_distribution(std::ostream& f, const bool& print_entire_domain = false) const;
	
		/** \brief Returns the maximum value in the distribution describing this potential
		*/
		float max_in_distribution() const;

		/**  \brief Returns the elements of the domain of this potential
		* @param[out] domain the returned domain, each elemnt is a vector of discrete values
		*/
		void Get_domain(std::vector<std::vector<size_t>>* domain) const;

		/**  \brief Returns the images of this potential.
		\details The corresponding domain can be extracted using I_Potential::Get_domain
		*/
		void Get_images(std::vector<float>* images) const;

		virtual Subject* Cast_to_Subject() = 0;

		/**  \brief This class describes a generic element in the domain of a potential, as well as the corresponding image
		*/
		class I_Distribution_value {
		public:
			virtual	~I_Distribution_value() {};

			/**  \brief Returns the image corresponding to the domain value described by this object
			*/
			virtual float		  Get_val() const = 0;

			/**  \brief Returns the domain value described by this object
			* @param[out] return the combination to describe is an array of discrete values: the address of the first
			element in this array is actually returned.
			*/
			virtual const size_t* Get_indeces() const = 0;
		};

		/**  \brief This class describes a collection of discrete values that can assume a specific set of categoric variables.
		\details The values of the combinations are ordered as the group of variables is ordered when building this object.
		*/
		class combinations {
		public:
			/**  \brief All the possible combinations of the group of passed variables is considered to build this object 
			* @param[in] variables the group of variables whose possible combinations are considered
			*/
			combinations(const std::list<Categoric_var*>& variables);

			/**  \brief The specified combinations, referring to the passed group of variables are considered to build this object
			* @param[in] variables the group of variables the passed combinations refer to
			* @param[in] Combination the series of combinations to consider. Each element is a collection of discrete value describing
			the value assumed by the passed variables.
			*/
			combinations(const std::list<std::list<size_t>>& Combinations, const std::list<Categoric_var*>& variables);

			/**  \brief Similar to combinations::combinations(const std::list<std::list<size_t>>& Combinations, const std::list<Categoric_var*>& variables), 
			passing only one single combination.
			*/
			combinations(const std::list<size_t>& Combination, const std::list<Categoric_var*>& variables) :
				combinations(std::list<std::list<size_t>>({ Combination }), variables) {};

			/**  \brief The values in the domain of the passed potential are considered, assuming the variables contained in the same potential.
			*/
			combinations(const I_Potential& pot);

			~combinations() { if ((this->Combinations != NULL)&& this->free_Combinations) free(this->Combinations); };

			/**  \brief This object is used to iterate the combinations contained in this object
			*/
			class iterator {
			public:
				/**  \brief After construction, the first element in to_iterate is pointed by the created iterator.
				*/
				iterator(const combinations& to_iterate);

				/**  \brief Get the combination toward which this iterator is actually pointing
				*/
				const size_t*	operator*() const;
				/**  \brief Pass to the next element
				*/
				iterator&		operator++();
				/**  \brief Check whether this object is incrementable.
				\details Returns true when the pointed element is not end (with the same meaning as in std::list:
				end is a non existing element that follows the last real element in the collection)
				*/
				bool			is_not_at_end() const;
			private:
				const combinations* Source;
				size_t						Source_cursor;
				size_t						Source_pos;
			};

			/**  \brief Returns the number of combinations contained in this object.
			*/
			const size_t& get_number_of_combinations() const { return this->Size; };

			/**  \brief Returns the values in pots, whose domain combination matches with the ones contained in this object.
			\details This method can be used when the number n2 of variables describing the domain of the passed potential is equal or
			lower to the size n1 of the group of variables describing this object, since for every value in pot a single possible match may exists.
			When checking whether two combinations hava a match, the order of the variables in this object as well as the one of the variables 
			in pot is taken into account.
			* @param[out] result each element is the match found for the corresponding combination, ordered as the they are contained in this object. 
			When a match was not found a NULL is put in the collection
			* @param[in] pot the potentials whose values must be addressed for computing the matches
			the value assumed by the passed variables.
			*/
			void		  Find_images_single_matches(std::vector<const I_Distribution_value*>* result, const I_Potential& pot) const;

			/**  \brief Returns the values in pots, whose domain combination matches with the ones contained in this object.
			\details This method can be used when the number n2 of variables describing the domain of the passed potential is grater
			than the size n1 of the group of variables describing this object, since for every value in pot multiple possible matches may exists.
			When checking whether two combinations hava a match, the order of the variables in this object as well as the one of the variables
			in pot is taken into account.
			* @param[out] result each element is the collection of matches found for the corresponding combination, ordered as the they are contained in this object.
			When no matches were found, an empty list is put in the returned collection.
			* @param[in] pot the potentials whose values must be addressed for computing the matches
			the value assumed by the passed variables.
			*/
			void		Find_images_multiple_matches(std::vector<std::list<const I_Distribution_value*>>* result, const I_Potential& pot) const;

			class _Training_set_t;
			combinations(combinations&& o);

			/**  \brief Returns the group of variables these combinations refer to
			*/
			const std::list<Categoric_var*>& Get_variables() const { return this->Variables; };

			/**  \brief Returns the size of the domain of the passed group of variables.
			\details For example when having vars = {V1, V2, V3} with size(V1) = 3, size(V1) = 4  and size(V3) = 2, 
			the returned number is 3X4X2=24.
			*/
			static size_t	    Get_joint_domain_size(const std::list<Categoric_var*>& vars);
		protected:
			combinations() : Combinations(NULL) {};
		// data
			bool							Entire_domain_was_allocated;
			std::list<Categoric_var*>		Variables;
			size_t							Size;
			size_t*							Combinations;
			bool							free_Combinations;
		};

	protected:

		class Distribution_value_concrete : public I_Distribution_value {
		public:
			Distribution_value_concrete(size_t* Comb, const float& v) : comb(Comb), val(v) {};
			~Distribution_value_concrete() { free(this->comb); };

			void				  Set_val(const float& new_v) { this->val = new_v; };
			virtual float		  Get_val() const { return this->val; };
			virtual const size_t* Get_indeces() const { return this->comb; };
		private:
		// data
			size_t* comb;
			float		val;
		};

		I_Potential(const I_Potential& to_copy);
		I_Potential() {};

		virtual       std::list<I_Distribution_value*>*				Get_distr() = 0;
		static        std::list<I_Distribution_value*>*				Get_distr_static(I_Potential* pot) { return pot->Get_distr(); };
		virtual const std::list<I_Distribution_value*>*				Get_distr() const = 0;
		static  const std::list<I_Distribution_value*>*				Get_distr_static(const I_Potential* pot) { return pot->Get_distr(); };
	public:
		virtual const std::list<Categoric_var*>*				Get_involved_var() const = 0;
	};


	/*!
	 * \brief Abstract decorator of a Potential, wrapping an Abstract potential
	 */
	template <typename Wrapped_Type>
	class I_Potential_Decorator : public I_Potential {
	public:
		~I_Potential_Decorator() { if (Destroy_wrapped) delete this->pwrapped; };

		virtual Subject* Cast_to_Subject() { return this->pwrapped->Cast_to_Subject(); };
		virtual const std::list<Categoric_var*>* Get_involved_var() const { return this->pwrapped->Get_involved_var(); };

		const Wrapped_Type* Get_wrapped() { return this->pwrapped; };
	protected:
		I_Potential_Decorator(Wrapped_Type* to_wrap) :pwrapped(to_wrap), Destroy_wrapped(true) {  };

		virtual std::list<I_Distribution_value*>* Get_distr() { return Get_distr_static(this->pwrapped); };
		virtual const std::list<I_Distribution_value*>* Get_distr() const { return Get_distr_static(this->pwrapped); };
	// data
		Wrapped_Type* pwrapped;		 /** the object wrapped by this decorator*/
		bool						Destroy_wrapped; /** when false, the wrapped abstract potential is wrapped also in another decorator, whihc is in charge of deleting the wrapped potential */		
	};


	/*!
	 * \brief It's the only possible concrete potential. It contains the domain and the image of the potential.
	 \details A Potential_Shape cannot be destroyed before destroying all the Node_factory and Potential_Exp_Shape containing it.
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

		/** \brief Returns simple correlating or anti_correlating shapes.  @latexonly\label{corr_anti_corr_Pot_constructor}@endlatexonly
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
		Potential_Shape(const Potential_Shape& to_copy, const std::list<Categoric_var*>& var_involved);

		/** \brief The passed potential is cloned as a simple shape,
		\details copying one by one  the combinations of its domain and its images. 
		The variables considered are the same.
		*
		* @param[in] to_copy the generic potential to clone as a simple shape 
		*/
		Potential_Shape(const I_Potential& to_copy);

		//Potential_Shape(const Potential_Shape& to_copy) { abort(); }; //unecessary since copy constructor of I_Potential was already inibited
		~Potential_Shape();

		/** \brief Add a new value in the image set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*
		* @param[in] new_indices combination related to the new value to add for the image
		* @param[in] new_val new val to insert
		*/
		void Add_value(const std::list<size_t>& new_indeces, const float& new_val);
		
		/** \brief All values in the image of the domain are set to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Set_ones();
		
		/** \brief All values in the image of the domain are randomly set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Set_random();
		
		/** \brief All values in the image of the domain are multipled by a scaling factor, in order to
		* to have maximal value equal to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Normalize_distribution();
		
		/** \brief Use this method for replacing the set of variables this potential must refer.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		void Substitute_variables(const std::list<Categoric_var*>& new_var);

		virtual Subject* Cast_to_Subject() { return &this->subject; };
		virtual const std::list<Categoric_var*>* Get_involved_var() const { return &this->Involved_var; };
	protected:
		void __Check_add_value(const std::list<size_t>& indices); //check the specified combination is not already present in the distribution
		void __Alloc_entire_domain(const float& val_to_use_4_entire_domain);
		void __Set_Distribution_val(I_Distribution_value* to_set, const float& new_val);

		virtual std::list<I_Distribution_value*>*		Get_distr() { return &this->Distribution; };
		virtual const std::list<I_Distribution_value*>* Get_distr() const { return &this->Distribution; };
	private:
		void								__Register_to_observers();
	// data
		std::list<Categoric_var*>			Involved_var; /** list of the involved variables in the domain of this potential */
		std::list<Subject::Observer>		var_observers;  
		std::list<I_Distribution_value*>	Distribution; /** Every element describes: a combination in the domain and its corresponding value in the image */
		Subject								subject;
	};


	/*!
	 * \brief Represents an exponential potential, wrapping a normal shape one: every value of the domain are assumed as exp(mWeight * val_in_shape_wrapped).
	 \details A Potential_Exp_Shape cannot be destroyed before destroying all the Node_factory containing it.
	*/
	class Potential_Exp_Shape : public I_Potential_Decorator<Potential_Shape> {
	public:
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the new shape passed as input
		*
		* @param[in] shape shape distribution to wrap
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(Potential_Shape& shape, const float& w = 1.f);
		
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the potential shape to wrap, which is instantiated in the constructor by
		* considering the textual file provided, see also Potential_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read)
		*
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(const std::list<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w = 1.f);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy exp_shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Potential_Exp_Shape(const Potential_Exp_Shape& to_copy, const std::list<Categoric_var*>& var_involved);

		~Potential_Exp_Shape();

		/** \brief Returns the weight assigned to this potential
		*/
		const float& get_weight() const { return this->mWeight; };

		/** \brief This class allows to remotely change the value of a Potential_Exp_Shape weight.
		\details You can derive an object from this one if you need. Anyway, you have to keep in mind
		that only one instance of a weigth_modifier at a time can handle a weight of a Potential_Exp_Shape.
		Moreover, tunable Potential_Exp_Shape passed to Random_Field or Conditional_Random_Field are automatically handled by some interal weigth_modifier
		for allowing the training of the model: until such potentials are contained in that models no other weigth_modifier are 
		allowed to remotely handle their weights.
		*/
		class weigth_modifier : public Subject::Observer {
		public:
			/** \brief The potential whose weight must be remotely handled must be passed
			*/
			weigth_modifier(Potential_Exp_Shape& involved_pot);
		protected:
		// data
			float*					pWeight;
		};

		/** \brief Use this method for replacing the set of variables this potential must refer.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		void Substitute_variables(const std::list<Categoric_var*>& new_var) { this->pwrapped->Substitute_variables(new_var); };

		/** \brief returns the wrapped Potential_Shape*/
		const Potential_Shape* Get_wrapped_Shape() const { return this->pwrapped; };

		virtual Subject* Cast_to_Subject() { return &this->subject; };
	protected:
		virtual std::list<I_Distribution_value*>*		Get_distr() { return &this->Distribution; };
		virtual const std::list<I_Distribution_value*>* Get_distr() const { return &this->Distribution; };
		void										Wrap(Potential_Shape* shape);

		class Distribution_exp_value;
	// data
		float														    mWeight;		/** Weight assumed for modulating the exponential (see description of the class) */
		std::list<I_Distribution_value*>							    Distribution;   /** Every element describes: a combination in the domain and its corresponding value in the image */
	private:
		Subject::Observer*												shape_observer;
		Subject															subject;
		Subject_singleObserver											subject_w_hndlr;
	};


	/*!
	\brief This class is mainly adopted for computing operations on potentials
	*/
	class Potential : public I_Potential_Decorator<I_Potential> {
	public:
		/**
		* @param[in] pot potential shape to wrap
		*/
		Potential(Potential_Shape& pot) : I_Potential_Decorator(&pot) { };
		/**
		* @param[in] pot exponential potential shape to wrap
		*/
		Potential(Potential_Exp_Shape& pot) : I_Potential_Decorator(&pot) { };

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
		Potential(const std::list<size_t>& val_observed, const std::list<Categoric_var*>& var_observed, Potential& pot_to_reduce);

		/** \brief Obtain the marginal probabilities of the variables in the domain of this potential,
		* when considering this potential only.
		Use I_potential::Get_domain to retrieve the combinations in the domain for which the marginals were computed 
		(values not contained in the returned domain, has marginal prob. = 0).
		*
		* @param[in] prob_distr marginals
		*/
		void Get_marginals(std::vector<float>* prob_distr) const;

		/** \brief Transfer the values in the distirbution of a potential into a simple shape.
		*\details The variables involved in the shape passed must be the same of this potential
		* when considering this potential only
		*
		* @param[in] shape the potential shape that will receive the cloned values
		*/
		void clone_distribution(Potential_Shape& shape) const;
	protected:
		Potential() : I_Potential_Decorator(NULL) {};
	};

}

#endif