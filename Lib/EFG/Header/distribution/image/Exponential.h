/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_IMAGE_EXPONENTIAL_H
#define EFG_DISTRIBUTION_IMAGE_EXPONENTIAL_H

#include <distribution/image/Evaluator.h>
#include <math.h>

namespace EFG::distribution::image {
    class Exponential : public Evaluator {
    public:
        Exponential(const float& weight) : weight(weight) {};

        inline float getWeight() const { return this->weight; };
        void setWeight(float w) { this->weight = w; };

        inline float evaluate(const float& toConvert) const { return expf(toConvert); };
        std::shared_ptr<Evaluator> copy() const override { return std::make_shared<Exponential>(this->weight); };

    private:
        float weight;
    };
}

#endif
