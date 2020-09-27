/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_POTENTIAL_H__
#define __EFG_POTENTIAL_H__

#include <Distribution.h>
#include <functional>

namespace EFG {

#define RAND_MAX_FLOAT float(RAND_MAX)

	/*!
	 * \brief Abstract interface for potentials handled by graphs
	 */
	class I_Potential {
	public:
		I_Potential(const I_Potential& ) = delete;
		void operator=(const I_Potential& ) = delete;

		virtual ~I_Potential() {};

		virtual Subject* 								Get_as_Subject() = 0;

		virtual const Discrete_Distribution&			Get_Distribution() const = 0;

		virtual void Substitute_variables(const std::vector<Categoric_var*>& new_var) = 0;		

		/** \brief Obtain the marginal probabilities of the variables in the domain of this potential,
		* when considering this potential only.
		Use I_potential::Get_domain to retrieve the combinations in the domain for which the marginals were computed 
		(values not contained in the returned domain, has marginal prob. = 0).
		*
		* @param[in] return the marginals
		*/
		std::vector<float> Get_marginals() const;


	protected:
		I_Potential() {};
	};



	template<typename Distr>
	class I_Potential_Concrete : public I_Potential {
	public:
		inline Subject* 						Get_as_Subject() override { return &this->subject; };
		inline const Discrete_Distribution&		Get_Distribution() const override { return this->distribution; };

		/** \brief Use this method for replacing the set of variables in the domain of this potential.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		inline void Substitute_variables(const std::vector<Categoric_var*>& new_var) override { if (this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed"); this->distribution.Substitute_variables(new_var);};

		/** \brief All values in the image of the domain are randomly set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Set_random() {
			if (this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed");
			//srand((unsigned int)time(NULL));

			this->distribution.clear();

			Domain_iterator domain(this->distribution.Get_Variables());
			while (domain.is_not_at_end()) {
				this->distribution.add(domain(), (float)rand() / RAND_MAX_FLOAT);
				++domain;
			}
		};

		void Operate_raw_values(const std::function<float(const float&)>& operation) {
			auto it = this->distribution.get_iter();
			float old_val;
			while (it.is_not_at_end()) {
				old_val = it->Get_val_raw();
				it->Set_val(operation(old_val));
				if (it->Get_val() < 0.f) {
					it->Set_val(old_val);
					throw std::runtime_error("operation on domain led to negative value: refused");
				}
				++it;
			}
		};

		void operator*=(const float& factor) {
			class Prod {
			public:
				Prod(const float& f) : F(f) {};
				float operator()(const float& val) const { return val * this->F; };
			private:
				const float& F;
			};
			this->Operate_raw_values(Prod(factor));
		};

		void operator+=(const float& addendum) {
			class Add {
			public:
				Add(const float& add) : A(add) {};
				float operator()(const float& val) const { return val + this->A; };
			private:
				const float& A;
			};
			this->Operate_raw_values(Add(addendum));
		};

		inline void operator-=(const float& subtrct) { (*this) += -subtrct; };

	protected:
		I_Potential_Concrete(const std::vector<Categoric_var*>& vars) : distribution(vars) {};

		Distr								distribution;
		Subject_multiObservers				subject;
	};


	/*!
	 * \brief It's the only possible concrete potential. It contains the domain and the image of the potential.
	 \details A Potential_Shape cannot be destroyed before destroying all the Node_factory and Potential_Exp_Shape containing it.
	 */
	class Potential_Shape : public I_Potential_Concrete<Discrete_Distribution> {
	public:
		/** \brief When building a new shape potential, all values of the image are assumed as all zeros
		*
		* @param[in] var_involved variables involved in the domain of this variables
		*/
		Potential_Shape(const std::vector<Categoric_var*>& var_involved);

		/**
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		*/
		Potential_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read);

		/** \brief Returns simple correlating or anti_correlating shapes.  @latexonly\label{corr_anti_corr_Pot_constructor}@endlatexonly
		* \details A simple correlating shape is a distribution having a value of 1 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 0 for all other combinations.
		* A simple anti_correlating shape is a distribution having a value of 0 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 1 for all other combinations.
		*
		* @param[in] var_involved variables involved in the domain of this variables: they must have all the same size
		* @param[in] correlated_or_not when true produce a simple correlating shape, when false produce a anti_correlating function
		*/
		Potential_Shape(const std::vector<Categoric_var*>& var_involved, const bool& correlated_or_not);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Potential_Shape(const Potential_Shape& to_copy, const std::vector<Categoric_var*>& var_involved);

		/** \brief The potential to create is obtained by merging a set of potentials referring to the same variables
		* (i.e. values in the image are obtained as a product of the ones in the potential_to_merge set)
		*
		* @param[in] potential_to_merge list of potential to merge, i.e. compute their product
		* @param[in] use_sparse_format when false, the entire domain is allocated even if some values are equal to 0
		*/
		Potential_Shape(const std::vector<const I_Potential*>& potential_to_merge, const bool& use_sparse_format, const bool& merge_domain = false);

		/** \brief The potential to create is obtained by marginalizing the observed variable passed as input.
		 * \details The returned values refer to the probabilities of the combinations in the domain of this potential, which can be 
		 * iterated by using I_Potential::Domain_iterator.
		*
		* @param[in] pot_to_reduce the potential from which the variables observed are marginalized
		* @param[in] var_observed variables observed in pot_to_reduce
		* @param[in] val_observed values observed (same oreder of var_observed)
		*/
		Potential_Shape(const std::vector<size_t>& val_observed, const std::vector<Categoric_var*>& var_observed, const I_Potential& pot_to_reduce);

		/** \brief Add a new value in the image set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*
		* @param[in] new_indices combination related to the new value to add for the image.
		* @param[in] new_val new val to insert
		*/
		template<typename Array>
		void Add_value(const Array& new_indeces, const float& new_val) {

			if (this->subject.is_observed()) throw std::runtime_error("cannot modify object while being observed");

			auto val = this->distribution.add(new_indeces, new_val);
			if (val->Get_val() < 0.f) {
				this->distribution.remove(new_indeces);
				throw std::runtime_error("invalid value requested to add: refused");
			}

		};

		/** \brief All values in the image of the domain are set to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Set_ones();
		
		/** \brief All values in the image of the domain are multipled by a scaling factor, in order to
		* to have maximal value equal to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Normalize_distribution();

		//sum of the images should return 1
		void Normalize_distribution2();
	};



	/*!
	 * \brief Represents an exponential potential, wrapping a normal shape one: every value of the domain are assumed as exp(mWeight * val_in_shape_wrapped).
	 \details A Potential_Exp_Shape cannot be destroyed before destroying all the Node_factory containing it.
	*/
	class Potential_Exp_Shape : public I_Potential_Concrete<Discrete_exp_Distribution> {
	public:
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the new shape passed as input
		*
		* @param[in] shape shape distribution to wrap
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(const Potential_Shape& shape, const float& w = 1.f);
		
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the potential shape to wrap, which is instantiated in the constructor by
		* considering the textual file provided, see also Potential_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read)
		*
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		* @param[in] w weight of the exponential
		*/
		Potential_Exp_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read, const float& w = 1.f);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy exp_shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Potential_Exp_Shape(const Potential_Exp_Shape& to_copy, const std::vector<Categoric_var*>& var_involved);

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
			inline void			set_w(const float& w) { this->pot.distribution.set_weight(w); };
		private:
			Potential_Exp_Shape&	pot;
		};

		inline const float& 		Get_w() const { return this->distribution.get_weight(); };
	private:
		Subject_singleObserver											subject_w_hndlr;
	};



	class I_Potential_Decorator : public I_Potential {
	public:

		~I_Potential_Decorator() { delete this->wrapped_obsv; delete this->wrapped; };

		inline Subject* 						Get_as_Subject() override { return this->wrapped->Get_as_Subject(); };

		inline const Discrete_Distribution&			Get_Distribution() const override { return this->wrapped->Get_Distribution(); };

		inline void Substitute_variables(const std::vector<Categoric_var*>& new_var) override { this->wrapped->Substitute_variables(new_var); };		
	protected:
		I_Potential_Decorator(I_Potential* to_wrap) : wrapped_obsv(new Subject::Observer(to_wrap->Get_as_Subject())), wrapped(to_wrap) {};

		I_Potential_Decorator() : wrapped_obsv(nullptr), wrapped(nullptr) {};

		inline I_Potential*	Get_wrapped() { return this->wrapped; };
		inline void Replace_wrapped(I_Potential* to_wrap = nullptr) { 
			delete this->wrapped_obsv;
			if (this->wrapped != nullptr) {
				if (!this->wrapped->Get_as_Subject()->is_observed()) delete this->wrapped;
			}
			this->wrapped = to_wrap;
			if(this->wrapped == nullptr) this->wrapped_obsv = nullptr;
			else 						 this->wrapped_obsv = new Subject::Observer(this->wrapped->Get_as_Subject());
		};
	private:
		Subject::Observer*	wrapped_obsv;
		I_Potential*		wrapped;
	};

}

#endif
