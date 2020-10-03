#include <node/NodeFactory.h>
#include "NeighbourConnection.h"
#include <distribution/FullMatchFinder.h>
#include <set>
#include <algorithm>
using namespace std;

namespace EFG::node {

	std::unique_ptr<pot::Factor> Node::NodeFactory::GetJointMarginalDistribution(const std::vector<std::string>& subgroup) {

		set<Node*> Group;
	
		vector<CategoricVariable*> varG;
		varG.reserve(subgroup.size());
		for (size_t k = 0; k < subgroup.size(); ++k) {
			Node* temp = this->_FindNode(subgroup[k]);
			if (temp == nullptr) throw std::runtime_error("variable not found");
			Group.emplace(temp);
			varG.push_back(temp->GetVar());
		}
		CategoricVariable::AreAllVarsDifferent(varG);

		this->_BeliefPropagation(true);

		class Indicator : public pot::Factor {
		public:
			Indicator(CategoricVariable* var, const std::size_t& val_observed) : pot::Factor({ var }) {

				size_t K = var->size();
				for (size_t k = 0; k < K; ++k) {
					if (k == val_observed) this->distribution.add(&k, 1.f);
					//else this->distribution.add(&k, 0.f);
				}

			};
		};

		set<const pot::IPotential*>  bin_pot;

		list<const pot::IPotential*> shapes_to_merge;
		list<Indicator> ind_created;
		for (auto it = Group.begin(); it != Group.end(); ++it) {
			auto ito = this->Observations.get_map()->find(*it);
			if (ito == this->Observations.get_map()->end()) {
				//hidden var
				std::for_each((*it)->GetPermanent()->begin(), (*it)->GetPermanent()->end(), [&shapes_to_merge](const pot::IPotential* p) { shapes_to_merge.push_back(p); });
				std::for_each((*it)->GetTemporary()->begin(), (*it)->GetTemporary()->end(), [&shapes_to_merge](const pot::Factor& p) { shapes_to_merge.push_back(&p); });

				auto active = (*it)->GetActiveConnections();
				for (auto it = active->begin(); it != active->end(); ++it) {
					auto itN = Group.find((*it)->GetNeighbour());
					if (itN == Group.end()) {
						//neighbour outside from the subgraph
						shapes_to_merge.push_back((*it)->GetIncomingMessage());
					}
					else {
						//neighbour inside of the subgraph
						const pot::IPotential* el = (*it)->GetPot();
						auto itB = bin_pot.find(el);
						if (itB == bin_pot.end()) {
							bin_pot.emplace(el);
							shapes_to_merge.push_back(el);
						}
					}
				}
			}
			else {
				//observed var
				ind_created.emplace_back((*it)->GetVar(), ito->second->second);
				shapes_to_merge.push_back(&ind_created.back());
			}
		}

		vector<const pot::IPotential*> shapes2;
		shapes2.reserve(shapes_to_merge.size());
		std::for_each(shapes_to_merge.begin(), shapes_to_merge.end(), [&shapes2](const pot::IPotential* p) { shapes2.push_back(p); });

		pot::Factor JointPunorderd(shapes2, false, true);
		JointPunorderd.Normalize2();

		pot::Factor* result = new pot::Factor(varG);
		distr::DiscreteDistribution::constFullMatchFinder JointFinder(JointPunorderd.GetDistribution(), varG);
		JointDomainIterator::forEach(varG, [result, &JointFinder](const vector<size_t>& comb) { result->AddValue(comb , JointFinder(comb)->GetVal()); });

		return unique_ptr<pot::Factor>(result);

	}

}