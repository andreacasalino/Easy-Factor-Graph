#include <potential/ExpFactor.h>
#include <distribution/FullMatchFinder.h>
#include <Error.h>
using namespace std;

namespace EFG::pot {

	ExpFactor::ExpFactor(const Factor& shape, const float& w) : IFactor<distr::ExpDistribution>(shape.GetDistribution().GetVariables()) {

		this->distribution.setWeight(w);

		if (CategoricVariable::GetJointDomainSize(this->distribution.GetVariables()) == shape.GetDistribution().size()) {
			auto it = shape.GetDistribution().getIter();
			itr::forEach<distr::DiscreteDistribution::constIterator>(it, [this](distr::DiscreteDistribution::constIterator& itt) { this->distribution.add(itt->GetIndeces(), itt->GetVal()); });
		}
		else {
			distr::DiscreteDistribution::constFullMatchFinder finder(shape.GetDistribution());
			JointDomainIterator::forEach(this->distribution.GetVariables(), [this, &finder](const vector<size_t>& comb) {
				auto val = finder(comb);
				if (val == nullptr) 	this->distribution.add(comb, 0.f);
				else 				this->distribution.add(comb, val->GetVal());
			});
		}

	};

	ExpFactor::ExpFactor(const std::vector<CategoricVariable*>& var_involved, const std::string& file_to_read, const float& w) :
		ExpFactor(Factor(var_involved, file_to_read), w) { };

	ExpFactor::ExpFactor(const ExpFactor& to_copy, const std::vector<CategoricVariable*>& var_involved) 
		: IFactor<distr::ExpDistribution>(var_involved) {

		if (var_involved.size() != to_copy.distribution.GetVariables().size()) throw Error("pot::ExpFactor", "invalid variable set");

		this->distribution.setWeight(to_copy.distribution.getWeight());

		auto it = to_copy.distribution.getIter();
		itr::forEach<distr::DiscreteDistribution::constIterator>(it, [this](distr::DiscreteDistribution::constIterator& itt) { this->distribution.add(itt->GetIndeces() , itt->GetValRaw()); });
	};

	ExpFactor::WeightModifier::WeightModifier(ExpFactor& involved_pot) : sbj::Subject::Observer(involved_pot.subjectWeightHndlr), pot(involved_pot) {};

	std::unique_ptr<ExpFactor> ExpFactor::Mover::createMoving(ExpFactor&& o) {
		if (o.distribution.size() == 0) throw Error("pot::ExpFactor::Mover" , "cannot move an already moved ExpFactor");
		return std::unique_ptr<ExpFactor>(new ExpFactor(std::move(o)));
	};

}