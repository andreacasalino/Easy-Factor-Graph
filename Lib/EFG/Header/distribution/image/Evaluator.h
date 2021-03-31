/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_IMAGE_EVALUATOR_H
#define EFG_DISTRIBUTION_IMAGE_EVALUATOR_H

#include <memory>

namespace EFG::distribution::image {
    class Evaluator {
    public:
        virtual float evaluate(const float& toConvert) const = 0;

        virtual std::shared_ptr<Evaluator> copy() = 0;
    };

    typedef std::shared_ptr<Evaluator> EvaluatorPtr;
}

#endif
