#include <model/Learnable.h>
#include <distribution/FullMatchFinder.h>

namespace EFG::model {

	GraphLearnable::AtomicHandler::AtomicHandler(pot::ExpFactor* pot_to_handle) :
		pot::IPotentialDecorator(pot_to_handle), WeightModifier(*pot_to_handle) { }

	void GraphLearnable::AtomicHandler::RecomputeAlfaPart(const distr::Combinations& train_set) {

		distr::DiscreteDistribution::constFullMatchFinder finder(this->GetWrapped()->GetDistribution(), train_set.GetVariables());
		auto it = train_set.getIter();
		this->alfaPart = 0.f;
		float coeff = 1.f / (float)train_set.size();
		itr::forEach<distr::Combinations::constIterator>(it, [this, &finder, &coeff](distr::Combinations::constIterator& itt) {
			auto val = finder(*itt);
			//impossible to have val == nullptr since it's an exp shape
			this->alfaPart += coeff * val->GetValRaw();
		});

	}

}
