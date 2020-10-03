/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_POTENTIAL_EXPFACTOR_H__
#define __EFG_POTENTIAL_EXPFACTOR_H__

#include <potential/Factor.h>
#include <distribution/DistributionExp.h>

namespace EFG::pot {

	/*!
	 * \brief Represents an exponential potential, wrapping a normal shape one: every value of the domain are assumed as exp(mWeight * val_in_shape_wrapped).
	 \details A Potential_Exp_Shape cannot be destroyed before destroying all the Node_factory containing it.
	*/
	class ExpFactor : public IFactor<distr::ExpDistribution> {
	public:
		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the new shape passed as input
		*
		* @param[in] shape shape distribution to wrap
		* @param[in] w weight of the exponential
		*/
		ExpFactor(const Factor& shape, const float& w = 1.f);

		/** \brief When building a new exponential shape potential, all the values of the domain are computed
		* according to the potential shape to wrap, which is instantiated in the constructor by
		* considering the textual file provided, see also Potential_Shape(const std::vector<Categoric_var*>& var_involved, const std::string& file_to_read)
		*
		* @param[in] var_involved variables involved in the domain of this variables
		* @param[in] file_to_read textual file to read containing the values for the image
		* @param[in] w weight of the exponential
		*/
		ExpFactor(const std::vector<CategoricVariable*>& var_involved, const std::string& file_to_read, const float& w = 1.f);

		/** \details Use this constructor for cloning a shape, but considering a different set of variables.
		* Variables in var_involved must be equal in number to those in the potential to clone and must have
		* the same sizes of the variables involved in the potential to clone.
		*
		* @param[in] to_copy exp_shape to clone
		* @param[in] var_involved new set of variables to consider when cloning
		*/
		ExpFactor(const ExpFactor& to_copy, const std::vector<CategoricVariable*>& var_involved);

		/** \brief This class allows to remotely change the value of a Potential_Exp_Shape weight.
		\details You can derive an object from this one if you need. Anyway, you have to keep in mind
		that only one instance of a weigth_modifier at a time can handle a weight of a Potential_Exp_Shape.
		Moreover, tunable Potential_Exp_Shape passed to Random_Field or Conditional_Random_Field are automatically handled by some interal weigth_modifier
		for allowing the training of the model: until such potentials are contained in that models no other weigth_modifier are
		allowed to remotely handle their weights.
		*/
		class Modifier : public sbj::Subject::Observer {
		public:
			/** \brief The potential whose weight must be remotely handled must be passed
			*/
			Modifier(ExpFactor& involved_pot);
		protected:
			inline void			SetWeight(const float& w) { this->pot.distribution.setWeight(w); };
		private:
			ExpFactor& pot;
		};

		inline const float& GetWeight() const { return this->distribution.getWeight(); };
	private:
		sbj::SingleObservable					subjectWeightHndlr;
	};

}

#endif