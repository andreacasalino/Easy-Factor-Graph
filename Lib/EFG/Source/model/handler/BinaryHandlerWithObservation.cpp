#include "BinaryHandlerWithObservation.h"
using namespace std;

namespace EFG::model {

	ConditionalRandomField::BinaryHandlerWithObservation::BinaryHandlerWithObservation(pot::ExpFactor* pot, node::Node* Hidden_var, std::size_t* observed_val) :
		AtomicHandler(pot), Hidden(Hidden_var), Observation(observed_val) {

		const vector<CategoricVariable*>& vars = this->GetWrapped()->GetDistribution().GetVariables();
		this->posObserved = 0;
		this->posHidden = 1;
		if (vars.front() == Hidden_var->GetVar()) {
			this->posObserved = 1;
			this->posHidden = 0;
		}

	};

	float ConditionalRandomField::BinaryHandlerWithObservation::GetBetaPart() {

		vector<float> marginals = pot::Factor(this->Hidden->GetAllUnaries(), false).GetMarginals();

		float beta = 0.f;
		auto it = this->GetWrapped()->GetDistribution().getIter();
		itr::forEach<distr::DiscreteDistribution::constIterator>(it, [this, &beta, &marginals](distr::DiscreteDistribution::constIterator& itt) {
			const std::size_t* comb = itt->GetIndeces();
			if (comb[this->posObserved] == *this->Observation)
				beta += itt->GetValRaw() * marginals[comb[this->posHidden]];
		});
		return beta;

	}

}