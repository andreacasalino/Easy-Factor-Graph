/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <gtest/gtest.h>
#include <DistributionCompare.h>
#include <distribution/DistributionIterator.h>
#include <CombinationCompare.h>

namespace EFG::test {
    void compare(const categoric::VariablePtr& a, const categoric::VariablePtr& b) {
        EXPECT_EQ(a->size(), b->size());
        EXPECT_EQ(a->name(), b->name());
    };

    void compare(const distribution::Distribution& a, const distribution::Distribution& b) {
        EXPECT_EQ(a.getGroup().getVariables(), b.getGroup().getVariables());
        std::size_t sizeA = 0, sizeB = 0;
        {
            auto itA = a.getIterator();
            iterator::forEach(itA, [&sizeA](const distribution::DistributionIterator& itA){
                ++sizeA;
            });
            auto itB = b.getIterator();
            iterator::forEach(itB, [&sizeB](const distribution::DistributionIterator& itB){
                ++sizeB;
            });
        }
        EXPECT_EQ(sizeA, sizeB);

        auto itA = a.getIterator();
        auto itB = b.getIterator();
        iterator::forEach(itB, [&itA](const distribution::DistributionIterator& itB){
            compare(itA.getCombination(), itB.getCombination());
            compare(itA.getImage(), itB.getImage());
            ++itA;
        });
    };

    void compare(const distribution::factor::cnst::Factor& a, const distribution::factor::cnst::Factor& b) {
        compare(static_cast<const distribution::Distribution&>(a), static_cast<const distribution::Distribution&>(b));
    }

    void compare(const distribution::factor::cnst::FactorExponential& a, const distribution::factor::cnst::FactorExponential& b) {
        compare(static_cast<const distribution::Distribution&>(a), static_cast<const distribution::Distribution&>(b));
        EXPECT_EQ(a.getWeight(), b.getWeight());
    }

    void compare(const distribution::factor::modif::FactorExponential& a, const distribution::factor::modif::FactorExponential& b) {
        compare(static_cast<const distribution::Distribution&>(a), static_cast<const distribution::Distribution&>(b));
        EXPECT_EQ(a.getWeight(), b.getWeight());
    }
}
