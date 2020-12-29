#include <node/Node.h>
#include "NeighbourConnection.h"
#include <algorithm>
using namespace std;

namespace EFG::node {

	Node::Node(CategoricVariable* var, const bool& dont_clone_var) {

		if (dont_clone_var) this->Variable = var;
		else 			    this->Variable = new CategoricVariable(var->size(), var->GetName());

	}

	Node::~Node() {

		std::for_each(this->ActiveConnections.begin(), this->ActiveConnections.end(), [](NeighbourConnection* c) { delete c; });
		std::for_each(this->DisabledConnections.begin(), this->DisabledConnections.end(), [](NeighbourConnection* c) { delete c; });
		
	}

	std::vector<const pot::IPotential*> Node::GetAllUnaries() const{

		vector<const pot::IPotential*> pot;
		pot.reserve(this->PermanentUnary.size() + this->TemporaryUnary.size() + this->ActiveConnections.size());

		std::for_each(this->PermanentUnary.begin(), this->PermanentUnary.end(), [&pot](const pot::IPotential* p) { pot.push_back(p); });
		std::for_each(this->TemporaryUnary.begin(), this->TemporaryUnary.end(), [&pot](const pot::Factor& f) { pot.push_back(&f); });
		std::for_each(this->ActiveConnections.begin(), this->ActiveConnections.end(), [&pot](const NeighbourConnection* n) { pot.push_back(n->GetIncomingMessage()); });
		return pot;

	}

}