/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_EVALUATOR_H
#define EFG_DISTRIBUTION_EVALUATOR_H

#include <memory>

namespace EFG::distribution {
    class Evaluator {
    public:
        /**
         * @brief applies a specific function to obtain the image from a row value
         * @param the row image value to convert
         * @return the converted image value
         */
        virtual float evaluate(const float& toConvert) const = 0;

        virtual std::shared_ptr<Evaluator> copy() const = 0;
    };

    typedef std::shared_ptr<Evaluator> EvaluatorPtr;
}

#endif
