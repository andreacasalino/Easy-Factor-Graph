#include "NeighbourConnection.h"
#include <distribution/PartialMatchFinder.h>
#include <algorithm>
#include <float.h>
using namespace std;

namespace EFG::node {

	void Node::NeighbourConnection::initConnection(Node* peer_A, Node* peer_B, const pot::IPotential& pot_shared) {

		NeighbourConnection* A_B = new NeighbourConnection();
		NeighbourConnection* B_A = new NeighbourConnection();

		A_B->Neighbour = peer_B;
		B_A->Neighbour = peer_A;

		A_B->wasNeighbourhoodUpdated = false;
		B_A->wasNeighbourhoodUpdated = false;

		A_B->Message2This = nullptr;
		B_A->Message2This = nullptr;

		A_B->Linked = B_A;
		B_A->Linked = A_B;

		A_B->SharedPotential = &pot_shared;
		B_A->SharedPotential = &pot_shared;

		peer_A->ActiveConnections.push_back(A_B);
		peer_B->ActiveConnections.push_back(B_A);

	}

	void Node::NeighbourConnection::Disable() {

		Node* this_node = this->Linked->Neighbour;
		this_node->ActiveConnections.remove(this);
		this_node->DisabledConnections.push_back(this);

		this->Neighbour->ActiveConnections.remove(this->Linked);
		this->Neighbour->DisabledConnections.push_back(this->Linked);

		this->Linked->ResetOutgoingMessage();
		this->ResetOutgoingMessage();

	}

	void Node::NeighbourConnection::UpdateNeighbourhood() {

		if (this->wasNeighbourhoodUpdated) return;
		this->Neighbourhood = this->Linked->Neighbour->ActiveConnections;
		this->Neighbourhood.remove(this);
		this->wasNeighbourhoodUpdated = true;

	}

	bool Node::NeighbourConnection::isOutgoingRecomputationPossible() {

		this->UpdateNeighbourhood();
		for (auto it = this->Neighbourhood.begin(); it != this->Neighbourhood.end(); ++it) {
			if ((*it)->Message2This == nullptr) return false;
		}
		return true;

	}

	float Node::NeighbourConnection::RecomputeOutgoing(const bool& Sum_or_MAP) {

		this->UpdateNeighbourhood();

		Node* this_node = this->Linked->Neighbour;
		vector<const pot::IPotential*> una_pot;
		una_pot.reserve(this_node->TemporaryUnary.size() + this_node->PermanentUnary.size() + this->Neighbourhood.size());
		std::for_each(this_node->TemporaryUnary.begin(), this_node->TemporaryUnary.end(), [&una_pot](pot::Factor& f) { una_pot.push_back(&f); });
		std::for_each(this_node->PermanentUnary.begin(), this_node->PermanentUnary.end(), [&una_pot](const pot::IPotential* p) { una_pot.push_back(p); });
		std::for_each(this->Neighbourhood.begin(), this->Neighbourhood.end(), [&una_pot](NeighbourConnection* c) { una_pot.push_back(c->Message2This); });

		if (this->Linked->Message2This == nullptr) {
			if (una_pot.empty()) this->Linked->Message2This = new Message(*this->SharedPotential, this_node->GetVar(), Sum_or_MAP);
			else 		 	    this->Linked->Message2This = new Message(*this->SharedPotential, una_pot, Sum_or_MAP);
			return FLT_MAX;
		}
		else {
			if (una_pot.empty()) return static_cast<Message*>(this->Linked->Message2This)->Update(*this->SharedPotential, this_node->GetVar(), Sum_or_MAP);
			else 		 	    return static_cast<Message*>(this->Linked->Message2This)->Update(*this->SharedPotential, una_pot, Sum_or_MAP);
		}

	}

	void Node::NeighbourConnection::SetIncoming2Ones() {

		delete this->Message2This;
		this->Message2This = new Message(this->Linked->Neighbour->GetVar());

	}

	Node::NeighbourConnection::Message::Message(CategoricVariable* var_involved) : pot::IPotentialDecorator() {
		auto ones_shape = new pot::Factor({ var_involved });
		ones_shape->SetOnes();
		this->reset(ones_shape);
	}

	float Node::NeighbourConnection::Message::Update(const pot::IPotential& binary_to_merge, const std::vector<const pot::IPotential*>& potential_to_merge, const bool& Sum_or_MAP) {

		//if (potential_to_merge.empty())  throw 0; //  Message::Update: wrong method for managing an empty set of unary to merge

		vector<const pot::IPotential*> to_merge;
		to_merge.reserve(1 + potential_to_merge.size());
		to_merge = potential_to_merge;
		to_merge.push_back(&binary_to_merge);
		pot::Factor M(to_merge, true, true);

		//if(M.Get_Distribution().Get_Variables().size() != 2) throw 1;

		return this->Update(M, potential_to_merge.front()->GetDistribution().GetVariables().front(), Sum_or_MAP);

	}

	float Node::NeighbourConnection::Message::Update(const pot::IPotential& binary_to_merge, CategoricVariable* var_to_marginalize, const bool& Sum_or_MAP) {

		const vector<CategoricVariable*>& vars = binary_to_merge.GetDistribution().GetVariables();
		//if(vars.size() != 2) throw 0;

		CategoricVariable* var_remaining = vars.front();
		if (var_remaining == var_to_marginalize) var_remaining = vars.back();

		pot::Factor* mex = new pot::Factor(vector<CategoricVariable*>{var_remaining});

		distr::DiscreteDistribution::constPartialMatchFinder finder(binary_to_merge.GetDistribution(), { var_remaining });

		size_t D = var_remaining->size();

		std::list<const distr::DiscreteDistribution::Value*> finder_match;
		float val;
		if (Sum_or_MAP) {
			for (size_t d = 0; d < D; ++d) {
				finder_match = finder(&d);
				val = 0.f;
				std::for_each(finder_match.begin(), finder_match.end(), [&val](const distr::DiscreteDistribution::Value* v) { val += v->GetVal(); });
				mex->AddValue(&d, val);
			}
		}
		else {
			float temp;
			for (size_t d = 0; d < D; ++d) {
				finder_match = finder(&d);
				val = 0.f;
				std::for_each(finder_match.begin(), finder_match.end(), [&val, &temp](const distr::DiscreteDistribution::Value* v) {
					temp = v->GetVal();
					if (temp > val) val = temp;
				});
				mex->AddValue(&d, val);
			}
		}
		mex->Normalize();

		float diff = FLT_MAX;
		if (this->GetWrapped() != nullptr) {
			const distr::DiscreteDistribution& D1 = mex->GetDistribution();
			const distr::DiscreteDistribution& D2 = this->GetWrapped()->GetDistribution();

			if (D1.size() == D2.size()) {
				diff = 0.f;
				auto it1 = D1.getIter();
				auto it2 = D2.getIter();
				while (it1.isNotAtEnd()) {
					diff += abs(it1->GetVal() - it2->GetVal());
					++it1;
					++it2;
				}
			}
		}
		this->reset(mex);
		return diff;

	}

}
