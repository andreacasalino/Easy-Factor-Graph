#include "BinaryHandler.h"
#include "../../node/NeighbourConnection.h"
using namespace std;

namespace EFG::model {

	GraphLearnable::BinaryHandler::BinaryHandler(GraphLearnable* model, pot::ExpFactor* pot_to_handle) :
		AtomicHandler(pot_to_handle),
		pNode1(model->_FindNode(pot_to_handle->GetDistribution().GetVariables().front()->GetName())),
		pNode2(model->_FindNode(pot_to_handle->GetDistribution().GetVariables().back()->GetName())) {
	};

	float GraphLearnable::BinaryHandler::BinaryHandler::GetBetaPart() {

		auto conn1 = this->pNode1->GetActiveConnections();
		auto temp1 = this->pNode1->GetTemporary();
		auto perm1 = this->pNode1->GetPermanent();

		auto conn2 = this->pNode2->GetActiveConnections();
		auto temp2 = this->pNode1->GetTemporary();
		auto perm2 = this->pNode1->GetPermanent();

		vector<const pot::IPotential*> pots;
		pots.reserve(temp1->size() + perm1->size() + conn1->size() + temp2->size() + perm2->size() + conn2->size() - 1);
		pots.push_back(this->GetWrapped());
		for (auto it = temp1->begin(); it != temp1->end(); ++it) pots.push_back(&(*it));
		for (auto it = perm1->begin(); it != perm1->end(); ++it) pots.push_back(*it);
		for (auto it = conn1->begin(); it != conn1->end(); ++it) {
			if ((*it)->GetNeighbour() != this->pNode2) pots.push_back((*it)->GetIncomingMessage());
		}
		for (auto it = temp2->begin(); it != temp2->end(); ++it) pots.push_back(&(*it));
		for (auto it = perm2->begin(); it != perm2->end(); ++it) pots.push_back(*it);
		for (auto it = conn2->begin(); it != conn2->end(); ++it) {
			if ((*it)->GetNeighbour() != this->pNode1) pots.push_back((*it)->GetIncomingMessage());
		}

		pot::Factor pot_tot(pots, false, true);

		auto marginals = pot_tot.GetMarginals();
		float beta = 0.f;
		size_t k = 0;
		auto it = this->GetWrapped()->GetDistribution().getIter();
		itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&marginals, &k, &beta](distr::DiscreteDistribution::constIterator& itt) {
			beta += itt->GetValRaw() * marginals[k];
			++k;
		});
		return beta;
	}

}
