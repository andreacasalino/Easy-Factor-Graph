/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_FULLMATCH_H__
#define __EFG_DISTRIBUTION_FULLMATCH_H__

#include <distribution/Distribution.h>

namespace EFG::distr {

    class DiscreteDistribution::IFullfinder : public DiscreteDistribution::Ifinder {
    protected:
        IFullfinder(const DiscreteDistribution& distrib) : Ifinder(distrib) { };
        IFullfinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars);

        template<typename Array, typename Val>
        Val* __find(const Array& combination) const {

            auto it = this->source->Map.find(DiscreteDistribution::Key{ &combination[0] , this->varOrder.get() });
            if (it == this->source->Map.end()) return nullptr;
            return it->second;

        };
    private:
        // data
        std::unique_ptr<std::vector<size_t>> varOrder;
    };

    class DiscreteDistribution::FullMatchFinder : public DiscreteDistribution::IFullfinder {
    public:
        FullMatchFinder(DiscreteDistribution& distrib) : IFullfinder(distrib) {};
        FullMatchFinder(DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : IFullfinder(distrib, vars) {};

        template<typename Array>
        inline Value* operator()(const Array& combination) { return this->__find<Array, Value>(combination); };
    };

    class DiscreteDistribution::constFullMatchFinder : public DiscreteDistribution::IFullfinder {
    public:
        constFullMatchFinder(const DiscreteDistribution& distrib) : IFullfinder(distrib) {};
        constFullMatchFinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : IFullfinder(distrib, vars) {};

        template<typename Array>
        inline const Value* operator()(const Array& combination)  const { return this->__find<Array, const Value>(combination); };
    };

}

#endif