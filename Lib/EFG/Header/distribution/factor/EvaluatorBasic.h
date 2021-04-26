/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_EVALUATORBASIC_H
#define EFG_DISTRIBUTION_FACTOR_EVALUATORBASIC_H

#include <distribution/Evaluator.h>

namespace EFG::distribution::factor {
    /**
     * @brief image = exp(w * rowImage)
     */
    class EvaluatorBasic : public Evaluator {
    public:
        inline float evaluate(const float& toConvert) const override { return toConvert; };
        std::shared_ptr<Evaluator> copy() const override { return std::make_shared<EvaluatorBasic>(); };
    };
}

#endif
