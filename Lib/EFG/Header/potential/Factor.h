/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_POTENTIAL_FACTOR_H__
#define __EFG_POTENTIAL_FACTOR_H__

#include <potential/Potential.h>

namespace EFG::pot {

	/*!
	 * \brief It's the only possible concrete potential. It contains the domain and the image of the potential.
	 \details A Potential_Shape cannot be destroyed before destroying all the Node_factory and Potential_Exp_Shape containing it.
	 */
	class Factor : public IFactor<distr::DiscreteDistribution> {
	public:
		/** \brief When building a new shape potential, all values of the image are assumed as all zeros
		*
		* @param[in] var_involved variables involved in the domain of this variables
		*/
		Factor(const std::vector<CategoricVariable*>& var_involved);

		/**
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		*/
		Factor(const std::vector<CategoricVariable*>& var_involved, const std::string& file_to_read);

		/** \brief Returns simple correlating or anti_correlating shapes.  @latexonly\label{corr_anti_corr_Pot_constructor}@endlatexonly
		* \details A simple correlating shape is a distribution having a value of 1 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 0 for all other combinations.
		* A simple anti_correlating shape is a distribution having a value of 0 for every combinations
		* {0,0,...,0}; {1,1,...,1} etc. and 1 for all other combinations.
		*
		* @param[in] var_involved variables involved in the domain of this variables: they must have all the same size
		* @param[in] correlated_or_not when true produce a simple correlating shape, when false produce a anti_correlating function
		*/
		Factor(const std::vector<CategoricVariable*>& var_involved, const bool& correlated_or_not);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		Factor(const Factor& to_copy, const std::vector<CategoricVariable*>& var_involved);

		/** \brief The potential to create is obtained by merging a set of potentials referring to the same variables
		* (i.e. values in the image are obtained as a product of the ones in the potential_to_merge set)
		*
		* @param[in] potential_to_merge list of potential to merge, i.e. compute their product
		* @param[in] use_sparse_format when false, the entire domain is allocated even if some values are equal to 0
		*/
		Factor(const std::vector<const IPotential*>& potential_to_merge, const bool& use_sparse_format, const bool& merge_domain = false);

		/** \brief The potential to create is obtained by marginalizing the observed variable passed as input.
		 * \details The returned values refer to the probabilities of the combinations in the domain of this potential, which can be
		 * iterated by using I_Potential::Domain_iterator.
		*
		* @param[in] pot_to_reduce the potential from which the variables observed are marginalized
		* @param[in] var_observed variables observed in pot_to_reduce
		* @param[in] val_observed values observed (same oreder of var_observed)
		*/
		Factor(const std::vector<size_t>& val_observed, const std::vector<CategoricVariable*>& var_observed, const IPotential& pot_to_reduce);

		/** \brief Add a new value in the image set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*
		* @param[in] new_indices combination related to the new value to add for the image.
		* @param[in] new_val new val to insert
		*/
		template<typename Array>
		void AddValue(const Array& new_indeces, const float& new_val) {

			if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed");

			auto val = this->distribution.add(new_indeces, new_val);
			if (val->GetVal() < 0.f) {
				this->distribution.remove(new_indeces);
				throw std::runtime_error("invalid value requested to add: refused");
			}

		};

		/** \brief All values in the image of the domain are set to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void SetOnes();

		/** \brief All values in the image of the domain are multipled by a scaling factor, in order to
		* to have maximal value equal to 1.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void Normalize();

		//sum of the images should return 1
		void Normalize2();
	};

}

#endif