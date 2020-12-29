#include "UnaryHandler.h"
#include <algorithm>

namespace EFG::model {

	float GraphLearnable::UnaryHandler::GetBetaPart() {

		auto marginals = pot::Factor(this->pNode->GetAllUnaries(), false).GetMarginals();
		float beta = 0.f;
		auto it = this->GetWrapped()->GetDistribution().getIter();
		itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&beta, &marginals](distr::DiscreteDistribution::constIterator& itt) {
			beta += itt->GetValRaw() * marginals[itt->GetIndeces()[0]];
		});
		return beta;

	}

}
