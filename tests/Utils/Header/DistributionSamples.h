/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TEST_DISTRIBUTION_SAMPLES_H
#define EFG_TEST_DISTRIBUTION_SAMPLES_H

#include <gtest/gtest.h>
#include <distribution/factor/const/Factor.h>

namespace EFG::test {
    class FactorTest 
    : public ::testing::Test
    , public distribution::factor::cnst::Factor { 
    protected: 
        FactorTest(const std::set<categoric::VariablePtr>& variables);

        const std::vector<categoric::Combination> combinations;
    };

    class FactorUnaryTest : public FactorTest {
    public:
        FactorUnaryTest();
    };

    class FactorBinaryTest : public FactorTest {
    public:
        FactorBinaryTest();
    };

    class FactorTernaryTest : public FactorTest {
    public:
        FactorTernaryTest();
    };
}

#endif
