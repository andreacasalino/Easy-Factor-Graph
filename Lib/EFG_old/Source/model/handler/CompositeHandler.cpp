#include <model/Learnable.h>
#include <algorithm>

namespace EFG::model {

	GraphLearnable::CompositeHandler::~CompositeHandler() {

		std::for_each(this->Components.begin(), this->Components.end(), [](AtomicHandler* a) { delete a; });

	}

	GraphLearnable::CompositeHandler::CompositeHandler(AtomicHandler* initial_A, AtomicHandler* initial_B) {

		this->Components.push_back(initial_A);
		this->Append(initial_B);

	}

	void GraphLearnable::CompositeHandler::SetWeight(const float& w_new) {

		auto it_end = this->Components.end();
		std::for_each(this->Components.begin(), this->Components.end(), [&w_new](AtomicHandler* a) { a->SetWeight(w_new); });
		
	}

	void GraphLearnable::CompositeHandler::RecomputeAlfaPart(const distr::Combinations& train_set) {

		this->alfaPart = 0.f;
		std::for_each(this->Components.begin(), this->Components.end(), [this, &train_set](AtomicHandler* a) {
			a->RecomputeAlfaPart(train_set);
			this->alfaPart += a->GetAlfaPart();
		});

	}

	float GraphLearnable::CompositeHandler::GetBetaPart() {

		float beta = 0.f;
		std::for_each(this->Components.begin(), this->Components.end(), [&beta](AtomicHandler* a) {
			beta += a->GetBetaPart();
		});
		return beta;

	}

	void GraphLearnable::CompositeHandler::Append(AtomicHandler* to_add) {

		this->Components.push_back(to_add);
		to_add->SetWeight(this->Components.front()->GetWeight());

	};

}
