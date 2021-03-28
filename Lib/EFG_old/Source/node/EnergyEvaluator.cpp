#include <node/EnergyEvaluator.h>
using namespace std;

namespace EFG::node {

	Node::NodeFactory::EnergyEvaluator::EnergyEvaluator(const Node::NodeFactory& factory, const std::vector<CategoricVariable*>& vars_order) 
		: sbj::Subject::Observer(factory.subject) {

		std::for_each(factory.__SimpleShapes.begin(), factory.__SimpleShapes.end(), [this](pot::Factor* p) { this->Potentials.push_back(p); });
		std::for_each(factory.__ExpShapes.begin(), factory.__ExpShapes.end(), [this](pot::ExpFactor* p) { this->Potentials.push_back(p); });

		std::for_each(this->Potentials.begin(), this->Potentials.end(), [this, &vars_order](const pot::IPotential* p) { this->Finders.emplace_back(p->GetDistribution(), vars_order); });

	}

}
