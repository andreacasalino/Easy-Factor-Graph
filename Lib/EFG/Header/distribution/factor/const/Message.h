/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_MESSAGE_H
#define EFG_DISTRIBUTION_FACTOR_CONST_MESSAGE_H

#include <distribution/factor/const/Factor.h>

namespace EFG::distribution::factor::cnst {
    class MessageSum : public distribution::factor::cnst::Factor {
    public:
        MessageSum(const distribution::Distribution& factor, const categoric::Group& toMarginalize);
    };

    class MessageMAP : public distribution::factor::cnst::Factor {
    public:
        MessageMAP(const distribution::Distribution& factor, const categoric::Group& toMarginalize);
    };
}

#endif
