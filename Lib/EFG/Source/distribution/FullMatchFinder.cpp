#include <distribution/FullMatchFinder.h>
#include <Error.h>

namespace EFG::distr {

    DiscreteDistribution::IFullfinder::IFullfinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : DiscreteDistribution::Ifinder(distrib) {

        auto distr_vars = distrib.GetVariables();
        if (vars.size() < distr_vars.size()) throw Error("distr::DiscreteDistribution::IFullfinder", "number of vars must be higher or equal to the domain cardinality");
        if (!CategoricVariable::AreAllVarsDifferent(vars)) throw Error("distr::DiscreteDistribution::IFullfinder", "repeated variable in set");

        this->varOrder = std::make_unique<std::vector<size_t>>();
        this->varOrder->reserve(distr_vars.size());
        std::size_t K2 = vars.size();
        for (size_t k = 0; k < distr_vars.size(); ++k) {
            for (size_t k2 = 0; k2 < K2; ++k2) {
                if (distr_vars[k] == vars[k2]) {
                    this->varOrder->push_back(k2);
                    break;
                }
            }
        }
        if (this->varOrder->size() != distrib.GetVariables().size()) throw Error("distr::DiscreteDistribution::IFullfinder", "some variables in the domain of the distributon were not found in vars");

    };

}