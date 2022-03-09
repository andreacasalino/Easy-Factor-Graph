/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TEST_DISTRIBUTION_COMPARE_H
#define EFG_TEST_DISTRIBUTION_COMPARE_H

#include <distribution/factor/const/Factor.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::test {
    void compare(const distribution::Distribution& a, const distribution::Distribution& b);

    void compare(const distribution::factor::cnst::Factor& a, const distribution::factor::cnst::Factor& b);

    void compare(const distribution::factor::cnst::FactorExponential& a, const distribution::factor::cnst::FactorExponential& b);

    void compare(const distribution::factor::modif::FactorExponential& a, const distribution::factor::modif::FactorExponential& b);
}

#endif
