#include <gtest/gtest.h>
#include <CombinationCompare.h>
#include <categoric/Range.h>
#include <list>
using namespace EFG;
using namespace EFG::categoric;

inline categoric::Combination makeCombination(const std::vector<std::size_t>& values) {
    return Combination(values.data(), values.size());
};

void compare(Range& range, const std::list<categoric::Combination>& expected) {
    auto it = expected.begin();
    EFG::iterator::forEach(range, [&it](const Range& range){
        test::compare(range.get(), *it);
        ++it;
    });    
}

TEST(Range, binaryGroupSmall) { 
    VariablePtr A = makeVariable(2 , "A");
    VariablePtr B = makeVariable(2 , "B");
    Range rangeAB({A, B});

    compare(rangeAB, {makeCombination({0,0}),
                      makeCombination({0,1}),
                      makeCombination({1,0}),
                      makeCombination({1,1})}); 
}

TEST(Range, binaryGroupBig) { 
    VariablePtr A = makeVariable(3 , "A");
    VariablePtr B = makeVariable(4 , "B");
    Range rangeAB({A, B});

    compare(rangeAB, {makeCombination({0,0}),
                      makeCombination({0,1}),
                      makeCombination({0,2}),
                      makeCombination({0,3}),
                      makeCombination({1,0}),
                      makeCombination({1,1}),
                      makeCombination({1,2}),
                      makeCombination({1,3}),
                      makeCombination({2,0}),
                      makeCombination({2,1}),
                      makeCombination({2,2}),
                      makeCombination({2,3})}); 
}

TEST(Range, ternaryGroup) { 
    VariablePtr A = makeVariable(3 , "A");
    VariablePtr B = makeVariable(4 , "B");
    VariablePtr C = makeVariable(2 , "C");
    Range rangeABC({A, B, C});

    compare(rangeABC, {makeCombination({0,0,0}),
                       makeCombination({0,0,1}),
                       makeCombination({0,1,0}),
                       makeCombination({0,1,1}),
                       makeCombination({0,2,0}),
                       makeCombination({0,2,1}),
                       makeCombination({0,3,0}),
                       makeCombination({0,3,1}),
                       makeCombination({1,0,0}),
                       makeCombination({1,0,1}),
                       makeCombination({1,1,0}),
                       makeCombination({1,1,1}),
                       makeCombination({1,2,0}),
                       makeCombination({1,2,1}),
                       makeCombination({1,3,0}),
                       makeCombination({1,3,1}),
                       makeCombination({2,0,0}),
                       makeCombination({2,0,1}),
                       makeCombination({2,1,0}),
                       makeCombination({2,1,1}),
                       makeCombination({2,2,0}),
                       makeCombination({2,2,1}),
                       makeCombination({2,3,0}),
                       makeCombination({2,3,1})}); 
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
