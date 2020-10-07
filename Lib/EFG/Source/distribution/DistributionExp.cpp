#include <distribution/DistributionExp.h>

namespace EFG::distr {

	ExpDistribution::ExpDistribution(const std::vector<CategoricVariable*>& vars, const float w)
		: DiscreteDistribution(vars, std::make_unique<ExpImageEvaluator>(w)) {
	};

}