/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <potential/Potential.h>
#include <distribution/FullMatchFinder.h>
using namespace std;

namespace EFG::pot {

	std::vector<float> IPotential::GetMarginals() const {

		size_t Joint_Size = CategoricVariable::GetJointDomainSize(this->GetDistribution().GetVariables());

		float Z = 0.f;
		{
			auto it = this->GetDistribution().getIter();
			itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&Z](distr::DiscreteDistribution::constIterator& itt) { Z += itt->GetVal();  });
		}
		if(Z < 1e-8) return vector<float>(Joint_Size , 1.f / (float)Joint_Size);

		Z = 1.f / Z;

		std::vector<float> prob_distr;
		prob_distr.reserve(Joint_Size);
		if(this->GetDistribution().size() == Joint_Size){
			auto it = this->GetDistribution().getIter();
			itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&Z, &prob_distr](distr::DiscreteDistribution::constIterator& itt) { prob_distr.push_back(itt->GetVal() * Z); });
		}
		else{
			distr::DiscreteDistribution::constFullMatchFinder finder(this->GetDistribution());
			JointDomainIterator::forEach(this->GetDistribution().GetVariables(), [&finder, &Z, &prob_distr](const std::vector<size_t>& comb) {
				auto val = finder(comb);
				if (nullptr == val) prob_distr.push_back(0.f);
				else prob_distr.push_back(val->GetVal() * Z);
			});
		}
		return prob_distr;

	}

	IPotentialDecorator::IPotentialDecorator(IPotential* to_wrap)
		: wrappedObsv(make_unique<sbj::Subject::Observer>(*to_wrap->GetAsSubject())) {
		this->wrapped.reset(to_wrap);
	}

	IPotentialDecorator::~IPotentialDecorator() { this->reset(); }

	void IPotentialDecorator::reset(IPotential* to_wrap) {
		this->wrappedObsv.reset();
		if (nullptr != this->wrapped) {
			if (this->wrapped->GetAsSubject()->isObserved()) this->wrapped.release();
		}

		this->wrapped.reset(to_wrap);
		if (nullptr != this->wrapped) 
			this->wrappedObsv = move( make_unique<sbj::Subject::Observer>(*this->wrapped->GetAsSubject()) );
	};

}
