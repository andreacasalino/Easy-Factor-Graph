/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_IMAGE_BASIC_H
#define EFG_DISTRIBUTION_IMAGE_BASIC_H

#include <distribution/image/Evaluator.h>

namespace EFG::distribution::image {
    class Basic : public Evaluator {
    public:
        inline float evaluate(const float& toConvert) const override { return toConvert; };
        std::shared_ptr<Evaluator> copy() override { return std::make_shared<Basic>(); };
    };
}

#endif
