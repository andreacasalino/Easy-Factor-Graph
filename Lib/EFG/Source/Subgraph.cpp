/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <Node.h>
#include <set>
using namespace std;

namespace EFG {

	std::unique_ptr<Potential_Shape> Node::Node_factory::Get_joint_marginal_distribution(const std::vector<std::string>& subgroup) {

		class NodeComp {
			less<string>  cmp;
		public:
			bool operator()(const Node* a, const Node* b) const {
				return this->cmp(a->Get_var()->Get_name(), b->Get_var()->Get_name());
			};
		};
		set<Node*, NodeComp> Group;
	
		vector<Categoric_var*> varG;
		varG.reserve(subgroup.size());
		for (size_t k = 0; k < subgroup.size(); ++k) {
			Node* temp = this->_Find_Node(subgroup[k]);
			if (temp == nullptr) throw std::runtime_error("variable not found");
			Group.emplace(temp);
			varG.push_back(temp->Get_var());
		}
		Categoric_var::check_all_vars_are_different(varG);

		this->_Belief_Propagation(true);

		class Indicator : public Potential_Shape {
		public:
			Indicator(Categoric_var* var, const size_t& val_observed) : Potential_Shape({ var }) {

				size_t K = var->size();
				for (size_t k = 0; k < K; ++k) {
					if (k == val_observed) this->distribution.add(&k, 1.f);
					//else this->distribution.add(&k, 0.f);
				}

			};
		};


		class BinPotHasher {
			hash<string> hasher;
		public:
			pair<const I_Potential*, size_t> operator()(const I_Potential* p) const {
				const vector<Categoric_var*>& var = p->Get_Distribution().Get_Variables();
				return std::make_pair(p, this->hasher(string(var.front()->Get_name() + var.back()->Get_name())));
			};
		} bin_hasher;
		class BinPotComp {
		public:
			bool operator()(const pair<const I_Potential*, size_t>& a, const pair<const I_Potential*, size_t>& b) const {
				return (a.second < b.second);
			};
		};
		set<pair<const I_Potential*, size_t>, BinPotComp>  bin_pot;

		list<const I_Potential*> shapes_to_merge;
		list<Indicator> ind_created;
		for (auto it = Group.begin(); it != Group.end(); ++it) {
			auto ito = this->Last_observation_set.get_map()->find(*it);
			if (ito == this->Last_observation_set.get_map()->end()) {
				//hidden var
				auto perm = (*it)->Get_Permanent();
				for (auto it = perm->begin(); it != perm->end(); ++it) shapes_to_merge.push_back(*it);
				auto temp = (*it)->Get_Temporary();
				for (auto it = temp->begin(); it != temp->end(); ++it) shapes_to_merge.push_back(&(*it));

				auto active = (*it)->Get_Active_connections();
				for (auto it = active->begin(); it != active->end(); ++it) {
					auto itN = Group.find((*it)->Get_Neighbour());
					if (itN == Group.end()) {
						//neighbour outside from the subgraph
						shapes_to_merge.push_back((*it)->Get_IncomingMessage());
					}
					else {
						//neighbour inside of the subgraph
						auto el = bin_hasher((*it)->Get_pot());
						auto itB = bin_pot.find(el);
						if (itB == bin_pot.end()) {
							bin_pot.emplace(move(el));
							shapes_to_merge.push_back(el.first);
						}
					}
				}
			}
			else {
				//observed var
				ind_created.emplace_back((*it)->Get_var(), ito->second->second);
				shapes_to_merge.push_back(&ind_created.back());
			}
		}

		vector<const I_Potential*> shapes2;
		shapes2.reserve(shapes_to_merge.size());
		for (auto it = shapes_to_merge.begin(); it != shapes_to_merge.end(); ++it) shapes2.push_back(*it);

		Potential_Shape JointPunorderd(shapes2, false, true);
		JointPunorderd.Normalize_distribution2();

		Potential_Shape* result = new Potential_Shape(varG);
		Discrete_Distribution::const_Full_Match_finder JointFinder(JointPunorderd.Get_Distribution(), varG);
		Domain_iterator JointIt(varG);
		while (JointIt.is_not_at_end()) {
			result->Add_value(JointIt(), JointFinder(JointIt())->Get_val());
			++JointIt;
		}
		return unique_ptr<Potential_Shape>(result);

	}

}