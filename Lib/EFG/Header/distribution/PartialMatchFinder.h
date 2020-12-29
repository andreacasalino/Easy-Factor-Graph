/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_PARTIALMATCH_H__
#define __EFG_DISTRIBUTION_PARTIALMATCH_H__

#include <distribution/Distribution.h>

namespace EFG::distr {

    class DiscreteDistribution::IPartialfinder : public DiscreteDistribution::Ifinder {
    protected:
        IPartialfinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars);

        template<typename Array, typename Val>
        std::list<Val*>                   __find(const Array& combination) const {

            std::list<Val*> match;
            bool flag;
            for (auto it = this->source->Map.begin(); it != this->source->Map.end(); ++it) {
                flag = true;
                for (size_t k = 0; k < this->varOrder.size(); ++k) {
                    if (combination[k] != it->second->GetIndeces()[this->varOrder[k]]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) match.push_back(it->second);
            }
            return match;

        };
    private:
        // data
        std::vector<size_t>                                 varOrder;
    };

    class DiscreteDistribution::PartialMatchFinder : public DiscreteDistribution::IPartialfinder {
    public:
        PartialMatchFinder(DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : IPartialfinder(distrib, vars) {};

        template<typename Array>
        inline std::list<Value*>                      operator()(const Array& combination) { return this->__find<Array, Value>(combination); };
    };

    class DiscreteDistribution::constPartialMatchFinder : public DiscreteDistribution::IPartialfinder {
    public:
        constPartialMatchFinder(const DiscreteDistribution& distrib, const std::vector<CategoricVariable*>& vars) : IPartialfinder(distrib, vars) {};

        template<typename Array>
        inline std::list<const Value*>              operator()(const Array& combination)  const { return this->__find<Array, const Value>(combination); };
    };

}

#endif