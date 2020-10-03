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

        class ExpImageEvaluator : public DiscreteDistribution::ImageEvaluator {
            inline float operator()(const float& rawVal) const override { return expf(this->weight * rawVal); };
        public:
            float weight;

            ExpImageEvaluator(const float& w) : weight(w) {};
        };

    public:
        ExpDistribution(const std::vector<CategoricVariable*>& vars, const float w = 1.f);

        inline const float& getWeight() const { return static_cast<ExpImageEvaluator*>(this->evaluator.get())->weight; }
        inline void         setWeight(const float& w) { static_cast<ExpImageEvaluator*>(this->evaluator.get())->weight = w; }

    };

}

#endif