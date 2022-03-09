/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <gtest/gtest.h>
#include <CombinationCompare.h>

namespace EFG::test {
    void compare(const categoric::Combination& a, const categoric::Combination& b) {
        EXPECT_EQ(a.size(), b.size());
        for(std::size_t k=0; k<a.size(); ++k) {
            EXPECT_EQ(a.data()[k], b.data()[k]);
        }
    }
}
