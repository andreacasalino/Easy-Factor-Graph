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
		ExpFactor(const ExpFactor&) = delete;

		template<typename ... Args>
		static ExpFactor makeFactor(Args ... args) { return std::move(ExpFactor(args...)); };

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

		/** \brief This is an interface for any object that should be allowed to modify the weight of an ExpFactor
		\details You have to keep in mind that only one WeightModifier instance at a time has the ownership of an ExpFactor and can modify the weight.
		*/
		class WeightModifier : public sbj::Subject::Observer {
		protected:
			WeightModifier(ExpFactor& involved_pot);

			inline void SetWeight(const float& w) { this->pot.distribution.setWeight(w); };
		private:
			ExpFactor& pot;
		};

		/** \brief Handler calling the move constructor to create a new ExpFactor.
		\details The move construction is a dangerous operation leave without values the distribution of the passed object.
		For this reason it's placed in this handler
		*/
		class Mover {
		protected:
			inline std::unique_ptr<ExpFactor> createMoving(ExpFactor&& o) {
				if (o.distribution.size() == 0) throw std::runtime_error("cannot move an already moved ExpFactor");
				return std::unique_ptr<ExpFactor>(new ExpFactor(std::move(o)));
			};
		};

		inline const float& GetWeight() const { return this->distribution.getWeight(); };
	protected:
		ExpFactor(ExpFactor&& o) : IFactor<distr::ExpDistribution>(std::move(o)) {};
	private:
		sbj::SingleObservable					subjectWeightHndlr;
	};

}

#endif