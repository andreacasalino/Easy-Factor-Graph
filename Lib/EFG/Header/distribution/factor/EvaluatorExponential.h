/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_EVALUATOREXPONENTIAL_H
#define EFG_DISTRIBUTION_FACTOR_EVALUATOREXPONENTIAL_H

#include <distribution/Evaluator.h>
#include <math.h>

namespace EFG::distribution::factor {
    /**
     * @brief An exponential function with weight w is used to obtain the image,
     * i.e. image = exp(w * rowImage)
     */
    class EvaluatorExponential : public Evaluator {
    public:
        EvaluatorExponential(const float& weight) : weight(weight) {};

        inline float getWeight() const { return this->weight; };
        void setWeight(float w) { this->weight = w; };

        inline float evaluate(const float& toConvert) const { return expf(this->weight * toConvert); };
        std::shared_ptr<Evaluator> copy() const override { return std::make_shared<EvaluatorExponential>(this->weight); };

    private:
        float weight;
    };
}

#endif
