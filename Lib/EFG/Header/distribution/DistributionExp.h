/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_EXP_H__
#define __EFG_DISTRIBUTION_EXP_H__

#include <distribution/Distribution.h>

namespace EFG::distr {

    class ExpDistribution : public DiscreteDistribution {
    public:
        ExpDistribution(const std::vector<CategoricVariable*>& vars, const float w = 1.f);

        ExpDistribution(ExpDistribution&& o);

        inline const float& getWeight() const { return this->weight; }
        inline void         setWeight(const float& w) { this->weight = w; }
    private:
        inline float evalImage(const float& valRaw) const final { return std::expf(this->weight * valRaw); };

        float weight;
    };

}

#endif