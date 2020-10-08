/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#pragma once
#ifndef __EFG_POTENTIAL_H__
#define __EFG_POTENTIAL_H__

#include <distribution/Distribution.h>

namespace EFG::pot {

#define RAND_MAX_FLOAT float(RAND_MAX)

	/*!
	 * \brief Abstract interface for potentials handled by graphs
	 */
	class IPotential {
	public:
		IPotential(const IPotential& ) = delete;
		void operator=(const IPotential& ) = delete;

		virtual ~IPotential() {};

		virtual sbj::Subject* 							GetAsSubject() = 0;

		virtual const distr::DiscreteDistribution&		GetDistribution() const = 0;

		virtual void SubstituteVariables(const std::vector<CategoricVariable*>& new_var) = 0;		

		/** \brief Obtain the marginal probabilities of the variables in the domain of this potential,
		* when considering this potential only.
		Use I_potential::Get_domain to retrieve the combinations in the domain for which the marginals were computed 
		(values not contained in the returned domain, has marginal prob. = 0).
		*
		* @param[in] return the marginals
		*/
		std::vector<float> GetMarginals() const;
	protected:
		IPotential() = default;
	};



	template<typename Distr>
	class IFactor : public IPotential {
	public:
		inline sbj::Subject*							GetAsSubject() override { return &this->subject; };
		inline const distr::DiscreteDistribution&		GetDistribution() const override { return this->distribution; };

		/** \brief Use this method for replacing the set of variables in the domain of this potential.
		* Variables in new_var must be equal in number to the original set of variables and
		* must have the same sizes.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape.
		*
		* @param[in] new_var variables to consider for the substitution
		*/
		inline void SubstituteVariables(const std::vector<CategoricVariable*>& new_var) override { if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed"); this->distribution.replace(new_var);};

		/** \brief All values in the image of the domain are randomly set.
		\details The modification is allowed only in case this potential is not inserted in a graph or a Potential_Exp_Shape
		*/
		void SetRandom() {
			if (this->subject.isObserved()) throw std::runtime_error("cannot modify object while being observed");
			//srand((unsigned int)time(NULL));

			this->distribution.clear();

			JointDomainIterator::forEach(this->distribution.GetVariables(), 
				[this](const std::vector<size_t>& comb) { this->distribution.add(comb, (float)rand() / RAND_MAX_FLOAT); });
		};

		void OperateValRaw(const std::function<float(const float&)>& operation) {
			auto it = this->distribution.getIter();
			float old_val;
			itr::forEach<distr::DiscreteDistribution::Iterator>(it, [&old_val, &operation](distr::DiscreteDistribution::Iterator& itt) {
				old_val = itt->GetValRaw();
				itt->SetValRaw(operation(old_val));
				if (itt->GetVal() < 0.f) {
					itt->SetValRaw(old_val);
					throw std::runtime_error("operation on domain led to negative value: refused");
				}
			});
		};

		void operator*=(const float& factor) { this->OperateValRaw([&factor](const float& f) { return f * factor; }); };

		void operator+=(const float& addendum) { this->OperateValRaw([&addendum](const float& f) { return f + addendum; }); };

		inline void operator-=(const float& subtrct) { (*this) += -subtrct; };

	protected:
		IFactor(const std::vector<CategoricVariable*>& vars) : distribution(vars) {};

		IFactor(IFactor&& o) : distribution(std::move(o.distribution)) {
			if (o.subject.isObserved()) {
				static_cast<distr::DiscreteDistribution&>(this->distribution) = std::move(static_cast<distr::DiscreteDistribution&>(o.distribution));
				throw std::runtime_error("potential to move should be not observed by anyone at the time the move is invoked");
			}
		};

		Distr								distribution;
		sbj::MultiObservable				subject;
	};



	class IPotentialDecorator : public IPotential {
	public:
		~IPotentialDecorator();

		inline sbj::Subject* 						GetAsSubject() override { return this->wrapped->GetAsSubject(); };

		inline const distr::DiscreteDistribution&	GetDistribution() const override { return this->wrapped->GetDistribution(); };

		inline void SubstituteVariables(const std::vector<CategoricVariable*>& new_var) override { this->wrapped->SubstituteVariables(new_var); };
	protected:
		IPotentialDecorator(IPotential* to_wrap);
		IPotentialDecorator() = default;

		inline IPotential*	GetWrapped() { return this->wrapped.get(); };
		inline void reset(IPotential* to_wrap = nullptr);
	private:
		std::unique_ptr<sbj::Subject::Observer>	wrappedObsv;
		std::unique_ptr<IPotential>			    wrapped;
	};

}

#endif
