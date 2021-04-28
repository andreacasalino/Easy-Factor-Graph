#include <gtest/gtest.h>
#include <categoric/Range.h>
using namespace EFG;
using namespace EFG::categoric;

TEST(Range, binaryGroupA) { 
    VariablePtr A = makeVariable(2 , "A");
    VariablePtr B = makeVariable(2 , "B");

    Range rangeAB({A, B});
    // EFG::iterator::forEach(rangeAB, [](const Range& rangeAB){

    // });    
}

TEST(Range, binaryGroupB) { 
    int n = 1; 
    EXPECT_EQ(n, 1);
}

TEST(Range, unaryGroup) { 
    int n = 1; 
    EXPECT_EQ(n, 1);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
