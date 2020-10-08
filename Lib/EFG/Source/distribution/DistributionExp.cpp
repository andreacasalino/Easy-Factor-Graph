#include <distribution/DistributionExp.h>
#include <math.h>

namespace EFG::distr {

	ExpDistribution::ExpDistribution(const std::vector<CategoricVariable*>& vars, const float w)
		: DiscreteDistribution(vars), weight(w) {
	};

	ExpDistribution::ExpDistribution(ExpDistribution&& o)
		: ExpDistribution(o.GetVariables(), o.weight) {
		static_cast<DiscreteDistribution&>(*this) = std::move(static_cast<DiscreteDistribution&>(o));
	}

}