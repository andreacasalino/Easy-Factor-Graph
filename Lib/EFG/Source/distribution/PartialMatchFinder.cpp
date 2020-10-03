#include <distribution/PartialMatchFinder.h>

namespace EFG::distr {

    DiscreteDistribution::IPartialfinder::IPartialfinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : Ifinder(distrib) {

        auto distr_vars = distrib.GetVariables();
        if (vars.size() >= distr_vars.size()) throw std::runtime_error("number of vars must be lower than the domain cardinality");
        if (!CategoricVariable::AreAllVarsDifferent(vars)) throw std::runtime_error("repeated variable in set");

        this->varOrder.reserve(vars.size());
        std::size_t K2 = distr_vars.size();
        for (size_t k = 0; k < vars.size(); ++k) {
            for (size_t k2 = 0; k2 < K2; ++k2) {
                if (vars[k] == distr_vars[k2]) {
                    this->varOrder.push_back(k2);
                    break;
                }
            }
        }
        if (this->varOrder.size() != vars.size()) throw std::runtime_error("some variables in vars were not found in this domain");

    };

}
